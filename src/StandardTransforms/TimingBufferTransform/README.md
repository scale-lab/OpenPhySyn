# OpenPhySyn Timing Buffer

Fix design rule violation by RAT-based buffer tree insertion.

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
> transform timing_buffer [buffers -all|<set of buffers>] [inverters "
    "-all|<set of inverters>] [enable_gate_resize] [enable_inverter_pair]
> write_def out.def
```
