set me [file normalize [info script]]
puts $me
read_lef ../tests/data/tech.lef
read_def ../tests/data/design.def
puts "Ok!"