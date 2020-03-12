import_lib ../tests/data/libraries/Nangate45/NangateOpenCellLibrary_typical.lib
import_lef ../tests/data/libraries/Nangate45/NangateOpenCellLibrary.mod.lef
import_def ../tests/data/designs/timing_buffer/ibex_resized.def
sta read_sdc ../tests/data/designs/timing_buffer/ibex.sdc
sta report_checks
set_log_level debug

set_wire_rc metal2
set num_buffers [transform timing_buffer -buffers BUF_X4]
puts "Added $num_buffers buffers"
export_def ./outputs/buffered.def

exit 0
