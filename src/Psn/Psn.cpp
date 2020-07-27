// BSD 3-Clause License

// Copyright (c) 2019, SCALE Lab, Brown University
// All rights reserved.

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:

// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.

// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.

// * Neither the name of the copyright holder nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include <Psn/Psn.hpp>
#include <flute.h>
#include <tcl.h>
#include "Config.hpp"
#include "Def/DefReader.hpp"
#include "Def/DefWriter.hpp"
#include "Lef/LefReader.hpp"
#include "Liberty/LibertyReader.hpp"
#include "OpenPhySyn/PsnLogger/PsnLogger.hpp"
#include "OpenPhySyn/Sta/DatabaseSta.hpp"
#include "OpenPhySyn/Sta/DatabaseStaNetwork.hpp"
#include "OpenPhySyn/Utils/PsnGlobal.hpp"
#include "PsnException/FileException.hpp"
#include "PsnException/FluteInitException.hpp"
#include "PsnException/NoTechException.hpp"
#include "PsnException/ParseLibertyException.hpp"
#include "PsnException/TransformNotFoundException.hpp"
#include "Utils/FileUtils.hpp"
#include "Utils/StringUtils.hpp"
#include "sta/ArcDelayCalc.hh"
#include "sta/ConcreteNetwork.hh"
#include "sta/Search.hh"
#include "sta/Sta.hh"
#include "sta/Units.hh"

#ifdef OPENPHYSYN_OPENDP_ENABLED
#include "opendp/MakeOpendp.h"
#include "opendp/Opendp.h"
#endif

extern "C"
{
    extern int Psn_Init(Tcl_Interp* interp);
    extern int Sta_Init(Tcl_Interp* interp);
}
namespace sta
{
extern void        evalTclInit(Tcl_Interp* interp, const char* inits[]);
extern const char* tcl_inits[];
} // namespace sta

