sta current_design ibex_core
sta create_clock -name core_clock -period 4.0000 -waveform {0.0000 2.0000} [sta get_ports {clk_i}]