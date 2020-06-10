import_lib ../tests/data/libraries/Nangate45/NangateOpenCellLibrary_typical.lib
import_lef ../tests/data/libraries/Nangate45/NangateOpenCellLibrary.mod.lef
import_def ../tests/data/designs/gcd/gcd.def
sta create_clock [sta get_ports clk]  -name core_clock  -period 10

set_wire_rc 0.0020 0.00020
set num_cloned [gate_clone -clone_max_cap_factor 1.5 -clone_non_largest_cells]
puts "Cloned $num_cloned gates"
export_def ./outputs/cloned.def

exit 0
