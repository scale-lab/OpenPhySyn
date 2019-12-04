R"===<><>===(

proc transform {transform_name args} {
    psn::transform_internal $transform_name $args
}

namespace eval psn {
    sta::define_cmd_args "optimize_design" {
        [-clone_max_cap_factor factor] \
        [-clone_non_largest_cells]
    }
    
    proc optimize_design { args } {
        sta::parse_key_args "optimize_design" args \
            keys {-clone_max_cap_factor} \
            flags {-clone_non_largest_cells}
        
        set clone_max_cap_factor 1.5
        set clone_largest_cells_only true

        if {[info exists keys(-clone_max_cap_factor)]} {
            set clone_max_cap_factor $keys(-clone_max_cap_factor)
        }
        if {[info exists flags(-clone_non_largest_cells)]} {
            set clone_largest_cells_only false
        }

        transform gate_clone $clone_max_cap_factor $clone_largest_cells_only
    }


    sta::define_cmd_args "optimize_fanout" {
        -buffer_cell buffer_cell_name \
        -buffer_input_port port \
        -buffer_output_port port \
        -max_fanout max_fanout
    }

    proc optimize_fanout { args } {
        sta::parse_key_args "optimize_fanout" args \
            keys {-buffer_cell -buffer_input_port -buffer_output_port -max_fanout} \
            flags {}
        if { ![info exists keys(-buffer_cell)] \
          || ![info exists keys(-buffer_input_port)] \
          || ![info exists keys(-buffer_output_port)] \
          || ![info exists keys(-max_fanout)]
         } {
            sta::cmd_usage_error "optimize_fanout"
        }
        set cell $keys(-buffer_cell)
        set inport $keys(-buffer_input_port)
        set outport $keys(-buffer_output_port)
        set max_fanout $keys(-max_fanout)
        transform buffer_fanout $max_fanout $cell $inport $outport
    }
    namespace export *
}
namespace import psn::*

)===<><>==="