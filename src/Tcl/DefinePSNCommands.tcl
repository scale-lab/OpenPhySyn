R"===<><>===(



namespace eval psn {
    proc define_cmd_args { cmd arglist } {
        sta::define_cmd_args $cmd $arglist
        namespace export $cmd
    }
    
    define_cmd_args "transform" {transform_name args}
    proc transform {transform_name args} {
        psn::transform_internal $transform_name $args
    }

    define_cmd_args "import_lef" {[-tech] [-library] filename}
    proc import_lef { args } {
       sta::parse_key_args "import_lef" args \
            keys {} \
            flags {-tech -library}
            set has_tech [info exists flags(-tech)]
            set has_lib [info exists flags(-library)]
            if {[llength $args] != 1} {
                sta::cmd_usage_error "import_lef"
                return
            }
            set filename [lindex $args 0]
            if {($has_tech && $has_lib) || (!$has_tech && !$has_lib)} {
                psn::import_lef_tech_sc $filename
            } elseif {$has_tech} {
                psn::import_lef_tech $filename
            } elseif {$has_lib} {
                psn::import_lef_sc $filename
            }
    }

    define_cmd_args "optimize_design" {\
        [-no_gate_clone] \
        [-no_pin_swap] \
        [-clone_max_cap_factor factor] \
        [-clone_non_largest_cells] \
    }
    
    proc optimize_design { args } {
        sta::parse_key_args "optimize_design" args \
            keys {-clone_max_cap_factor} \
            flags {-clone_non_largest_cells -no_gate_clone -no_pin_swap}

        set do_pin_swap true
        set do_gate_clone true
        if {[info exists flags(-no_gate_clone)]} {
            set do_gate_clone false
        }
        if {[info exists flags(-no_pin_swap)]} {
            set do_pin_swap false
        }
        if {![has_transform gate_clone]} {
            set do_gate_clone false
        }
        if {![has_transform pin_swap]} {
            set do_pin_swap false
        }
        set num_swapped 0
        set num_cloned 0
        if {$do_pin_swap} {
            set num_swapped [transform pin_swap]
            if {$num_swapped < 0} {
                return $num_swapped
            }
        }
        if {$do_gate_clone} {
            set clone_max_cap_factor 1.5
            set clone_largest_cells_only true

            if {[info exists keys(-clone_max_cap_factor)]} {
                set clone_max_cap_factor $keys(-clone_max_cap_factor)
            }
            if {[info exists flags(-clone_non_largest_cells)]} {
                set clone_largest_cells_only false
            }

            set num_cloned [transform gate_clone $clone_max_cap_factor $clone_largest_cells_only]
            if {$num_cloned < 0} {
                return $num_cloned
            }
        }
        return 1
    }
    
    define_cmd_args "optimize_logic" {\
        [-no_constant_propagation] \
        [-tiehi tiehi_cell_name] \
        [-tielo tielo_cell_name] \
    }

    proc optimize_logic { args } {
        sta::parse_key_args "optimize_power" args \
            keys {-tiehi tielo} \
            flags {-no_constant_propagation}
        set do_constant_propagation true
        set tiehi_cell_name ""
        set tielo_cell_name ""
        set max_prop_depth -1
        if {[info exists flags(-no_constant_propagation)]} {
            set no_constant_propagation false
        }
        if {![has_transform constant_propagation]} {
            set do_constant_propagation false
        }
        if {$do_constant_propagation} {
            if {[info exists keys(-tiehi)]} {
                set tiehi_cell_name $keys(-tiehi)
            }
            if {[info exists keys(-tielo)]} {
                set tielo_cell_name $keys(-tielo)
            }
            set propg [transform constant_propagation true $max_prop_depth $tiehi_cell_name $tielo_cell_name]
            if {$propg < 0} {
                return $propg
            }
        }
        return 0
    }

