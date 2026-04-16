import argparse
import csv
import itertools
import multiprocessing
import re
import subprocess
import sys
from concurrent.futures import ProcessPoolExecutor
from pathlib import Path

PIVOTS = ("first", "best")
START_SOLS = ("random", "c_and_w")
NEIGHBORHOODS = (["transpose"], ["exchange"], ["insert"])

RESULT_PATTERN = re.compile(
    r"RESULT\s+cost=(?P<cost>\d+)\s+time=(?P<time>[0-9.eE+-]+)\s+solution=(?P<solution>[\d+\s]+)"
)
BenchmarkJob = tuple[str, int, Path, str, list[str], str]


def parse_best_known(path: Path) -> list[tuple[str, int]]:
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
    cmd_cmake_setup = [
        "cmake",
        "-S.",
        "-Bbuild",
    ]

    cmd_build = [
        "cmake",
        "--build",
        "build",
    ]

    completed = subprocess.run(
        cmd_cmake_setup,
        check=False,
        capture_output=True,
        text=True,
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
    return Path("build/bin/lop")


def run_solver_once(
    binary_path: Path,
    instance_path: Path,
    pivot: str,
    neighborhoods: list[str],
    sol_start: str,
    timeout_seconds: float,
    is_solution: bool,
) -> tuple[int, float, list[int] | None]:
    cmd = [
        str(binary_path),
        "-i",
        str(instance_path),
        "-p",
        pivot,
        "-s",
        sol_start,
    ]
    for neighb in neighborhoods:
        cmd.extend(["-n", neighb])

    # print(f"Running command: {' '.join(cmd)}", file=sys.stderr)

    completed = subprocess.run(
        cmd,
        check=False,
        capture_output=True,
        text=True,
        timeout=timeout_seconds if timeout_seconds > 0 else None,
    )

    if completed.returncode != 0:
        raise RuntimeError(
            "Solver run failed\n"
            f"Command: {' '.join(cmd)}\n"
            f"Exit code: {completed.returncode}\n"
            f"STDOUT:\n{completed.stdout}\n"
            f"STDERR:\n{completed.stderr}"
        )

    match = RESULT_PATTERN.search(completed.stdout)
    if match is None:
        raise RuntimeError(
            "Could not parse solver result line\n"
            f"Command: {' '.join(cmd)}\n"
            f"STDOUT:\n{completed.stdout}\n"
            f"STDERR:\n{completed.stderr}"
        )

    cost = int(match.group("cost"))
    elapsed_seconds = float(match.group("time"))
    solution = (
        [int(i) for i in match.group("solution").strip().split(" ")]
        if is_solution
        else None
    )
    return cost, elapsed_seconds, solution


def run_benchmark_job(
    job: BenchmarkJob,
    binary_path: Path,
    runs_per_job: int,
    timeout_seconds: float,
    is_solution: bool,
) -> dict[str, object]:
    (
        instance_name,
        best_known_cost,
        instance_path,
        pivot,
        neighborhoods,
        sol_start,
    ) = job

    costs: list[int] = []
    times: list[float] = []
    solutions: list[list[int] | None] = []

    for _ in range(runs_per_job):
        cost, elapsed, solution = run_solver_once(
            binary_path=binary_path,
            instance_path=instance_path,
            pivot=pivot,
            neighborhoods=neighborhoods,
            sol_start=sol_start,
            timeout_seconds=timeout_seconds,
            is_solution=is_solution,
        )
        costs.append(cost)
        times.append(elapsed)
        solutions.append(solution)

    cost = max(costs)
    time = min(times)
    solution = solutions[0]
    relative_percentage_deviation = ((cost - best_known_cost) / best_known_cost) * 100

    return {
        "instance": instance_name,
        "best_known_cost": best_known_cost,
        "pivot": pivot,
        "neighborhoods": neighborhoods,
        "sol_start": sol_start,
        "cost": cost,
        "time_s": time,
        "rel_percent_deviation": relative_percentage_deviation,
        "gap_to_best": cost - best_known_cost,
        "solution": solution,
    }


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Run all best_known instances for every combination of "
            "pivot/neighborhood/start of lop."
        )
    )
    parser.add_argument(
        "--k",
        type=int,
        default=1,
        help="Number of runs per algo combination (instance, pivot, neighborhood, sol_start) and select the best time.",
    )
    parser.add_argument(
        "--output_it_imp",
        type=Path,
        default=Path("data/output/lop_analysis.csv"),
        help="Output CSV path for iterative improvement lop.",
    )
    parser.add_argument(
        "--output_vnd",
        type=Path,
        default=Path("data/output/lop_vnd_analysis.csv"),
        help="Output CSV path for VND results.",
    )
    parser.add_argument(
        "--binary",
        type=Path,
        default=Path("build/bin/lop"),
        help="Path to the solver binary.",
    )
    parser.add_argument(
        "--best-known-file",
        type=Path,
        default=Path("data/best_known.txt"),
        help="Path to the best-known instances file.",
    )
    parser.add_argument(
        "--instances-dir",
        type=Path,
        default=Path("data/input/instances"),
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
        help="Timeout in seconds for each solver run. 0 means no timeout.",
    )
    parser.add_argument(
        "-w",
        "--workers",
        type=int,
        default=0,
        help=(
            "Number of workers used to run benchmark combinations in parallel processes. "
            "Use 1 to disable parallel execution and 0 to use all CPUs minus one."
            "Overloading the CPU may show slower time per test measurement but will be faster if only the result matter"
        ),
    )
    return parser.parse_args()


