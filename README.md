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
> make test # Makes and runs the tests.
> make coverage # Generate a coverage report.
> make doc # Generate html documentation.
```

## Building Custom Transforms

Physical Synthesis transforms libraries are loaded from the directory referred to by the variable `PSN_TRANSFORM_PATH`, defaulting to `./transforms`.

To build a new transform, refer to the transform [template](https://github.com/scale-lab/OpenPhySynHelloTransform).

## Getting Started

### Read design and run a transorm:

```bash
> ./Psn
> import_lef <lef file>
> import_def <def file>
> transform <transform name> <arguments...>
> export_def <def file>
```

### List available commands:

```
> ./Psn
> help
design_area		Report design total cell area
export_db		Export OpenDB database file
export_def		Export design DEF file
get_database		Return OpenDB database object
get_database_handler	Return OpenPhySyn database handler
get_handler		Alias for get_database_handler
get_liberty		Return first loaded liberty file
has_transform		Check if the specified transform is loaded
help			Print this help
import_db		Import OpenDB database file
import_def		Import design DEF file
import_lef		Import technology LEF file
import_lib		Alias for import_liberty
import_liberty		Import liberty file
link			Alias for link_design
link_design		Link design top module
make_steiner_tree	Create steiner tree around net
optimize_design		Perform timing optimization
optimize_fanout		Perform maximum-fanout based buffering
optimize_logic		Perform logic optimization
optimize_power		Perform power optimization
print_liberty_cells	Print liberty cells available in the loaded library
print_license		Print license information
print_transforms	Print loaded transforms
print_usage		Print usage instructions
print_version		Print tool version
set_log			Alias for set_log_level
set_log_level		Set log level [trace, debug, info, warn, error, critical, off]
set_log_pattern	        Set log printing pattern, refer to spdlog logger for pattern formats
set_max_area		Set maximum design area
set_wire_rc		Set wire resistance/capacitance per micron, you can also specify technology layer
transform		Run loaded transform
version			Alias for print_version
```

### List loaded transforms:

```bash
> ./Psn
> print_transforms
```

### Print usage information for a transform:

```bash
> ./Psn
> transform <transform name> help
```

### Running OpenSTA commands:

```bash
> ./Psn
> import_lef <lef file>
> import_def <def file>
> sta create_clock [sta get_ports clk] -name core_clock -period 10
> sta report_checks
```

## Default Transforms

By default, the following transforms are built with OpenPhySyn:

-   `hello_transform`: a demo transform that adds a random wire.
-   `buffer_fanout`: adds buffers to high fan-out nets.
-   `gate_clone`: performs load driven gate cloning.
-   `pin_swap`: performs timing-driven/power-driven commutative pin-swapping optimization.
-   `constant_propagation`: Perform constant propagation optimization across the design hierarchy.
-   `timing_buffer`: Perform van Ginneken based buffer tree insertion to fix capacitance and transition violations.

# Fixing timing violations

The `repair_timing` command repairs maximum capacitance and transition time violations by buffer tree insertion.

`repair_timing` options:

-   **-buffers {buffer cells}**: Specify a list of non-inverting buffer cells to use.
-   **-inverters {inverter cells}**: Specify a list of inverting buffer cells to use.
-   **-iterations {iterations=1}**: Specify the maximum number of iterations to fix violations.
-   **-min_gain {gain=0.0}**: Minimum slack gain to accept a buffering option.
-   **-enable_gate_resize**: Enable driver sizing.
-   **-area_penalty {penalty area/time=0.0}**: When used with **-enable_gate_resize** penalizes solutions that cause area increase.

## Optimization Commands

The main provided command for optimization are `optimize_design` for physical design optimization and `optimize_logic`.

Currently, `optimize_design` performs pin swapping and load-driven gate-cloning to enhance the design timing. `optimize_logic` performs constant propagation optimization across the design hierarchy.

`optimize_design` options:

-   `[-no_gate_clone]`: Disable gate-cloning.
-   `[-no_pin_swap]`: Disable pin-swap.
-   `[-clone_max_cap_factor <factor>]`: Set gate-cloning capacitance load ratio, defaults to _1.5_.
-   `[-clone_non_largest_cells]`: Allow cloning of cells that are not the largest of their cell-footprint, not recommended.

`optimize_logic` options:

-   `[-no_constant_propagation]`: Disable constant propagation.
-   `[-tihi tihi_cell_name]`: Manually specify the Logic 1 cell for constant propagation.
-   `[-tilo tilo_cell_name]`: Manually specify the Logic 0 cell for constant propagation.

## Dependencies

OpenPhySyn depends on the following libraries:

-   [CPP TaskFlow](https://github.com/cpp-taskflow/cpp-taskflow) [included, optional]
-   [Flute](https://github.com/The-OpenROAD-Project/flute3) [included]
-   [OpenSTA](https://github.com/The-OpenROAD-Project/OpenSTA) [included]
-   [OpenDB](https://github.com/The-OpenROAD-Project/OpenDB) [included]
-   [cxxopts](https://github.com/jarro2783/cxxopts) [included]
-   [SWIG](http://www.swig.org/Doc1.3/Tcl.html)
-   [Doxygen](http://www.doxygen.nl) [included, optional]
-   [Doctests](https://github.com/onqtam/doctest) [included, optional]

## Issues

Please open a GitHub [issue](https://github.com/scale-lab/OpenPhySyn/issues/new) if you find any bugs.

## To-Do

-   [ ] Add API documentation
-   [ ] Add Coding Guideline and Contribution Guide
