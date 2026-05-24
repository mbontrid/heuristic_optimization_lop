import argparse
import csv
import itertools
import multiprocessing
import re
import signal
import subprocess
from concurrent.futures import ProcessPoolExecutor
from pathlib import Path
from typing import Sequence

from tqdm import tqdm

PIVOTS = ("first", "best")
START_SOLS = ("random", "c_and_w")
NEIGHBORHOODS = (["transpose"], ["exchange"], ["insert"])


PATH_TO_OUTPUT = Path("data/output/")
PATH_TO_IN = Path("data/input/")

PATH_TO_BEST_KNOWN = PATH_TO_IN / Path("best_known.txt")
PATH_TO_INSTANCES = PATH_TO_IN / Path("instances/")
PATH_TO_BINARY = Path("build/bin/lop")

IT_IMP_OUTPUT_FILE = Path("it_im_results.csv")
LOP_VND_OUTPUT_FILE = Path("lop_vnd_results.csv")
MEME_OUTPUT_FILE = Path("meme_results.csv")
ILS_OUTPUT_FILE = Path("ils_results.csv")
ILS_PARAM_OUTPUT_FILE = Path("ils_param_results.csv")
MEME_PARAM_OUTPUT_FILE = Path("meme_param_results.csv")
PROJECT_ROOT = Path(__file__).resolve().parents[1]


RESULT_PATTERN = re.compile(
    r"RESULT\s+cost=(?P<cost>\d+)\s+time=(?P<time>[0-9.eE+-]+)\s+solution=(?P<solution>[\d+\s]+)"
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Run all best_known instances for every combination of "
            "pivot/neighborhood/start of lop."
        )
    )
    parser.add_argument(
        "--bench",
        type=str,
        choices=["it_imp", "vnd", "meme_param", "memetic", "ils", "ils_param", "all"],
        default="all",
        help=(
            "which benchmark to run: 'it_imp' for iterative improvement, 'vnd' for VND, "
            "'memetic' for memetic algorithm, 'meme_param' for memetic parameter sweep, "
            "'ils' for iterated local search, 'ils_param' for ILS parameter sweep, 'all' for all benchmarks. Default: all"
        ),
    )
    parser.add_argument(
        "-k",
        "--runs",
        type=int,
        default=1,
        help="Number of runs per algo combination and select the best time.",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=PATH_TO_OUTPUT,
        help="Output CSV directory.",
    )
    parser.add_argument(
        "--binary",
        type=Path,
        default=PATH_TO_BINARY,
        help="Path to the solver binary.",
    )
    parser.add_argument(
        "--best_known_file",
        type=Path,
        default=PATH_TO_BEST_KNOWN,
        help="Path to the best-known instances file.",
    )
    parser.add_argument(
        "--instances-dir",
        type=Path,
        default=PATH_TO_INSTANCES,
        help="Directory containing instance files.",
    )
    parser.add_argument(
        "-s",
        "--solution",
        # action=argparse.BooleanOptionalAction,
        action="store_true",
        help="Whether to include the solution in the output CSV.",
    )
    parser.add_argument(
        "--timeout",
        type=int,
        default=0,
        help="Timeout in seconds for each solver run. 0 means no timeout. The result will be retrieved.",
    )
    parser.add_argument(
        "-w",
        "--workers",
        type=int,
        default=-1,
        help=(
            "Number of workers used to run benchmark combinations in parallel processes. "
            "Use 1 to disable parallel execution and -x to use all CPUs minus x."
            "Overloading the CPU may show slower time per test measurement but will be faster if only the result matter"
        ),
    )
    args = parser.parse_args()
    args = args_fix(args)
    return args


