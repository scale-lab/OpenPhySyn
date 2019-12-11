# OpenPhySyn Gate Cloining Transform

Load-driven gate cloning transform using [OpenPhySyn](https://github.com/The-OpenROAD-Project/OpenPhySyn) psniscal synthesis tool

## Building

Build by making a build directory (i.e. `build/`), run `cmake ..` in that directory, and then use `make` to build the desired target.

Example:

```bash
> mkdir build && cd build
> cmake .. -DPSN_HOME=<OpenPhySyn Source Code Path> -DPSN_LIB=<OpenPhySyn Built Library Directory> \
> -DOPENDB_HOME=<OpenDB Source Code Directory> -DOPENDB_LIB=<OpenDB Built Library Directory> \
> -DOPENSTA_HOME=<OpenSTA Source Code Directory>
> make
> make install # Or sudo make install
```

## Usage

```bash
> ./Psn
> transform gate_clone 1.4 1
> export_def out.def
```

Make sure to set `PSN_HOME` to OpenPhySyn source code path, `PSN_LIB` to the directory containing the built OpenPhySyn library file, `OPENDB_HOME` to [OpenDB](https://github.com/The-OpenROAD-Project/OpenDB) include path, `OPENDB_LIB` the directory containing the built [OpenDB](https://github.com/The-OpenROAD-Project/OpenDB) library files.
