
R"===<><>===(

if {$tcl_interactive} {
    if { ![info exists unrenamed_source] } {
      proc unknown {args} {
          array set arg $args
          builtin_unknown {*}[array get arg]
      }
      if { [info proc source] != ""} {
        rename source ""
      }
      rename builtin_source source
      set unrenamed_source 1
    }
    package require tclreadline
    ::tclreadline::Loop
}

)===<><>==="