read_lef ../tests/data/tech.lef
read_def ../tests/data/design.def
puts "Read ok!"
transform buffer_fanout 2 BUF_X1 A Z clk
write_def ./outputs/buffered.def
puts "Buffer ok!"
exit 0