    define_cmd_args "optimize_power" {\
        [-no_pin_swap] \
        [-pin_swap_paths path_count] \
    }
    proc optimize_power { args } {
        sta::parse_key_args "optimize_power" args \
            keys {-pin_swap_paths} \
            flags {-no_pin_swap}

        set do_pin_swap true

        if {[info exists flags(-no_pin_swap)]} {
            set do_pin_swap false
        }
        if {![has_transform pin_swap]} {
            set do_pin_swap false
        }

        set num_swapped 0

        if {$do_pin_swap} {
            set pin_swap_paths 50
            if {[info exists keys(-pin_swap_paths)]} {
                set pin_swap_paths $keys(-pin_swap_paths)
            }
            set num_swapped [transform pin_swap true $pin_swap_paths]

            if {$num_swapped < 0} {
                return $num_swapped
            }
        }
        return 1
    }


    define_cmd_args "optimize_fanout" { \
        -buffer_cell buffer_cell_name \
        -max_fanout max_fanout \
    }

    proc optimize_fanout { args } {
        sta::parse_key_args "optimize_fanout" args \
            keys {-buffer_cell -max_fanout} \
            flags {}
        if { ![info exists keys(-buffer_cell)] \
          || ![info exists keys(-max_fanout)]
         } {
            sta::cmd_usage_error "optimize_fanout"
        }
        set cell $keys(-buffer_cell)
        set max_fanout $keys(-max_fanout)
        transform buffer_fanout $max_fanout $cell
    }

    define_cmd_args "cluster_buffers" {[-cluster_threshold diameter] [-cluster_size single|small|medium|large|all]}
    proc cluster_buffers { args } {
        sta::parse_key_args "cluster_buffers" args \
        keys {-cluster_threshold -cluster_size} \
        flags {}

        if {![psn::has_liberty]} {
            sta::sta_error "No liberty filed is loaded"
            return
        }

        if {[info exists keys(-cluster_threshold)] && [info exists keys(-cluster_size)]} {
            sta::sta_error "You can only specify either -cluster_threshold or -cluster_size"
            return
        } elseif {![info exists keys(-cluster_threshold)] && ![info exists keys(-cluster_size)]} {
            sta::cmd_usage_error "cluster_buffers"
            return
        }
        if {[info exists keys(-cluster_threshold)]} {
            if {($keys(-cluster_threshold) < 0.0) || ($keys(-cluster_threshold) > 1.0)} {
                sta::sta_error "-cluster_threshold should be between 0.0 and 1.0"
                return
            }
            return [psn::cluster_buffer_names $keys(-cluster_threshold) true]
        } else {
            set size $keys(-cluster_size)
            set valid_lib_size [list "single" "small" "medium" "large" "all"]
            if {[lsearch -exact $valid_lib_size $size] < 0} {
                sta::sta_error "Invalid value for -cluster_size, valid values are $valid_lib_size"
                return
            }
            set cluster_threshold ""
            if {$size == "single"} {
                set cluster_threshold 1.0
            } elseif {$size == "small"} {
                set cluster_threshold [expr 3.0 / 4.0]
            } elseif {$size == "medium"} {
                set cluster_threshold [expr 1.0 / 4.0]
            } elseif {$size == "large"} {
                set cluster_threshold [expr 1.0 / 12.0]
            } elseif {$size == "all"} {
                set cluster_threshold 0.0
            }

            return [psn::cluster_buffer_names $cluster_threshold true]
        }
    }

    define_cmd_args "repair_timing" {[-maximum_capacitance] [-maximum_transition] [-negative_slack]\
				 [-timerless] [-repair_by_resize] [-repair_by_clone]\
				 [-auto_buffer_library single|small|medium|large|all]\
				 [-minimize_buffer_library] [-fast]\
				 [-use_inverting_buffer_library] [-buffers buffers]\
				 [-inverters inverters ] [-iterations iterations] [-area_penalty area_penalty]\
				 [-legalization_frequency count] [-min_gain gain] [-enable_driver_resize] \
    }

    proc repair_timing { args } {
        sta::parse_key_args "repair_timing" args \
        keys {-auto_buffer_library -buffers -inverters -iterations -min_gain -area_penalty -legalization_frequency}\
        flags {-negative_slack -timerless -maximum_capacitance -maximum_transition -repair_by_resize -fast -repair_by_clone -enable_driver_resize -minimize_buffer_library -use_inverting_buffer_library -maximum_capacitance] -maximum_transition}
        
        set buffer_lib_flag ""
        set auto_buf_flag ""
        set mode_flag ""

        set has_max_cap [info exists flags(-maximum_capacitance)]
        set has_max_transition [info exists flags(-maximum_transition)]
        set has_max_ns [info exists flags(-negative_slack)]

