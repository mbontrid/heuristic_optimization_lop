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

#### development

The debug target has flag and verbose compiled with it.

```bash
#On the first compilation and each folder modification and file addition.
cmake -S. -Bbuild
# to compile the debug target
cmake --build build --target lop_debug
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

## Documentation
