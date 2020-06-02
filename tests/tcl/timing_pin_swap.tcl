import_lib ../tests/data/libraries/Nangate45/NangateOpenCellLibrary_typical.lib
import_lef ../tests/data/libraries/Nangate45/NangateOpenCellLibrary.mod.lef
import_def ../tests/data/designs/gcd/gcd.def
create_clock [get_ports clk] -name core_clock -period 10 
report_checks -digit 6
set_log_level debug
transform pin_swap 30
report_checks -digit 6
export_def ./outputs/buffered.def
exit 0
