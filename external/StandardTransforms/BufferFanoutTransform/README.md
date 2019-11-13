# PhyKnight Buffer Fan-Out Transform

A simple buffering transform using [PhyKnight](https://github.com/The-OpenROAD-Project/PhyKnight) phyiscal synthesis tool

## Building

Build by making a build directory (i.e. `build/`), run `cmake ..` in that directory, and then use `make` to build the desired target.

Example:

```bash
> mkdir build && cd build
> cmake .. -DPHY_HOME=<PhyKnight Source Code Path> -DPHY_LIB=<PhyKnight Built Library Directory> \
> -DOPENDB_HOME=<OpenDB Source Code Directory> -DOPENDB_LIB=<OpenDB Built Library Directory> \
> -DOPENSTA_HOME=<OpenSTA Source Code Directory>
> make
> make install # Or sudo make install
```

## Usage

```bash
> ./Phy
> # Run simple buffering algorithm for cell with max-fanout 2
> transform buffer_fanout 2 BUF_X1 A Z clk
> write_def out.def
```

Make sure to set `PHY_HOME` to PhyKnight source code path, `PHY_LIB` to the directory containing the built PhyKnight library file, `OPENDB_HOME` to [OpenDB](https://github.com/The-OpenROAD-Project/OpenDB) include path, `OPENDB_LIB` the directory containing the built [OpenDB](https://github.com/The-OpenROAD-Project/OpenDB) library files.