        set repair_target_flag ""

        if {$has_max_cap} {
            set repair_target_flag "-maximum_capacitance"
        }
        if {$has_max_transition} {
            set repair_target_flag "$repair_target_flag -maximum_transition"
        }
        if {$has_max_ns} {
            set repair_target_flag "$repair_target_flag -negative_slack"
        }
        if {[info exists flags(-repair_by_resize)]} {
            set repair_target_flag "$repair_target_flag -repair_by_resize"
        }
        if {[info exists flags(-repair_by_clone)]} {
            set repair_target_flag "$repair_target_flag -repair_by_clone"
        }

        if {[info exists flags(-timerless)]} {
            set mode_flag "-timerless"
        }
        if {[info exists flags(-cirtical_path)]} {
            set mode_flag "$mode_flag -cirtical_path"
        }

        set fast_mode_flag ""
        if {[info exists flags(-fast)]} {
            set fast_mode_flag "-fast"
        }
        

        if {[info exists flags(-maximize_slack)]} {
            if {[info exists flags(-timerless)]} {
                 sta::sta_error "Cannot use -maximize_slack with -timerless mode"
                retrun
            }
            set mode_flag "$mode_flag -maximize_slack"
        }

        set has_auto_buff [info exists keys(-auto_buffer_library)]

        if {![psn::has_liberty]} {
            sta::sta_error "No liberty filed is loaded"
            return
        }

        if {$has_auto_buff} {
            set valid_lib_size [list "single" "small" "medium" "large" "all"]
            set buffer_lib_size $keys(-auto_buffer_library)
            if {[lsearch -exact $valid_lib_size $buffer_lib_size] < 0} {
                sta::sta_error "Invalid value for -auto_buffer_library, valid values are $valid_lib_size"
                return
            }
            set auto_buf_flag "-auto_buffer_library $buffer_lib_size"
        }

        if {[info exists keys(-buffers)]} {
            set blist $keys(-buffers)
            set buffer_lib_flag "-buffers $blist"
        }
        set inverters_flag ""
        if {[info exists keys(-inverters)]} {
            set ilist $keys(-inverters)
            set inverters_flag "-inverters $ilist"
        }
        set minimuze_buf_lib_flag ""
        if {[info exists flags(-minimize_buffer_library)]} {
            if {!$has_auto_buff} {
                sta::sta_error "-minimize_buffer_library can only be used with -auto_buffer_library"
                return
            }
            set minimuze_buf_lib_flag  "-minimize_buffer_library"
        }
        set use_inv_buf_lib_flag ""
        if {[info exists flags(-use_inverting_buffer_library)]} {
            if {!$has_auto_buff} {
                sta::sta_error "-use_inverting_buffer_library can only be used with -auto_buffer_library"
                return
            }
            set use_inv_buf_lib_flag  "-use_inverting_buffer_library"
        }
        set min_gain_flag ""
        if {[info exists keys(-min_gain)]} {
            set min_gain_flag  "-min_gain $keys(-min_gain)"
        }
        set legalization_freq_flag ""
        if {[info exists keys(-legalization_frequency)]} {
            set legalization_freq_flag  "-legalization_frequency $keys(-legalization_frequency)"
        }
        set area_penalty_flag ""
        if {[info exists keys(-area_penalty)]} {
            set area_penalty_flag  "-area_penalty $keys(-area_penalty)"
        }
        set resize_flag ""
        if {[info exists flags(-enable_driver_resize)]} {
            set resize_flag  "-enable_driver_resize"
        }
        set iterations 1
        if {[info exists keys(-iterations)]} {
            set iterations "$keys(-iterations)"
        }
        set bufargs "$repair_target_flag $fast_mode_flag $mode_flag $auto_buf_flag $minimuze_buf_lib_flag $use_inv_buf_lib_flag $legalization_freq_flag $buffer_lib_flag $inverters_flag $min_gain_flag $resize_flag $area_penalty_flag -iterations $iterations"
        set affected [transform timing_buffer {*}$bufargs]
        if {$affected < 0} {
            puts "Timing buffer failed"
            return
        }
        puts "Added/updated $affected cells"
    }
}
namespace import psn::*

)===<><>==="