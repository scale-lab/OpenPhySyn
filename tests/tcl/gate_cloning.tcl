read_lef ../tests/data/Nangate45/NangateOpenCellLibrary.mod.lef
read_def ../tests/data/design.def
read_lib ../tests/data/Nangate45/NangateOpenCellLibrary_typical.lib
puts "Read ok!"
transform gate_clone 1.4 true
# write_def ./outputs/buffered.def
puts "Clone ok!"
exit 0