def benchmark(args, combinations: list, instances, output_path: Path):
    total_runs = len(instances) * len(combinations) * args.k
    current_run = 0

    jobs: list[BenchmarkJob] = []

    for instance_name, best_known_cost in instances:
        instance_path = args.instances_dir / instance_name
        if not instance_path.is_file():
            raise FileNotFoundError(f"Instance file not found: {instance_path}")

        for pivot, neighborhoods, sol_start in combinations:
            jobs.append(
                (
                    instance_name,
                    best_known_cost,
                    instance_path,
                    pivot,
                    neighborhoods,
                    sol_start,
                )
            )

    output_path.parent.mkdir(parents=True, exist_ok=True)
    with output_path.open("w", encoding="utf-8", newline="") as csv_file:
        writer = csv.DictWriter(
            csv_file,
            fieldnames=[
                "instance",
                "best_known_cost",
                "pivot",
                "neighborhoods",
                "sol_start",
                "cost",
                "time_s",
                "rel_percent_deviation",
                "gap_to_best",
                "solution",
            ],
        )
        writer.writeheader()

        with ProcessPoolExecutor(max_workers=args.workers) as executor:
            for row in executor.map(
                run_benchmark_job,
                jobs,
                itertools.repeat(args.binary),
                itertools.repeat(args.k),
                itertools.repeat(args.timeout),
                itertools.repeat(args.solution),
            ):
                current_run += args.k
                print(
                    f"[{current_run}/{total_runs}] {row['instance']} "
                    f"s={row['sol_start']} p={row['pivot']} n={row['neighborhoods']}",
                    file=sys.stderr,
                )
                writer.writerow(row)


def main() -> int:
    args = parse_args()

    if args.k < 1:
        raise ValueError("--k must be greater than 0")
    if args.workers < 0:
        raise ValueError("--workers must be greater than or equal to 0")
    if args.workers == 0:
        args.workers = max(1, multiprocessing.cpu_count() - 1)
    if args.workers > multiprocessing.cpu_count():
        raise ValueError(
            "--workers more workers specified than available on this system"
        )
    print(f"{args.workers} processors allocated")
    if not args.binary.is_file():
        args.binary = run_compile_target()
        if not args.binary.is_file():
            raise FileNotFoundError(f"Solver binary not found: {args.binary}")
    if not args.best_known_file.is_file():
        raise FileNotFoundError(f"Best-known file not found: {args.best_known_file}")
    if not args.instances_dir.is_dir():
        raise FileNotFoundError(f"Instances directory not found: {args.instances_dir}")

    instances = parse_best_known(args.best_known_file)
    if not instances:
        raise RuntimeError("No instances loaded from best-known file")

    # iterative improvement benchmark with all pivot/neighborhood/sol_start combinations

    combinations = list(itertools.product(PIVOTS, NEIGHBORHOODS, START_SOLS))

    benchmark(args, combinations, instances, args.output_it_imp)

    print(f"Wrote iterative improvment benchmark results to: {args.output_it_imp}")

    # variant neighborhood descent benchmark with two different neighborhood orderings

    vnd_neighborhoods = [
        ["transpose", "exchange", "insert"],
        ["transpose", "insert", "exchange"],
    ]

    combinations = [
        ["first", vnd_neighbs, "c_and_w"] for vnd_neighbs in vnd_neighborhoods
    ]

    benchmark(args, combinations, instances, args.output_vnd)

    print(f"Wrote VND benchmark results to: {args.output_vnd}")

    print("All done !")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
