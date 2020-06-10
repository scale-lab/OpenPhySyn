import_lib ../tests/data/libraries/Nangate45/NangateOpenCellLibrary_typical.lib
import_lef ../tests/data/libraries/Nangate45/NangateOpenCellLibrary.mod.lef
import_def ../tests/data/designs/timing_buffer/ibex_gp.def
sta read_sdc ../tests/data/designs/timing_buffer/ibex_tight_clk.sdc

set_wire_rc 1.0e-03 1.0e-03 ; # or set_wire_rc <metal layer>

puts "=============== Initial Reports ============="
report_checks
report_check_types -max_slew -max_capacitance -violators
puts "Capacitance violations: [llength [capacitance_violations]]"
puts "Transition violations: [llength [transition_violations]]"
report_wns
report_tns

puts "Initial area: [expr round([design_area] * 10E12) ] um2"

repair_timing -fast -capacitance_violations -transition_violations -negative_slack_violations -iterations 1 -auto_buffer_library medium -upsize_enabled -pin_swap_enabled

puts "=============== Final Reports ============="
report_checks
report_check_types -max_slew -max_capacitance -violators
puts "Capacitance violations: [llength [capacitance_violations]]"
puts "Transition violations: [llength [transition_violations]]"
report_wns
report_tns

puts "Final area: [expr round([design_area] * 10E12) ] um2"

exit 0
