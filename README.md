# Heuristic Optimization: LOP

The project is made of three executables: `analysis.ipynb`, `benchmark_best_known.py` and `lop`. The first analyse the benchmark results, the second is the benchmark and de last one is the main executable to run the algorithms.

`analysis.ipynb` call `benchmark_best_known.py` to run the benchmark and then analyze the results, but `benchmark_best_known.py can also be called without analysis. It is the most complete way to analyze the results of the benchmark.

To run `anaylsis.ipynb` from the project directory, uv has to be installed.

```bash
uv sync
uv run --with jupyter jupyter lab
```

To run the full benchmark, only one command is needed (compilation included):

```bash
uv run tools/benchmark_best_known.py
```

Dependencies: [cmake](https://cmake.org/download/) and python

Use `--workers <n>` to run benchmark combinations in parallel processes (default to all CPUs minus one; use `1` to run sequentially).
If time measurement is important, run with the minimum number of workers.

## Project structure

- Instances are preferably places in `data/input/instances/`.
- `best_known.txt` associates each instance with its best known solution. It is used in the benchmark to compute the gap of the solutions found by the algorithms.

```tree
.
├── analysis.ipynb
├── build
│   ├── bin
│   │   ├── lop
│   │   └── lop_debug
├── CMakeLists.txt
├── data
│   ├── input
│   │   ├── best_known.txt
│   │   └── instances
│   │       ├── N-be75eec_150
│   │       ├── N-be75eec_250
   .       .
.   .       .
.   .       .
│   └── output
│       ├── figs
│       │   ├── it_imp_boxplot.svg
│       │   ├── it_imp_summary.svg
│       │   └── vnd_summary.svg
│       ├── it_im_pairwise_tests.csv
│       ├── it_im_results.csv
│       ├── it_im_summary_stats.csv
│       ├── lop_vnd_results.csv
│       ├── vnd_stat_test.csv
│       └── vnd_summary_stats.csv
├── doc
├── Makefile
├── README.md
├── src
├── tools
│   └── benchmark_best_known.py
```

## Compile
  
[cmake](https://cmake.org/) is required to compile this project.

```bash
#On the first compilation
cmake -S. -Bbuild
# To compile the target
cmake --build build
```

To clean the repo from all builds.

```bash
rm -rf build/
```

Only use the Makefile if there is compatibility issues with cmake.

## Run

For the full benchmark help :

```
```bash
python3 tools/benchmark_best_known.py --help
```

The binary (after compilation) is located in `./build/bin/`.

The target has the --help functionality.

```bash
./build/bin/lop --help
```

To use with default parameters:

```bash
./build/bin/lop
```

### examples

#### Iterative improvement lop

```bash
./build/bin/lop -i ./data/input/instances/N-be75eec_150 -s c_and_w -n exchange -p best
```

#### variable neighborhood descent lop

The difference with iterative improvement is just the number of `-n` called.

```bash
 ./build/bin/lop -i ./data/input/instances/N-be75eec_150 -s c_and_w -p first -n transpose -n exchange -n insert -v

```

#### iterated local search

```bash
 ./build/bin/lop -i ./data/input/instances/N-be75eec_150 -s c_and_w -p first -n transpose -n exchange -n insert -a ils -t 20 -r 0.1 -w 1000
```

## development

The debug target has flag and verbose compiled with it `(-Og)`. It is far more verbose and there is great number of high complexity asserts.

Build debug target:

```bash
#On the first compilation and each folder modification and file addition.
cmake -S. -Bbuild
# to compile the debug target
cmake --build build --target lop_debug
```

To run the debug:

```bash
./build/bin/lop_debug -i ./data/input/instances/N-be75eec_150 -s c_and_w -n insert -p best

```

## Documentation
