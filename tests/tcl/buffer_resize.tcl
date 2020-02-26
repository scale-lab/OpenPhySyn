import_lib ../tests/data/libraries/Nangate45/NangateOpenCellLibrary_typical.lib
import_lef ../tests/data/libraries/Nangate45/NangateOpenCellLibrary.mod.lef
# import_def ../tests/data/designs/ibex/ibex.def
import_def ../tests/data/designs/aes_drv/aes.def
sta create_clock [sta get_ports clk] -name core_clock -period 5
# sta create_clock [sta get_ports clk_i] -name core_clock -period 5
sta report_checks -path_delay min_max
sta report_check_types -all_violators
transform buffer_resize -buffers BUF_X4
# export_def ./outputs/buffered.def
exit 0