namespace psn
{
using sta::evalTclInit;
using sta::tcl_inits;

Psn* Psn::psn_instance_;
bool Psn::is_initialized_ = false;

Psn::Psn(Database* db) : db_(db), interp_(nullptr)
{
    if (db_ == nullptr)
    {
        initializeDatabase();
    }
    exec_path_ = FileUtils::executablePath();
    initializeSta();
    db_handler_ = new DatabaseHandler(this, sta_);
}
void
Psn::initialize(Database* db, bool load_transforms, Tcl_Interp* interp,
                bool init_flute)
{
    psn_instance_ = new Psn(db);
    if (load_transforms)
    {
        psn_instance_->loadTransforms();
    }
    if (interp != nullptr)
    {
        psn_instance_->setupInterpreter(interp);
    }
    if (init_flute)
    {
        psn_instance_->initializeFlute();
    }
    setupLegalizer();
    is_initialized_ = true;
}

Psn::Psn(sta::DatabaseSta* sta) : sta_(sta), db_(nullptr), interp_(nullptr)
{
    if (sta == nullptr)
    {
        initializeDatabase();
        initializeSta();
    }
    exec_path_  = FileUtils::executablePath();
    db_         = sta_->db();
    db_handler_ = new DatabaseHandler(this, sta_);
}

void
Psn::initialize(sta::DatabaseSta* sta, bool load_transforms, Tcl_Interp* interp,
                bool init_flute, bool import_psn_namespace,
                bool print_psn_version, bool setup_sta_tcl)
{
    psn_instance_ = new Psn(sta);
    if (load_transforms)
    {
        psn_instance_->loadTransforms();
    }
    if (interp != nullptr)
    {
        psn_instance_->setupInterpreter(interp, import_psn_namespace,
                                        print_psn_version, setup_sta_tcl);
    }
    if (init_flute)
    {
        psn_instance_->initializeFlute();
    }

    setupLegalizer();
    is_initialized_ = true;
}

Psn::~Psn()
{
    delete db_handler_;
    delete sta_;
    if (db_ != nullptr)
    {
        Database::destroy(db_);
    }
}

int
Psn::readDef(const char* path)
{
    DefReader reader(db_);
    try
    {
        int rc = reader.read(path);
        sta_->postReadDef(db_->getChip()->getBlock());
        return rc;
    }
    catch (FileException& e)
    {
        PSN_LOG_ERROR(e.what());
        return -1;
    }
    catch (NoTechException& e)
    {
        PSN_LOG_ERROR(e.what());
        return -1;
    }
}

int
Psn::readLib(const char* path)
{
    LibertyReader reader(sta_);
    try
    {
        liberty_ = reader.read(path);
        sta_->getDbNetwork()->readLibertyAfter(liberty_);
        if (liberty_)
        {
            return 1;
        }
        return -1;
    }
    catch (PsnException& e)
    {
        PSN_LOG_ERROR(e.what());
        return -1;
    }
}

int
Psn::readLef(const char* path, bool import_library, bool import_tech)
{
    LefReader reader(db_);
    try
    {
        Library*           library = nullptr;
        LibraryTechnology* tech    = nullptr;
        if (import_library && import_tech)
        {
            if (tech == nullptr)
            {
                library = reader.readLibAndTech(path);
                tech    = library->getTech();
            }
            else
            {
                // Might consider adding a warning here
                library = reader.readLib(path);
            }
            if (library)
            {
                sta_->postReadLef(tech, library);
            }
        }
        else if (import_library)
        {
            library = reader.readLib(path);
            if (library)
            {
                sta_->postReadLef(tech, library);
            }
        }
        else if (import_tech)
        {
            tech = reader.readTech(path);
        }
        else
        {
            return 0;
        }

        return 1;
    }
    catch (PsnException& e)
    {
        PSN_LOG_ERROR(e.what());
        return -1;
    }
}

int
Psn::writeDef(const char* path)
{
    DefWriter writer(db_);
    try
    {
        return writer.write(path);
    }
    catch (PsnException& e)
    {
        PSN_LOG_ERROR(e.what());
        return -1;
    }
}

int
Psn::readDatabase(const char* path)
{
    FILE* stream = fopen(path, "r");
    if (stream)
    {
        db_->read(stream);
        sta_->postReadDb(db_);
        fclose(stream);
        return 1;
    }
    return 0;
}
int
Psn::writeDatabase(const char* path)
{
    FILE* stream = fopen(path, "w");
    if (stream)
    {
        db_->write(stream);
        fclose(stream);
        return 1;
    }
    return 0;
}

Database*
Psn::database() const
{
    return db_;
}
Liberty*
Psn::liberty() const
{
    return liberty_;
}

LibraryTechnology*
Psn::tech() const
{
    return db_->getTech();
}

ProgramOptions&
Psn::programOptions()
{
    return program_options_;
}

DatabaseHandler*
Psn::handler() const
{
    return db_handler_;
}

bool
Psn::hasDesign() const
{
    return (database() && database()->getChip() != nullptr);
}
bool
Psn::hasLiberty() const
{
    return handler()->hasLiberty();
}

Psn&
Psn::instance()
{
    if (!is_initialized_)
    {
        PSN_LOG_CRITICAL("OpenPhySyn is not initialized!");
    }
    return *psn_instance_;
}
Psn*
Psn::instancePtr()
{
    if (!is_initialized_)
    {
        PSN_LOG_CRITICAL("OpenPhySyn is not initialized!");
    }
    return psn_instance_;
}

int
Psn::loadTransforms()
{
    int load_count = 0;

    OPENPHYSYN_LOAD_TRANSFORMS(transforms_, transforms_info_, load_count);

#ifdef OPENPHYSYN_ENABLE_DYNAMIC_TRANSFORM_LIBRARY
    std::string transforms_paths(
        FileUtils::joinPath(FileUtils::homePath(), ".OpenPhySyn/transforms") +
        ":" + FileUtils::joinPath(exec_path_, "transforms") + ":" +
        std::string(PSN_TRANSFORM_INSTALL_FULL_PATH));
    const char* env_path = std::getenv("PSN_TRANSFORM_PATH");

    if (env_path)
    {
        transforms_paths =
            transforms_paths + std::string(":") + std::string(env_path);
    }

    std::vector<std::string> transforms_dirs =
        StringUtils::split(transforms_paths, ":");

    for (auto& transform_parent_path : transforms_dirs)
    {
        if (!transform_parent_path.length())
        {
            continue;
        }
        if (!FileUtils::isDirectory(transform_parent_path))
        {
            continue;
        }
        PSN_LOG_DEBUG("Searching for transforms at {}", transform_parent_path);
        std::vector<std::string> transforms_paths =
            FileUtils::readDirectory(transform_parent_path, true);
        for (auto& path : transforms_paths)
        {
            PSN_LOG_DEBUG("Loading transform {}", path);
            handlers_.push_back(psn::TransformHandler(path));
        }

        PSN_LOG_DEBUG("Found {} transforms under {}.", transforms_paths.size(),
                      transform_parent_path);
    }

    for (auto tr : handlers_)
    {
        std::string tr_name(tr.name());
        if (!transforms_.count(tr_name))
        {
            auto transform            = tr.load();
            transforms_[tr_name]      = transform;
            transforms_info_[tr_name] = TransformInfo(
                tr_name, tr.help(), tr.version(), tr.description());
            load_count++;
        }
        else
        {
            PSN_LOG_DEBUG(
                "Transform {} was already loaded, discarding subsequent loads",
                tr_name);
        }
    }
#endif
    PSN_LOG_INFO("Loaded {} transforms.", load_count);
    return load_count;
}

bool
Psn::hasTransform(std::string transform_name)
{
    return transforms_.count(transform_name);
}

int
Psn::runTransform(std::string transform_name, std::vector<std::string> args)
{
    if (!hasDesign())
    {
        PSN_LOG_ERROR("Could not find any loaded design.");
        return -1;
    }
    try
    {
        if (!transforms_.count(transform_name))
        {
            throw TransformNotFoundException();
        }
        if (args.size() && args[0] == "version")
        {
            PSN_LOG_INFO("{}", transforms_info_[transform_name].version());
            return 0;
        }
        else if (args.size() && args[0] == "help")
        {
            PSN_LOG_INFO("{}", transforms_info_[transform_name].help());
            return 0;
        }
        else
        {

            PSN_LOG_INFO("Invoking {} transform", transform_name);
            int rc = transforms_[transform_name]->run(this, args);
            PSN_LOG_INFO("Finished {} transform ({})", transform_name, rc);
            return rc;
        }
    }
    catch (PsnException& e)
    {
        PSN_LOG_ERROR(e.what());
        return -1;
    }
}

void
Psn::setupLegalizer()
{
#ifdef OPENPHYSYN_OPENDP_ENABLED
    ord::makeOpendp();
    psn_instance_->setLegalizer([=](int max_displacment) -> bool {
        opendp::Opendp* opendp = opendp::Opendp::instance;
        opendp->detailedPlacement(max_displacment);
        return true;
    });
#endif
}

Tcl_Interp*
Psn::interpreter() const
{
    return interp_;
}
int
Psn::setupInterpreter(Tcl_Interp* interp, bool import_psn_namespace,
                      bool print_psn_version, bool setup_sta)
{
    if (interp_)
    {
        PSN_LOG_WARN("Multiple interpreter initialization!");
    }
    interp_ = interp;
    if (Psn_Init(interp) == TCL_ERROR)
    {
        return TCL_ERROR;
    }
    if (setup_sta)
    {
        sta_->setTclInterp(interp_);
        if (Sta_Init(interp) == TCL_ERROR)
        {
            return TCL_ERROR;
        }
        evalTclInit(interp, tcl_inits);
        const char* tcl_psn_setup =
#include "Tcl/SetupPsnSta.tcl"
            ;
        if (evaluateTclCommands(tcl_psn_setup) != TCL_OK)
        {
            return TCL_ERROR;
        }
    }
    else
    {

        const char* tcl_psn_setup =
#include "Tcl/SetupPsn.tcl"
            ;
        if (evaluateTclCommands(tcl_psn_setup) != TCL_OK)
        {
            return TCL_ERROR;
        }
    }

    const char* tcl_define_cmds =
#include "Tcl/DefinePSNCommands.tcl"
        ;
    if (evaluateTclCommands(tcl_define_cmds) != TCL_OK)
    {
        return TCL_ERROR;
    }

    if (import_psn_namespace)
    {
        const char* tcl_psn_import =
#include "Tcl/ImportNS.tcl"
            ;
        if (evaluateTclCommands(tcl_psn_import) != TCL_OK)
        {
            return TCL_ERROR;
        }
    }
    if (print_psn_version)
    {
        const char* tcl_print_version =
#include "Tcl/PrintVersion.tcl"
            ;
        if (evaluateTclCommands(tcl_print_version) != TCL_OK)
        {
            return TCL_ERROR;
        }
    }

#ifdef OPENPHYSYN_OPENDP_ENABLED
    ord::initOpendp(psn_instance_->interpreter(), psn_instance_->database());
#endif

    return TCL_OK;
}
int
Psn::evaluateTclCommands(const char* commands) const
{
    if (!interp_)
    {
        PSN_LOG_ERROR("Tcl Interpreter is not initialized");
        return TCL_ERROR;
    }
    return Tcl_Eval(interp_, commands);
}
void
Psn::printVersion(bool raw_str)
{
    if (raw_str)
    {

        PSN_LOG_RAW("OpenPhySyn: {}", PSN_VERSION_STRING);
    }
    else
    {

        PSN_LOG_INFO("OpenPhySyn: {}", PSN_VERSION_STRING);
    }
}
void
Psn::printUsage(bool raw_str, bool print_transforms, bool print_commands)
{
    PSN_LOG_RAW("");
    if (raw_str)
    {
        PSN_LOG_RAW(programOptions().usage());
    }
    else
    {
        PSN_LOG_INFO(programOptions().usage());
    }
    if (print_commands)
    {
        printCommands(true);
    }
    PSN_LOG_RAW("");
    if (print_transforms)
    {
        printTransforms(true);
    }
}
void
Psn::printLicense(bool raw_str)
{
    std::string license = R"===<><>===(BSD 3-Clause License
Copyright (c) 2019, SCALE Lab, Brown University
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE."
)===<><>===";
    license = std::string("OpenPhySyn ") + PSN_VERSION_STRING + "\n" + license;
    PSN_LOG_RAW("");
    if (raw_str)
    {
        PSN_LOG_RAW(license);
    }
    else
    {
        PSN_LOG_INFO(license);
    }
}
void
Psn::printCommands(bool raw_str)
{
    if (raw_str)
    {
        PSN_LOG_RAW("Available command: ");
    }
    else
    {

        PSN_LOG_INFO("Available commands: ");
    }
    PSN_LOG_RAW("");
    std::string commands_str;
    commands_str +=
        "design_area			Report design total cell area\n"
        "export_db			Export OpenDB database file\n"
        "export_def			Export design DEF file\n"
        "gate_clone			Perform load-driven gate cloning\n"
        "get_database			Return OpenDB database object\n"
        "get_database_handler		Return OpenPhySyn database "
        "handler\n"
        "get_handler			Alias for "
        "get_database_handler\n"
        "get_liberty			Return first loaded liberty "
        "file\n"
        "has_transform			Check if the specified transform is "
        "loaded\n"
        "help				Print this help\n"
        "import_db			Import OpenDB database file\n"
        "import_def			Import design DEF file\n"
        "import_lef			Import technology LEF file\n"
        "import_lib			Alias for import_liberty\n"
        "import_liberty			Import liberty file\n"
        "link				Alias for link_design\n"
        "link_design			Link design top module\n"
        "make_steiner_tree		Create steiner tree around "
        "net\n"
        "optimize_design			Perform timing optimization\n"
        "optimize_fanout			Perform maximum-fanout based "
        "buffering\n"
        "optimize_logic			Perform logic optimization\n"
        "optimize_power			Perform power optimization\n"
        "pin_swap			Perform timing optimization by "
        "commutative pin swapping\n"
        "print_liberty_cells		Print liberty cells available "
        "in the  loaded library\n"
        "print_license			Print license information\n"
        "print_transforms		Print loaded transforms\n"
        "print_usage			Print usage instructions\n"
        "print_version			Print tool version\n"
        "propagate_constants		Perform logic optimization by constant "
        "propgation\n"
        "timing_buffer			Repair violations through buffer tree "
        "insertion\n"
        "repair_timing			Repair design timing and electrical "
        "violations "
        "through resizing, buffer insertion, and pin-swapping\n"
        "capacitance_violations		Print pins with capacitance limit "
        "violation\n"
        "transition_violations		Print pins with transition limit "
        "violation\n"
        "set_log				Alias for "
        "set_log_level\n"
        "set_log_level			Set log level [trace, debug, info, "
        "warn, error, critical, off]\n"
        "set_log_pattern			Set log printing pattern, "
        "refer to spdlog logger for pattern formats\n"
        "set_max_area			Set maximum design area\n"
        "set_wire_rc			Set wire "
        "resistance/capacitance per micron, you can also specify technology "
        "layer\n"
        "transform			Run loaded transform\n"
        "version				Alias for "
        "print_version\n";
    PSN_LOG_RAW("{}", commands_str);
}
void
Psn::printTransforms(bool raw_str)
{
    if (raw_str)
    {
        PSN_LOG_RAW("Loaded transforms: ");
    }
    else
    {

        PSN_LOG_INFO("Loaded transforms: ");
    }
    PSN_LOG_RAW("");
    std::string transform_str;
    for (auto it = transforms_.begin(); it != transforms_.end(); ++it)
    {
        transform_str = it->first;
        transform_str += " (";
        transform_str += transforms_info_[it->first].version();
        transform_str += "): ";
        transform_str += transforms_info_[it->first].description();
        if (raw_str)
        {
            PSN_LOG_RAW(transform_str);
        }
        else
        {
            PSN_LOG_INFO(transform_str);
        }
    }
    PSN_LOG_RAW("");

} // namespace psn
void
Psn::processStartupProgramOptions()
{

    if (programOptions().hasLogLevel())
    {
        setLogLevel(programOptions().logLevel().c_str());
    }
    if (programOptions().verbose())
    {
        setLogLevel(LogLevel::debug);
    }
    if (programOptions().quiet())
    {
        setLogLevel(LogLevel::off);
    }
    if (programOptions().hasLogFile())
    {
        PsnLogger::instance().setLogFile(programOptions().logFile());
    }
    if (programOptions().hasFile())
    {
        sourceTclScript(programOptions().file().c_str());
    }
}
void
Psn::setProgramOptions(int argc, char* argv[])
{
    program_options_ = ProgramOptions(argc, argv);
}

