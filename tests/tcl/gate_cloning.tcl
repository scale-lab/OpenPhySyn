read_lef ../tests/data/libraries/Nangate45/NangateOpenCellLibrary.mod.lef
read_def ../tests/data/designs/gcd/gcd.def
read_lib ../tests/data/libraries/Nangate45/NangateOpenCellLibrary_typical.lib
puts "Read ok!"
sta create_clock [sta get_ports clk]  -name core_clock  -period 10
set_wire_rc 0.0020 0.00020
# set_log_level trace
set num_cloned [transform gate_clone 1.5 false]
puts "----"
sta report_checks
puts "Cloned $num_cloned gates"
write_def ./outputs/cloned.def
puts "Clone ok!"
exit 0
