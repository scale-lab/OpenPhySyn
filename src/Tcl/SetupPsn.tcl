
R"===<><>===(
namespace eval psn {
    namespace export *
    namespace ensemble create
}

proc transform {transform_name args} {
    psn::transform_internal $transform_name $args
}


)===<><>==="