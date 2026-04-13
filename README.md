# Heuristic Optimization: LOP

To run the full benchmark, only one command is needed:

```bash
python3 tools/benchmark_best_known.py
```

Dependencies: [cmake](https://cmake.org/download/) and python

Warning: the full benchmark takes hours.
Use `--workers <n>` to run benchmark combinations in parallel processes (defaults to all CPUs minus one; use `1` to run sequentially).
If time measurement is important, run with the minimum number of workers.

## Project structure

- Instances are preferably places in `data/input/instances/`.
- `best_known.txt` associates each instance with its best known solution. It is used in the benchmark to compute the gap of the solutions found by the algorithms.

```tree
.
├── build
│   ├── bin
│   │   ├── lop
│   │   └── lop_debug
├── CMakeLists.txt
├── data
│   ├── best_known.txt
│   ├── input
│   │   └── instances
│   │       ├── N-be75eec_150
.   .       .
.   .       .
.   .       .
│   └── output
│       ├── lop_analysis.csv
│       └── lop_vnd_analysis.csv
├── doc
├── include
├── Makefile
├── README.md
├── src
├── tools
    └── benchmark_best_known.py
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
./build/bin/lop -i ./instances/N-be75eec_150 -s c_and_w -n exchange -p best
```

#### variable neighborhood descent lop

The difference with iterative improvement is just the number of `-n` called.

```bash
 ./build/bin/lop -i ./data/input/instances/N-be75eec_150 -s c_and_w -p first -n transpose -n exchange -n insert -v
```

```
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
./build/bin/lop_debug -i ./instances/N-be75eec_150 -s c_and_w -n insert -p best

```

To illustrate the difference of time between the lop executable and the lop_debug executable:

```bash
./build/bin/lop_debug -i ./instances/N-be75eec_150 -s c_and_w -n exchange -p best -v
```

Takes 130 seconds

```bash
./build/bin/lop -i ./instances/N-be75eec_150 -s c_and_w -n exchange -p best -v
```

Takes 0.88 seconds.

## Documentation
