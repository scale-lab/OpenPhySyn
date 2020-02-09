import_lib ../tests/data/libraries/Nangate45/NangateOpenCellLibrary_typical.lib
import_lef ../tests/data/libraries/Nangate45/NangateOpenCellLibrary.mod.lef
import_def ../tests/data/designs/constants/constants_simple.def
set propg [transform constant_propagation]
puts "Propagated $propg"
export_def ./outputs/propg.def

exit 0
