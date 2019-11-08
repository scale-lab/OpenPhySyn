# PhyKnight Buffer Fan-Out Transform

A simple buffering transform using [PhyKnight](https://github.com/The-OpenROAD-Project/PhyKnight) phyiscal synthesis tool

## Building

Build by making a build directory (i.e. `build/`), run `cmake ..` in that directory, and then use `make` to build the desired target.

Example:

```bash
> mkdir build && cd build
> export PHY_HOME_PATH=<PhyKnight Source Code Path>
> export PHY_LIB_PATH=<PhyKnight Built Library Directory>
> export OPENDB_HOME_PATH=<OpenDB Source Code Directory>
> export OPENDB_LIB_PATH=<OpenDB Built Library Directory>
> export OPENDB_STA_PATH=<OpenSTA Source Code Directory>
> cmake ..
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

Make sure to set `PHY_HOME_PATH` to PhyKnight source code path, `PHY_LIB_PATH` to the directory containing the built PhyKnight library file, `OPENDB_HOME_PATH` to [OpenDB](https://github.com/The-OpenROAD-Project/OpenDB) include path, `OPENDB_LIB_PATH` the directory containing the built [OpenDB](https://github.com/The-OpenROAD-Project/OpenDB) library files.