def args_fix(args: argparse.Namespace) -> argparse.Namespace:
    if args.runs < 1:
        raise ValueError("--runs must be greater than 0")

    if not args.output.is_absolute():
        args.output = PROJECT_ROOT / args.output
    if not args.binary.is_absolute():
        args.binary = PROJECT_ROOT / args.binary
    if not args.best_known_file.is_absolute():
        args.best_known_file = PROJECT_ROOT / args.best_known_file
    if not args.instances_dir.is_absolute():
        args.instances_dir = PROJECT_ROOT / args.instances_dir

    print(f"this computer has {multiprocessing.cpu_count()} CPU cores")
    print(args.workers)
    if args.workers <= 0:
        args.workers = multiprocessing.cpu_count() + args.workers
        print(
            f"Auto-detected {multiprocessing.cpu_count()} cores. assigning {args.workers} workers for benchmark (all CPUs minus {-args.workers})"
        )
    if args.workers > multiprocessing.cpu_count():
        args.workers = max(1, multiprocessing.cpu_count())
    print(f"{args.workers} processes allocated")

    if not args.binary.is_file():
        run_compile_target()
        if not args.binary.is_file():
            raise FileNotFoundError(f"Solver binary not found: {args.binary}")
    if not args.best_known_file.is_file():
        raise FileNotFoundError(f"Best-known file not found: {args.best_known_file}")
    if not args.instances_dir.is_dir():
        raise FileNotFoundError(f"Instances directory not found: {args.instances_dir}")

    if args.output.exists() and not args.output.is_dir():
        raise NotADirectoryError(f"Output path must be a directory: {args.output}")

    args.output_it_imp = args.output / IT_IMP_OUTPUT_FILE
    args.output_vnd = args.output / LOP_VND_OUTPUT_FILE
    args.output_ils = args.output / ILS_OUTPUT_FILE
    args.output_ils_param = args.output / ILS_PARAM_OUTPUT_FILE
    args.output_memetic = args.output / MEME_OUTPUT_FILE
    args.output_meme_param = args.output / MEME_PARAM_OUTPUT_FILE

    return args


class RunArgs:
    def __init__(self):
        self.instance_path: Path = PATH_TO_INSTANCES / Path("N-be75eec_150")
        self.pivot = "first"
        self.neighborhoods = ["exchange"]
        self.sol_start = "c_and_w"
        self.algo = "vnd"
        self.ils_perturb_rate = 0.1
        self.ils_n_try = 10
        self.ils_worst = 0
        self.meme_divers_try = 5
        self.meme_mean_try = 10
        self.meme_cross_rate_mut = 0.8
        self.meme_offspring = 10
        self.meme_pop = 20
        self.meme_mut_rate = 0.1
        self.meme_cross_rate = 0.5

    def get_args_dict(self) -> dict[str, object]:
        dict_args = {}

        dict_args["instance"] = self.instance_path
        dict_args["algo"] = self.algo
        dict_args["pivot"] = self.pivot
        dict_args["neighborhood"] = self.neighborhoods

        if self.algo in ("vnd", "ils"):
            dict_args["sol_start"] = self.sol_start

        if self.algo == "ils":
            dict_args["ils_perturb_rate"] = self.ils_perturb_rate
            dict_args["ils_n_try"] = self.ils_n_try
            dict_args["ils_worst"] = self.ils_worst
        elif self.algo == "memetic":
            dict_args["meme_divers_try"] = self.meme_divers_try
            dict_args["meme_mean_try"] = self.meme_mean_try
            dict_args["meme_cross_rate_mut"] = self.meme_cross_rate_mut
            dict_args["meme_offspring"] = self.meme_offspring
            dict_args["meme_pop"] = self.meme_pop
            dict_args["meme_mut_rate"] = self.meme_mut_rate
            dict_args["meme_cross_rate"] = self.meme_cross_rate

        return dict_args

    def get_arg_list(self) -> list[str]:
        cmd_args = []
        args_dict = self.get_args_dict()
        for key, value in args_dict.items():
            if isinstance(value, (list, tuple)):
                for entry in value:
                    cmd_args.extend([f"--{key}", str(entry)])
                continue

            cmd_args.extend([f"--{key}", str(value)])

        cmd_args.append("--result")

        return cmd_args


