read_lef ../tests/data/tech.lef
read_def ../tests/data/design.def
puts "Read ok!"
transform hello_transform buffer 2 BUF_X1 A Z clk
puts "Bufer ok!"
exit 0