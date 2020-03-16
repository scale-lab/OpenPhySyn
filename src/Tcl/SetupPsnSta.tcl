
R"===<><>===(
namespace eval psn {
    namespace eval sta {
        sta::define_sta_cmds
        namespace import ::sta::*
        namespace export *
        namespace ensemble create
    }
    rename help psn_help
    rename set_max_area psn_set_max_area
    rename link_design psn_link_design
    namespace import ::sta::*
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
    rename help ""
    rename psn_help help
    rename set_max_area ""
    rename psn_set_max_area set_max_area
    rename link_design ""
    rename psn_link_design link_design
    namespace export *
    namespace ensemble create
}

)===<><>==="