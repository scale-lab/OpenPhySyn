# This should run from the root directory of OpenPhySyn
puts "Getting started sample, this should run from the root directory of OpenPhySyn (e.g. ./build/Psn ./tests/tcl/getting_started.tcl"
import_lib ./tests/data/libraries/Nangate45/NangateOpenCellLibrary_typical.lib
import_lef ./tests/data/libraries/Nangate45/NangateOpenCellLibrary.mod.lef
import_def ./tests/data/designs/timing_buffer/ibex_gp.def
read_sdc ./tests/data/designs/timing_buffer/ibex_tight_clk.sdc

set_wire_rc 1.0e-03 1.0e-03 ; # or set_wire_rc <metal layer>

puts "=============== Initial Reports ============="
report_checks
report_check_types -max_slew -max_capacitance -violators
puts "Capacitance violations: [llength [capacitance_violations]]"
puts "Transition violations: [llength [transition_violations]]"
report_wns
report_tns

puts "Initial area: [expr round([design_area] * 10E12) ] um2"

puts "OpenPhySyn timing repair:"
repair_timing

puts "=============== Final Reports ============="
report_checks
report_check_types -max_slew -max_capacitance -violators
puts "Capacitance violations: [llength [capacitance_violations]]"
puts "Transition violations: [llength [transition_violations]]"
report_wns
report_tns

puts "Final area: [expr round([design_area] * 10E12) ] um2"

puts "Export optimized design"
export_def optimized.def

exit 0