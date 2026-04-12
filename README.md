# Heuristic Optimization: LOP

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

## Run

The binary (after compilation) is located in `./build/bin/`.

The target has the --help functionality.

```bash
./build/bin/lop --help
```

To use with default parameters:

```bash
./build/bin/lop
```

Use example :

```bash
./build/bin/lop -i ./instances/N-test_10 -s c_and_w -n exchange -p best
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
