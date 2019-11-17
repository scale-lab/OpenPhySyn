# OpenPhySyn

OpenPhySyn is a plugin-based physical synthesis optimization kit developed as part of the [OpenROAD](https://theopenroadproject.org/) flow.

## Building

Build by creating a build directory (i.e. `build/`), run `cmake ..` in that directory, and then use `make` to build the desired target.

Example:

```bash
> mkdir build && cd build
> cmake .. -DCMAKE_INSTALL_PREFIX=${HOME}/apps/OpenPhySyn -DCMAKE_BUILD_TYPE=[Debug | Coverage | Release]
> make
> make install
> ./Psn
> make test      # Makes and runs the tests.
> make coverage  # Generate a coverage report.
> make doc       # Generate html documentation.
```

## Building Custom Transforms

Physical Synthesis transforms libraries are loaded from the directory refered to by the variable `PSN_TRANSFORM_PATH`
To build a new transform refer to the transform [template](https://github.com/The-OpenROAD-Project/OpenPhySynHelloTransform).

## Runing Transforms

To run any transorm:

```bash
> ./Psn
> read_lef <lef file>
> read_def <def file>
> transform <transform name> <arguments...>
> write_def <def file>
```

To list available commands:
```bash
> ./Psn
> help
print_version                         print version
help                                  print help
print_transforms                      list loaded transforms
read_lef <file path>                  load LEF file
read_def <file path>                  load DEF file
read_lib <file path>                  load a liberty file
read_liberty <file path>              load a liberty file
write_def <output file>               Write DEF file
set_wire_rc <res> <cap>               Set resistance & capacitance per micron
set_max_area <area>                   Set maximum design area
transform <transform name> <args>     Run transform on the loaded design
link_design <design name>             Link design top module
sta <OpenSTA commands>                Run OpenSTA commands
set_log <log level>                   Set log level [trace, debug, info, warn, error, critical, off]
set_log_pattern <pattern>             Set log printing pattern, refer to spdlog logger for pattern formats

```

To list loaded transforms:
```bash
> ./Psn
> print_transforms
```

To get usage info of any transform:
```bash
> ./Psn
> transform <transform name> help
```

To run OpenSTA commands:
```bash
> ./Psn
> read_lef <lef file>
> read_def <def file>
> sta create_clock [sta get_ports clk]  -name core_clock  -period 10
> sta report_checks
```

## Default Transforms

By default, the following transforms are built with OpenPhySyn:

-   `hello_transform`: a demo transform that adds a random wire.
-   `buffer_fanout`: adds buffers to high fan-out nets.
-   `gate_clone`: performs load driven gate cloning.

## Dependencies

OpenPhySyn depends on the following libraries:

-   [Boost](https://www.boost.org/)
-   [CPP TaskFlow](https://github.com/cpp-taskflow/cpp-taskflow)
-   [Flute](https://github.com/The-OpenROAD-Project/flute3)
-   [OpenSTA](https://github.com/The-OpenROAD-Project/OpenSTA)
-   [OpenDB](https://github.com/The-OpenROAD-Project/OpenDB)
-   [SWIG](http://www.swig.org/Doc1.3/Tcl.html)
-   [Doxygen](http://www.doxygen.nl) (optional)
-   [Doctests](https://github.com/onqtam/doctest) (optional)

## Issues

Please open a GitHub [issue](https://github.com/The-OpenROAD-Project/OpenPhySyn/issues/new) if you find any bugs.

## To-Do

-   [x] Integrate Steiner Tree
-   [x] Integrate Liberty Parser
-   [x] Integrate OpenSTA
-   [x] Support reading scripts from file
-   [x] Add unit tests
-   [x] Add Gate Cloning Transform
-   [ ] Fix issues with OpenSTA commands integration
-   [ ] Add API documentation
-   [ ] Expose lower-level APIs through TCL/Python interface
-   [ ] Add Optimization Transforms
-   [ ] Add Coding Guideline and Contribution Guide