int
Psn::setLogLevel(const char* level)
{
    std::string level_str(level);
    std::transform(level_str.begin(), level_str.end(), level_str.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    if (level_str == "trace")
    {
        return setLogLevel(LogLevel::trace);
    }
    else if (level_str == "debug")
    {
        return setLogLevel(LogLevel::debug);
    }
    else if (level_str == "info")
    {
        return setLogLevel(LogLevel::info);
    }
    else if (level_str == "warn")
    {
        return setLogLevel(LogLevel::warn);
    }
    else if (level_str == "error")
    {
        return setLogLevel(LogLevel::error);
    }
    else if (level_str == "critical")
    {
        return setLogLevel(LogLevel::critical);
    }
    else if (level_str == "off")
    {
        return setLogLevel(LogLevel::off);
    }
    else
    {
        PSN_LOG_ERROR("Invalid log level {}", level);
        return false;
    }
    return true;
} // namespace psn
int
Psn::setLogPattern(const char* pattern)
{
    PsnLogger::instance().setPattern(pattern);

    return true;
}
int
Psn::setLogLevel(LogLevel level)
{
    PsnLogger::instance().setLevel(level);
    return true;
}
int
Psn::setupInterpreterReadline()
{
    const char* rl_setup =
#include "Tcl/Readline.tcl"
        ;
    return evaluateTclCommands(rl_setup);
}

int
Psn::sourceTclScript(const char* script_path)
{
    if (!FileUtils::pathExists(script_path))
    {
        PSN_LOG_ERROR("Failed to open {}", script_path);
        return -1;
    }
    if (interp_ == nullptr)
    {
        PSN_LOG_ERROR("Tcl Interpreter is not initialized");
        return -1;
    }
    std::string script_content;
    try
    {
        script_content = FileUtils::readFile(script_path);
    }
    catch (FileException& e)
    {
        PSN_LOG_ERROR("Failed to open {}", script_path);
        PSN_LOG_ERROR("{}", e.what());
        return -1;
    }
    if (evaluateTclCommands(script_content.c_str()) == TCL_ERROR)
    {
        return -1;
    }
    return 1;
}
void
Psn::setLegalizer(Legalizer legalizer)
{
    db_handler_->setLegalizer(legalizer);
}
void
Psn::setWireRC(float res_per_micron, float cap_per_micron)
{
    if (!database() || database()->getChip() == nullptr)
    {
        PSN_LOG_ERROR("Could not find any loaded design.");
        return;
    }
    res_per_micron =
        (sta_->units()->resistanceUnit()->scale() * res_per_micron) /
        (sta_->units()->distanceUnit()->scale() * 1.0);
    cap_per_micron =
        (sta_->units()->capacitanceUnit()->scale() * cap_per_micron) /
        (sta_->units()->distanceUnit()->scale() * 1.0);
    handler()->setWireRC(res_per_micron, cap_per_micron);
}
int
Psn::setWireRC(const char* layer_name)
{
    auto tech = db_->getTech();

    if (!tech)
    {
        PSN_LOG_ERROR("Could not find any loaded technology file.");
        return -1;
    }

    auto layer = tech->findLayer(layer_name);
    if (!layer)
    {
        PSN_LOG_ERROR("Could not find layer with the name {}.", layer_name);
        return -1;
    }
    auto  width_dbu      = layer->getWidth();
    auto  width          = handler()->dbuToMicrons(width_dbu);
    float res_per_micron = (layer->getResistance() / width) * 1E6;
    float cap_per_micron =
        (width * layer->getCapacitance() + 2 * layer->getEdgeCapacitance()) *
        1E-6;
    handler()->setWireRC(res_per_micron, cap_per_micron);
    return 1;
}
int
Psn::linkDesign(const char* design_name)
{
    int rc = sta_->linkDesign(design_name);
    sta_->postReadDb(db_);
    return rc;
}

// Private methods:
int
Psn::initializeDatabase()
{
    if (db_ == nullptr)
    {
        db_ = odb::dbDatabase::create();
    }
    return 0;
}
int
Psn::initializeSta(Tcl_Interp* interp)
{
    if (interp == nullptr)
    {
        interp = Tcl_CreateInterp();
        Tcl_Init(interp);
    }
    sta_ = new sta::DatabaseSta;
    sta_->init(interp, db_);

    return 0;
}

void
Psn::clearDatabase()
{
    handler()->clear();
}

int
Psn::initializeFlute()
{
    Flute::readLUT();
    return 1;
}
} // namespace psn