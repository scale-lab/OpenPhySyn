# PhyKnight

PhyKnight is a plugin-based physical synthesis optimization kit developed as part of the [OpenROAD](https://theopenroadproject.org/) flow.

## Building

Build by creating a build directory (i.e. `build/`), run `cmake ..` in that directory, and then use `make` to build the desired target.

Example:

```bash
> mkdir build && cd build
> cmake .. -DCMAKE_BUILD_TYPE=[Debug | Coverage | Release]
> make
> ./phy
> make test      # Makes and runs the tests.
> make coverage  # Generate a coverage report.
> make doc       # Generate html documentation.
```

## Building Custom Transforms

Physical Synthesis transforms libraries are loaded from the directory refered to by the variable `PHY_TRANSFORM_PATH`
To build a new transform refer to the transform [template](https://github.com/The-OpenROAD-Project/PhyKnightHelloTransform).

## Runing Transforms

To run any transorm simply:

```bash
> ./Phy
> read_lef <lef file>
> read_def <def file>
> transform <transform name> <arguments...>
> write_def <def file>
```

## Dependencies

PhyKnight depends on the following libraries:

-   [Boost](https://www.boost.org/)
-   [CPP TaskFlow](https://github.com/cpp-taskflow/cpp-taskflow)
-   [Flute](https://github.com/The-OpenROAD-Project/flute3)
-   [OpenSTA](https://github.com/The-OpenROAD-Project/OpenSTA)
-   [OpenDB](https://github.com/The-OpenROAD-Project/OpenDB)
-   [SWIG](http://www.swig.org/Doc1.3/Tcl.html)
-   [Doxygen](http://www.doxygen.nl) (optional)
-   [Doctests](https://github.com/onqtam/doctest) (optional)

## To-Do

-   [ ] Integrate Steiner Tree
-   [ ] Integrate Liberty Parser
-   [ ] Integrate OpenSTA
-   [ ] Support reading scripts from file
-   [ ] Add unit tests
-   [ ] Add API documentation
-   [ ] Add Optimization Transforms
-   [ ] Add Coding Guideline and Contribution Guide
