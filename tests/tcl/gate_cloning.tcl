read_lef ../tests/data/libraries/Nangate45/NangateOpenCellLibrary.mod.lef
read_def ../tests/data/designs/loaded/loaded4.def
read_lib ../tests/data/libraries/Nangate45/NangateOpenCellLibrary_typical.lib
puts "Read ok!"
set_wire_rc 0.0020 0.00020
# set_log_level debug
set num_cloned [transform gate_clone 1.5 true]
puts "Cloned $num_cloned"
write_def ./outputs/cloned.def
puts "Clone ok!"
exit 0
