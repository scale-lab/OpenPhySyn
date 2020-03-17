import_lib ../tests/data/libraries/Nangate45/NangateOpenCellLibrary_typical.lib
import_lef ../tests/data/libraries/Nangate45/NangateOpenCellLibrary.mod.lef
import_def ../tests/data/designs/timing_buffer/ibex_resized.def
# import_db ibex.db
sta read_sdc ../tests/data/designs/timing_buffer/ibex.sdc

set_wire_rc metal3
set num_buffers [transform timing_buffer -buffers CLKBUF_X1 CLKBUF_X2 CLKBUF_X3 BUF_X1 BUF_X2 BUF_X4 -inverters INV_X1 INV_X2 INV_X4]
puts "Added $num_buffers buffers"

export_def ./outputs/buffered.def

exit 0
