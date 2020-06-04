# OpenPhySyn Timing Buffer

Repair design timing and electrical violations through resizing, buffer insertion, and pin-swapping.

## Building

Build by making a build directory (i.e. `build/`), run `cmake ..` in that directory, and then use `make` to build the desired target.

Example:

```bash
> mkdir build && cd build
> cmake .. -DPSN_HOME=<OpenPhySyn Source Code Path> \
> -DOPENDB_HOME=<OpenDB Source Code Directory> \
> -DOPENSTA_HOME=<OpenSTA Source Code Directory>
> make
> make install # Or sudo make install
```

## Usage

```bash
> ./Psn
> import_lef <lef file>
> import_def <def file>
> transform repair_timing [-capacitance_violations] [-transition_violations] [-negative_slack_violations] [-iterations] [-buffers buffer_cells] [-inverters inverter cells] [-min_gain 0.0] [-area_penalty=0.0 unit_time/unit_area] [-auto_buffer_library] [-no_minimize_buffer_library] [-auto_buffer_library_inverters_enabled] [-buffer_disabled] [-minimum_cost_buffer_enabled] [-upsize_enabled] [-downsize_enabled] [-pin_swap_enabled] [-legalize_eventually] [-legalize_each_iteration] [-post_place|-post_route] [-legalization_frequency <num_edits>] [-fast]
> write_def out.def
```
