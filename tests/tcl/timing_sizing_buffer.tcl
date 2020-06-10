import_lib ../tests/data/libraries/Nangate45/NangateOpenCellLibrary_typical.lib
import_lef ../tests/data/libraries/Nangate45/NangateOpenCellLibrary.mod.lef
import_def ../tests/data/designs/timing_buffer/ibex_resized.def
# import_def ../tests/data/designs/timing_buffer/ibex_gp.def
# import_db ibex.db
sta read_sdc ../tests/data/designs/timing_buffer/ibex.sdc

set_wire_rc metal3
repair_timing -buffers {CLKBUF_X1 BUF_X1 BUF_X2 BUF_X4} -enable_gate_resize -area_penalty 0.5


export_def ./outputs/buffered.def

exit 0
