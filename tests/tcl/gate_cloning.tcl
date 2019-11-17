read_lef ../tests/data/Nangate45/NangateOpenCellLibrary.mod.lef
read_def ../tests/data/design.def
read_lib ../tests/data/Nangate45/NangateOpenCellLibrary_typical.lib
puts "Read ok!"
set_wire_rc 1.59 0.235146e-12
set_log_level debug
transform gate_clone 1.1 true
# write_def ./outputs/buffered.def
puts "Clone ok!"
exit 0
