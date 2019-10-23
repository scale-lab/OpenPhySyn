
R"===<><>===(
namespace eval phy {
    namespace export *
}

namespace import phy::*

proc transform {transform_name args} {
    transform_internal $transform_name $args
}
print_version



# Do all the logic  before this line..

if {$tcl_interactive} {
    package require tclreadline
    ::tclreadline::Loop
}

)===<><>==="