class RunInfo:
    def __init__(
        self,
        instance_path: Path,
        best_known_cost: int,
        binary_path: Path = PATH_TO_BINARY,
    ):
        self.run_args = RunArgs()

        self.binary_path: Path = binary_path

        self.best_known_cost: int = best_known_cost

        self.elapsed_seconds_list: list[float] = []
        self.cost_list: list[int] = []
        self.solution_list: list[list[int] | None] = []

    @property
    def instance_name(self) -> str:
        return self.run_args.instance_path.name

    def get_cmd_list(self) -> list[str]:
        return [str(self.binary_path)] + self.run_args.get_arg_list()

    def get_info_results_dict_field(self) -> list[str]:
        namefield = []
        namefield.extend(self.run_args.get_args_dict().keys())
        namefield.remove("instance")
        namefield.extend(
            ["instance", "best_known_cost", "cost", "elapsed_seconds", "solution"]
        )
        return namefield

    def get_info_results(self) -> list[dict[str, object]]:
        results_list = []
        for cost, elapsed, solution in zip(
            self.cost_list, self.elapsed_seconds_list, self.solution_list
        ):
            results = {}
            for key, value in self.run_args.get_args_dict().items():
                if key not in results.keys():
                    results[key] = value

            results.update({"instance": self.instance_name})
            results["best_known_cost"] = self.best_known_cost
            results["cost"] = cost
            results["elapsed_seconds"] = elapsed
            results["solution"] = solution

            results_list.append(results)

        return results_list


def parse_best_known(path: Path) -> list[tuple[str, int]]:
    """Retrieves the list of bes_know solutions of instances.

    Args:
        path: Path to list of best-Known.

    Returns:
        list[tuple["name", "bset_cost"]]
    """
    instances: list[tuple[str, int]] = []

    for line_num, raw_line in enumerate(
        path.read_text(encoding="utf-8").splitlines(), 1
    ):
        stripped = raw_line.strip()
        if not stripped:
            continue

        columns = stripped.split()
        if len(columns) < 2:
            raise ValueError(f"{path}:{line_num}: expected '<instance> <cost>' format")

        try:
            best_cost = int(columns[-1])
        except ValueError as exc:
            raise ValueError(
                f"{path}:{line_num}: last column must be an integer cost"
            ) from exc

        # Join all non-cost columns to tolerate accidental spaces in names.
        instance_name = "".join(columns[:-1])
        instances.append((instance_name, best_cost))

    return instances


def run_compile_target() -> Path:
    build_dir = PROJECT_ROOT / "build"
    cmd_cmake_setup = [
        "cmake",
        "-S",
        str(PROJECT_ROOT),
        "-B",
        str(build_dir),
    ]

    cmd_build = [
        "cmake",
        "--build",
        str(build_dir),
    ]

    completed = subprocess.run(
        cmd_cmake_setup,
        check=False,
        capture_output=True,
        text=True,
        cwd=PROJECT_ROOT,
    )

    if completed.returncode != 0:
        raise RuntimeError(
            "Cmake setup failed\n"
            f"Command: {' '.join(cmd_cmake_setup)}\n"
            f"Exit code: {completed.returncode}\n"
            f"STDOUT:\n{completed.stdout}\n"
            f"STDERR:\n{completed.stderr}"
        )

    completed = subprocess.run(
        cmd_build,
        check=False,
        capture_output=True,
        text=True,
        cwd=PROJECT_ROOT,
    )

    if completed.returncode != 0:
        raise RuntimeError(
            "Cmake build failed\n"
            f"Command: {' '.join(cmd_build)}\n"
            f"Exit code: {completed.returncode}\n"
            f"STDOUT:\n{completed.stdout}\n"
            f"STDERR:\n{completed.stderr}"
        )

    print("binary built.")
    return build_dir / "bin" / "lop"


