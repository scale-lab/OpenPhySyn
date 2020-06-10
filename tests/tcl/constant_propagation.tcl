import_lib ../tests/data/libraries/Nangate45/NangateOpenCellLibrary_typical.lib
import_lef ../tests/data/libraries/Nangate45/NangateOpenCellLibrary.mod.lef
import_def ../tests/data/designs/constants/constants.def

propagate_constants

export_def ./outputs/propg.def
export_db ./outputs/prop.db

exit 0
