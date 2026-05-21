# Heuristic Optimization: LOP

This project implement different algorithms to find good solutions to the linear ordering problem (LOP).

Available algorithms:

- iterative improvement
- variable neighborhood descent
- iterated local search with perturbation
- memetic algorithm (population based with crossover and mutation)

The project is made of three executables: `analysis.ipynb`, `benchmark_best_known.py` and `lop`.

`analysis.ipynb` call `benchmark_best_known.py` to run the benchmark and then analyze the results.

Dependencies: [cmake](https://cmake.org/download/) and [uv](https://uvlang.io/).

Direct analysis:
(this can take hours if there is no results csv in data/output/)

```bash
uv run --with jupyter jupyter-lab analysis/analysis.ipynb
```

Benchmark use `--workers <n>` to run benchmark combinations in parallel processe (default to all CPUs minus one; use `1` to run sequentially).
If time measurement is important, run with the minimum number of workers.

## Project structure

- Instances are preferably places in `data/input/instances/`.
- `best_known.txt` associates each instance with its best known solution. It is used in the benchmark to compute the gap of the solutions found by the algorithms.

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

The target has the --help functionality.

```bash
./build/bin/lop --help
```

benchmark_best_know.py has also help :

```bash
uv run analysis/benchmark_best_known.py --help
```

### examples lop executable

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
./build/bin/lop -i ./data/input/instances/N-be75eec_150 -s c_and_w -p best -n transpose -n exchange -n insert --algo ils --ils_n_try=100 --ils_perturb_rate 0.1 --ils_worst 10 -v
```

#### memetic algorithm

```bash
./build/bin/lop -i ./data/input/instances/N-be75eec_150 -s random -p first -n transpose -n exchange -n insert -a memetic --meme_pop=20 --meme_offspring=10 --meme_mut_rate=0.1 --meme_mean_try=5 --meme_divers_try=10 --meme_cross_rate=0.5 --meme_cross_rate_mut=0.8 -v
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
