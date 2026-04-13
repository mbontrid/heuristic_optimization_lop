import argparse
import csv
import itertools
import re
import statistics
import subprocess
import sys
from pathlib import Path

PIVOTS = ("first", "best")
START_SOLS = ("random", "c_and_w")
NEIGHBORHOODS = ("transpose", "exchange", "insert")

RESULT_PATTERN = re.compile(
    r"RESULT\s+cost=(?P<cost>\d+)\s+time=(?P<time>[0-9.eE+-]+)\s+solution=(?<solution>[\d+\s]+)/gm"
)


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


def run_solver_once(
    binary_path: Path,
    instance_path: Path,
    pivot: str,
    neighborhood: list[str],
    sol_start: str,
    timeout_seconds: float,
) -> tuple[int, float, list[int]]:
    cmd = [
        str(binary_path),
        "-i",
        str(instance_path),
        "-p",
        pivot,
        "-s",
        sol_start,
    ]
    for neighb in neighborhood:
        cmd.extend(["-n", neighb])

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
    return cost, elapsed_seconds


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
        required=True,
        help="Number of runs per algo combination (instance, pivot, neighborhood, sol_start).",
    )
    parser.add_argument(
        "--output_it_imp",
        type=Path,
        default=Path("data/output/lop_analysis.csv"),
        help="Output CSV path.",
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
        default=Path("best_known/best_known.txt"),
        help="Path to the best-known instances file.",
    )
    parser.add_argument(
        "--instances-dir",
        type=Path,
        default=Path("instances"),
        help="Directory containing instance files.",
    )
    parser.add_argument(
        "--solution",
        type=bool,
        default=False,
        help="Whether to include the solution in the output CSV.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()

    if args.k <= 0:
        raise ValueError("--k must be greater than 0")
    if not args.binary.is_file():
        raise FileNotFoundError(f"Solver binary not found: {args.binary}")
    if not args.best_known_file.is_file():
        raise FileNotFoundError(f"Best-known file not found: {args.best_known_file}")
    if not args.instances_dir.is_dir():
        raise FileNotFoundError(f"Instances directory not found: {args.instances_dir}")

    instances = parse_best_known(args.best_known_file)
    if not instances:
        raise RuntimeError("No instances loaded from best-known file")

    combinations = list(itertools.product(PIVOTS, NEIGHBORHOODS, START_SOLS))
    total_runs = len(instances) * len(combinations) * args.k
    current_run = 0

    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w", encoding="utf-8", newline="") as csv_file:
        writer = csv.DictWriter(
            csv_file,
            fieldnames=[
                "instance",
                "best_known_cost",
                "pivot",
                "neighborhoods",
                "sol_start",
                "cost",
                "real_percent_deviationtime_s",
                "avg_gap_to_best",
                "solution",
            ],
        )
        writer.writeheader()

        for instance_name, best_known_cost in instances:
            instance_path = args.instances_dir / instance_name
            if not instance_path.is_file():
                raise FileNotFoundError(f"Instance file not found: {instance_path}")

            for pivot, neighborhood, sol_start in combinations:
                costs: list[int] = []
                times: list[float] = []
                solutions: list[list[int]] = []

                for _ in range(args.k):
                    current_run += 1
                    print(
                        f"[{current_run}/{total_runs}] {instance_name} "
                        f"p={pivot} n={neighborhood} s={sol_start}",
                        file=sys.stderr,
                    )
                    cost, elapsed, solution = run_solver_once(
                        binary_path=args.binary,
                        instance_path=instance_path,
                        pivot=pivot,
                        neighborhood=[neighborhood],
                        sol_start=sol_start,
                        timeout_seconds=args.timeout,
                    )
                    costs.append(cost)
                    times.append(elapsed)
                    if args.solution:
                        solutions.append(solution)

                avg_cost = statistics.fmean(costs)
                avg_time = statistics.fmean(times)

                writer.writerow(
                    {
                        "instance": instance_name,
                        "best_known_cost": best_known_cost,
                        "pivot": pivot,
                        "neighborhood": neighborhood,
                        "sol_start": sol_start,
                        "k": args.k,
                        "avg_cost": f"{avg_cost:.6f}",
                        "avg_time_s": f"{avg_time:.6f}",
                        "avg_gap_to_best": f"{avg_cost - best_known_cost:.6f}",
                    }
                )

    print(f"Wrote benchmark results to: {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
