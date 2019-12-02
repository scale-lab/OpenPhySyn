read_lef ../tests/data/libraries/Nangate45/NangateOpenCellLibrary.mod.lef
read_def ../tests/data/designs/gcd/gcd.def
read_lib ../tests/data/libraries/Nangate45/NangateOpenCellLibrary_typical.lib
# set_wire_rc 0.0020 0.00020
sta create_clock [sta get_ports clk]  -name core_clock  -period 10
sta report_checks
exit 0