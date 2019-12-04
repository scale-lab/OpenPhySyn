read_lib ../tests/data/libraries/Nangate45/NangateOpenCellLibrary_typical.lib
read_lef ../tests/data/libraries/Nangate45/NangateOpenCellLibrary.mod.lef
read_def ../tests/data/designs/fanout/fanout_nan.def
sta create_clock [sta get_ports clk] -name core_clock -period 10
puts "Read ok!"
transform buffer_fanout 2 BUF_X1 A Z clk
write_def ./outputs/buffered.def
puts "Buffer ok!"
exit 0
