read_lib ../tests/data/libraries/Nangate45/NangateOpenCellLibrary_typical.lib
read_lef ../tests/data/libraries/Nangate45/NangateOpenCellLibrary.mod.lef
read_def ../tests/data/designs/gcd/gcd.def
sta create_clock [sta get_ports clk]  -name core_clock  -period 10
sta report_checks
exit 0