def run_solver_once(
    cmd_list: list[str],
    timeout_seconds: float,
    is_solution: bool,
) -> tuple[list[int], list[float], list[list[int] | None]]:
    """Run solver once and return final cost, time, solution"""

    timeout_hit = False
    proc = subprocess.Popen(
        cmd_list,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    try:
        stdout, stderr = proc.communicate(
            timeout=timeout_seconds if timeout_seconds > 0 else None
        )
    except subprocess.TimeoutExpired:
        timeout_hit = True
        proc.send_signal(signal.SIGINT)
        try:
            stdout, stderr = proc.communicate(timeout=2)
        except subprocess.TimeoutExpired:
            proc.terminate()
            try:
                stdout, stderr = proc.communicate(timeout=2)
            except subprocess.TimeoutExpired:
                proc.kill()
                stdout, stderr = proc.communicate()

    if proc.returncode != 0 and not timeout_hit:
        raise RuntimeError(
            "Solver run failed\n"
            f"Command: {' '.join(cmd_list)}\n"
            f"Exit code: {proc.returncode}\n"
            f"STDOUT:\n{stdout}\n"
            f"STDERR:\n{stderr}"
        )

    # Extract all RESULT lines
    all_matches = list(RESULT_PATTERN.finditer(stdout))
    if not all_matches:
        raise RuntimeError(
            "Could not parse solver result line\n"
            f"Command: {' '.join(cmd_list)}\n"
            f"STDOUT:\n{stdout}\n"
            f"STDERR:\n{stderr}"
        )

    costs = [int(i.group("cost")) for i in all_matches]
    elapsed_seconds = [float(i.group("time")) for i in all_matches]
    solutions = [
        [int(i) for i in sol.strip().split(" ")] if is_solution else None
        for sol in [match.group("solution") for match in all_matches]
    ]

    return costs, elapsed_seconds, solutions


def run_benchmark_job(
    run_info: RunInfo,
    runs_per_job: int,
    timeout_seconds: float,
    is_solution: bool,
) -> list[dict[str, object]]:

    costs: list[int] = []
    elapsed_second: list[float] = []
    solutions: list[list[int] | None] = [[]]

    cmd_list = run_info.get_cmd_list()

    for _ in range(runs_per_job):
        costs, elapsed_second, solutions = run_solver_once(
            cmd_list=cmd_list,
            timeout_seconds=timeout_seconds,
            is_solution=is_solution,
        )

    run_info.cost_list = costs
    run_info.elapsed_seconds_list = elapsed_second
    run_info.solution_list = solutions

    result_info = run_info.get_info_results()

    return result_info


def benchmark(
    run_info_list: list[RunInfo],
    workers: int,
    n_runs: int,
    timeout: int,
    is_solution: bool,
    output_path: Path,
):
    total_runs = len(run_info_list) * n_runs

    output_path.parent.mkdir(parents=True, exist_ok=True)

    # Collect all fieldnames first
    fieldnames = run_info_list[0].get_info_results_dict_field()
    fieldnames = list(fieldnames)

    with output_path.open("w", encoding="utf-8", newline="") as csv_file:
        writer = csv.DictWriter(
            csv_file,
            fieldnames=fieldnames,
            restval="",
        )
        writer.writeheader()

        with tqdm(total=total_runs, unit="run", desc="benchmark") as progress:
            with ProcessPoolExecutor(max_workers=workers) as executor:
                for rows in executor.map(
                    run_benchmark_job,
                    run_info_list,
                    itertools.repeat(n_runs),
                    itertools.repeat(timeout),
                    itertools.repeat(is_solution),
                ):
                    progress.update(n_runs)
                    progress.set_postfix_str(
                        format_progress_details(rows[-1]), refresh=False
                    )
                    for row in rows:
                        writer.writerow(row)


def build_run_info_list(
    binary_path: Path,
    instances_dir: Path,
    instances: list[tuple[str, int]],
    combinations: Sequence[tuple[str, Sequence[str], str]],
    algo: str,
    extra_params: Sequence[dict[str, object]] | None = None,
) -> list[RunInfo]:
    run_info_list: list[RunInfo] = []
    if not extra_params:
        extra_params = [{}]
    for instance_name, best_cost in instances:
        instance_path = instances_dir / instance_name
        if not instance_path.is_file():
            raise FileNotFoundError(f"Instance file not found: {instance_path}")
        for pivot, neighborhoods, sol_start in combinations:
            for param_set in extra_params:
                run_info = RunInfo(instance_path, best_cost, binary_path)
                run_info.run_args.algo = algo
                run_info.run_args.pivot = pivot
                run_info.run_args.neighborhoods = list(neighborhoods)
                run_info.run_args.sol_start = sol_start
                for key, value in param_set.items():
                    setattr(run_info.run_args, key, value)
                run_info_list.append(run_info)
    return run_info_list


def format_progress_details(row: dict[str, object]) -> str:
    parts = [str(row["instance"])]
    for key in (
        "algo",
        "sol_start",
        "pivot",
        "neighborhood",
        "ils_perturb_rate",
        "ils_n_try",
        "ils_worst",
        "meme_pop",
        "meme_offspring",
        "meme_divers_try",
        "meme_mean_try",
        "meme_cross_rate_mut",
        "meme_mut_rate",
        "meme_cross_rate",
    ):
        if key in row:
            parts.append(f"{key}={row[key]}")
    return " ".join(parts)


def build_param_grid(
    param_values: dict[str, Sequence[object]],
) -> list[dict[str, object]]:
    if not param_values:
        return [{}]
    keys = list(param_values.keys())
    return [
        dict(zip(keys, values))
        for values in itertools.product(*(param_values[key] for key in keys))
    ]


def select_instances(
    instances: list[tuple[str, int]],
    instance_names: Sequence[str],
) -> list[tuple[str, int]]:
    instance_lookup = {name: cost for name, cost in instances}
    missing = [name for name in instance_names if name not in instance_lookup]
    if missing:
        raise ValueError(f"Instances not found in best-known list: {missing}")
    return [(name, instance_lookup[name]) for name in instance_names]


def main() -> int:
    args = parse_args()

    instances = parse_best_known(args.best_known_file)
    if not instances:
        raise RuntimeError("No instances loaded from best-known file")

    #####################################################################################
    # iterative improvement benchmark with all pivot/neighborhood/sol_start combinations
    #####################################################################################
    if args.bench in ("it_imp", "all"):
        print("Running iterative improvement benchmark...")
        combinations = list(itertools.product(PIVOTS, NEIGHBORHOODS, START_SOLS))

        run_info_list = build_run_info_list(
            args.binary,
            args.instances_dir,
            instances,
            combinations,
            algo="vnd",
        )
        benchmark(
            run_info_list,
            args.workers,
            args.runs,
            args.timeout,
            args.solution,
            args.output_it_imp,
        )

        print(f"Wrote iterative improvment benchmark results to: {args.output_it_imp}")

    #####################################################################################
    # variant neighborhood descent benchmark with two different neighborhood orderings
    #####################################################################################

    if args.bench in ("vnd", "all"):
        print("Running VND benchmark...")
        vnd_neighborhoods = [
            ["transpose", "exchange", "insert"],
            ["transpose", "insert", "exchange"],
        ]

        combinations = [
            ("first", vnd_neighbs, "c_and_w") for vnd_neighbs in vnd_neighborhoods
        ]

        run_info_list = build_run_info_list(
            args.binary,
            args.instances_dir,
            instances,
            combinations,
            algo="vnd",
        )
        benchmark(
            run_info_list,
            args.workers,
            args.runs,
            args.timeout,
            args.solution,
            args.output_vnd,
        )

        print(f"Wrote VND benchmark results to: {args.output_vnd}")

    #####################################################################################
    # iterated local search parameter benchmark
    #####################################################################################

    if args.bench in ("ils_param", "all"):
        print("Running iterated local search parameter benchmark...")
        param_instances = select_instances(instances, ["N-be75eec_150"])
        ils_neighborhoods = [["exchange"], ["transpose", "exchange", "insert"]]
        ils_param_grid = build_param_grid(
            {
                "ils_perturb_rate": [0.2, 0.5, 0.8],
                "ils_n_try": [10, 100],
                "ils_worst": [0, 10],
            }
        )

        ils_pivot = ["first", "best"]
        ils_start_sols = ["c_and_w", "random"]

        combinations = list(
            itertools.product(ils_pivot, ils_neighborhoods, ils_start_sols)
        )

        run_info_list = build_run_info_list(
            args.binary,
            args.instances_dir,
            param_instances,
            combinations,
            algo="ils",
            extra_params=ils_param_grid,
        )
        benchmark(
            run_info_list,
            args.workers,
            args.runs,
            args.timeout,
            args.solution,
            args.output_ils_param,
        )

        print(f"Wrote ILS parameter benchmark results to: {args.output_ils_param}")

    #####################################################################################
    # iterated local search benchmark
    #####################################################################################

    if args.bench in ("ils", "all"):
        print("Running iterated local search benchmark...")
        instances_150 = [
            (name, cost) for name, cost in instances if name.endswith("_150")
        ]
        ils_neighborhoods = [["transpose", "exchange", "insert"]]
        ils_param_grid = build_param_grid(
            {
                "ils_perturb_rate": [0.2],
                "ils_n_try": [100],
                "ils_worst": [0],
            }
        )

        ils_pivot = ["first"]
        ils_start_sols = ["c_and_w"]

        combinations = list(
            itertools.product(ils_pivot, ils_neighborhoods, ils_start_sols)
        )

        run_info_list = build_run_info_list(
            args.binary,
            args.instances_dir,
            instances_150,
            combinations,
            algo="ils",
            extra_params=ils_param_grid,
        )
        benchmark(
            run_info_list,
            args.workers,
            args.runs,
            args.timeout,
            args.solution,
            args.output_ils,
        )

        print(f"Wrote ILS benchmark results to: {args.output_ils}")

    #######################################################################################
    # memetic parameters algorithm benchmark
    #######################################################################################

    if args.bench in ("meme_param", "all"):
        print("Running memetic parameter benchmark...")
        param_instances = select_instances(instances, ["N-be75eec_150"])
        vnd_neighborhoods = [
            ["exchange"],
            ["transpose", "exchange", "insert"],
        ]

        memetic_param_grid = build_param_grid(
            {
                "meme_pop": [20],
                "meme_offspring": [10],
                "meme_divers_try": [3],
                "meme_mean_try": [10],
                "meme_cross_rate_mut": [0, 0.5, 0.8, 1],
                "meme_mut_rate": [0.1, 0.3],
                "meme_cross_rate": [0.5],
            }
        )

        combinations = list(
            itertools.product(["first", "best"], vnd_neighborhoods, ["random"])
        )

        run_info_list = build_run_info_list(
            args.binary,
            args.instances_dir,
            param_instances,
            combinations,
            algo="memetic",
            extra_params=memetic_param_grid,
        )
        benchmark(
            run_info_list,
            args.workers,
            args.runs,
            args.timeout,
            args.solution,
            args.output_meme_param,
        )

        print(
            f"Wrote memetic parameters benchmark results to: {args.output_meme_param}"
        )

    #####################################################################################
    # memetic algorithm benchmark
    #####################################################################################

    if args.bench in ("memetic", "all"):
        print("Running memetic benchmark...")
        instances_150 = [
            (name, cost) for name, cost in instances if name.endswith("_150")
        ]
        memetic_neighborhoods = [["transpose", "exchange", "insert"]]
        memetic_param_grid = build_param_grid(
            {
                "meme_pop": [25],
                "meme_offspring": [11],
                "meme_divers_try": [5],
                "meme_mean_try": [30],
                "meme_cross_rate_mut": [0.8],
                "meme_mut_rate": [0.1],
                "meme_cross_rate": [0.5],
            }
        )

        combinations = list(
            itertools.product(["best"], memetic_neighborhoods, ["random"])
        )

        run_info_list = build_run_info_list(
            args.binary,
            args.instances_dir,
            instances_150,
            combinations,
            algo="memetic",
            extra_params=memetic_param_grid,
        )
        benchmark(
            run_info_list,
            args.workers,
            args.runs,
            args.timeout,
            args.solution,
            args.output_memetic,
        )

        print(f"Wrote memetic benchmark results to: {args.output_memetic}")

    print("All done !")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
