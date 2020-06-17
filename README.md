# OpenPhySyn

OpenPhySyn is a physical synthesis optimization kit developed at [Brown University SCALE lab](http://scale.engin.brown.edu) as part of the [OpenROAD](https://theopenroadproject.org/) flow.

## Building

Clone and build OpenPhySyn using the following commands:

```sh
git clone --recursive https://github.com/scale-lab/OpenPhySyn.git
cd OpenPhySyn
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=[Debug | Coverage | Release]
make
make install # Or sudo make install
Psn
make test # Runs the unit tests
make coverage # Generate a coverage report.
```

## Getting Started

### Read design and run a transform:

```tcl
Psn
import_lef <lef file>
import_def <def file>
transform <transform name> <arguments...>
export_def <def file>
```

### Example: Repair design timing violations

```tcl
Psn

import_lib ./tests/data/libraries/Nangate45/NangateOpenCellLibrary_typical.lib
import_lef ./tests/data/libraries/Nangate45/NangateOpenCellLibrary.mod.lef
import_def ./tests/data/designs/timing_buffer/ibex_gp.def
sta read_sdc ./tests/data/designs/timing_buffer/ibex_tight_clk.sdc

set_wire_rc 1.0e-03 1.0e-03 ; # or set_wire_rc <metal layer>

puts "=============== Initial Reports ============="
report_checks
report_check_types -max_slew -max_capacitance -violators
puts "Capacitance violations: [llength [capacitance_violations]]"
puts "Transition violations: [llength [transition_violations]]"
report_wns
report_tns

puts "Initial area: [expr round([design_area] * 10E12) ] um2"

repair_timing -fast -capacitance_violations -transition_violations -negative_slack_violations -iterations 1 -auto_buffer_library medium -upsize_enabled -pin_swap_enabled

puts "=============== Final Reports ============="
report_checks
report_check_types -max_slew -max_capacitance -violators
puts "Capacitance violations: [llength [capacitance_violations]]"
puts "Transition violations: [llength [transition_violations]]"
report_wns
report_tns

puts "Final area: [expr round([design_area] * 10E12) ] um2"

exit 0
```

### List available commands:

```tcl
> Psn
> help
design_area			Report design total cell area
export_db			Export OpenDB database file
export_def			Export design DEF file
gate_clone			Perform load-driven gate cloning
get_database			Return OpenDB database object
get_database_handler		Return OpenPhySyn database handler
get_handler			Alias for get_database_handler
get_liberty			Return first loaded liberty file
has_transform			Check if the specified transform is loaded
help				Print this help
import_db			Import OpenDB database file
import_def			Import design DEF file
import_lef			Import technology LEF file
import_lib			Alias for import_liberty
import_liberty			Import liberty file
link				Alias for link_design
link_design			Link design top module
make_steiner_tree		Create steiner tree around net
optimize_design			Perform timing optimization
optimize_fanout			Perform maximum-fanout based buffering
optimize_logic			Perform logic optimization
optimize_power			Perform power optimization
pin_swap			Perform timing optimization by commutative pin swapping
print_liberty_cells		Print liberty cells available in the  loaded library
print_license			Print license information
print_transforms		Print loaded transforms
print_usage			Print usage instructions
print_version			Print tool version
propagate_constants		Perform logic optimization by constant propgation
timing_buffer			Repair violations through buffer tree insertion
repair_timing			Repair design timing and electrical violations through resizing, buffer insertion, and pin-swapping
capacitance_violations		Print pins with capacitance limit violation
transition_violations		Print pins with transition limit violation
set_log				Alias for set_log_level
set_log_level			Set log level [trace, debug, info, warn, error, critical, off]
set_log_pattern			Set log printing pattern, refer to spdlog logger for pattern formats
set_max_area			Set maximum design area
set_wire_rc			Set wire resistance/capacitance per micron, you can also specify technology layer
transform			Run loaded transform
version				Alias for print_version
```

### List loaded transforms:

```tcl
Psn
print_transforms
```

### Print usage information for a transform:

```tcl
Psn
transform <transform name> help
```

### Running OpenSTA commands:

```tcl
Psn
import_lef <lef file>
import_def <def file>
sta create_clock [sta get_ports clk] -name core_clock -period 10
sta report_checks
```

## Default Transforms

By default, the following transforms are built with OpenPhySyn:

-   `buffer_fanout`: adds buffers to high fan-out nets.
-   `gate_clone`: performs load driven gate cloning.
-   `pin_swap`: performs timing-driven/power-driven commutative pin-swapping optimization.
-   `constant_propagation`: perform constant propagation optimization across the design hierarchy.
-   `timing_buffer`: perform van Ginneken based buffer tree insertion to fix capacitance and transition violations.
-   `repair_timing`: repair design timing and electrical violations through resizing, buffer insertion, and pin-swapping.

## Fixing Timing Violations

The `repair_timing` command repairs negative slack, maximum capacitance and transition violations by buffer tree insertion, gate sizing, and pin-swapping.

`repair_timing` options:

-   `[-capacitance_violations]`: Repair capacitance violations.
-   `[-transition_violations]`: Repair transition violations.
-   `[-negative_slack_violations]`: Repair paths with negative slacks.
-   `[-iterations]`: Maximum number of iterations.
-   `[-buffers buffer_cells]`: Manually specify buffer cells to use.
-   `[-inverters inverter cells]`: Manually specify inverter cells to use.
-   `[-min_gain <unit_time>]`: Minimum slack gain to accept an optimization.
-   `[-auto_buffer_library <single|small|medium|large|all>]`: Auto-select buffer library.
-   `[-no_minimize_buffer_library]`: Do not run initial pruning phase for buffer selection.
-   `[-auto_buffer_library_inverters_enabled]`: Include inverters in the selected buffer library.
-   `[-buffer_disabled]`: Disable all buffering.
-   `[-pessimism_factor factor]` Scaling factor for transition and capacitance violation limits, default is 1.0, should be non-negative, < 1.0 is pessimistic, 1.0 is ideal, > 1.0 is optimistic.
-   `[-minimum_cost_buffer_enabled]`: Enable minimum cost buffering.
-   `[-upsize_enabled]`: Enable repair by upsizing.
-   `[-pin_swap_enabled]`: Enable pin-swapping.
-   `[-legalize_eventually]`: Legalize at the end of the optimization.
-   `[-legalize_each_iteration]`: Legalize after each iteration.
-   `[-post_place|-post_route]`: Post-placement phase mode or post-routing phase mode (not currently supported).
-   `[-legalization_frequency <num_edits>]`: Legalize after how many edits.
-   `[-fast]`: Trade-off runtime versus optimization quality by aggressive pruning.

## Example Code

Refer to the provided [tests directory](https://github.com/scale-lab/OpenPhySyn/tree/master/tests) for C++ example code.

You can also refer to the [tcl tests directory](https://github.com/scale-lab/OpenPhySyn/tree/master/tests/tcl) for examples for using the Tcl API.

## Building Custom Transforms

Physical Synthesis transforms libraries are loaded from the directory referred to by the variable `PSN_TRANSFORM_PATH`, defaulting to `./transforms`.

To build a new transform, refer to the transform [template](https://github.com/scale-lab/OpenPhySynHelloTransform).

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
