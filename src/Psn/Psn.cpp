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

// Temproary fix for OpenSTA
#define THROW_DCL throw()

#include <Config.hpp>
#include <OpenPhySyn/PsnLogger/PsnLogger.hpp>
#include <OpenPhySyn/Sta/DatabaseStaNetwork.hpp>
#include <OpenSTA/dcalc/ArcDelayCalc.hh>
#include <OpenSTA/network/ConcreteNetwork.hh>
#include <OpenSTA/search/Search.hh>
#include <OpenSTA/search/Sta.hh>
#include <Psn/Psn.hpp>
#include <flute.h>
#include <tcl.h>
#include "DefReader/DefReader.hpp"
#include "DefWriter/DefWriter.hpp"
#include "LefReader/LefReader.hpp"
#include "LibertyReader/LibertyReader.hpp"
#include "PsnException/FileException.hpp"
#include "PsnException/NoTechException.hpp"
#include "PsnException/ParseLibertyException.hpp"
#include "PsnException/TransformNotFoundException.hpp"
#include "Transform/TransformHandler.hpp"
#include "Utils/FileUtils.hpp"
#include "Utils/StringUtils.hpp"

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

#ifdef OPENROAD_BUILD
namespace sta
{
extern const char* dbsta_tcl_inits[];
}
#endif
namespace psn
{
using sta::evalTclInit;
using sta::tcl_inits;

Psn* Psn::psn_instance_;
bool Psn::is_initialized_ = false;

#ifndef OPENROAD_BUILD
Psn::Psn(Database* db) : db_(db), interp_(nullptr)
{
    if (db_ == nullptr)
    {
        initializeDatabase();
    }
    settings_ = new DesignSettings();
    initializeSta();
    db_handler_ = new DatabaseHandler(sta_);
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
    is_initialized_ = true;
}
#endif

Psn::Psn(sta::DatabaseSta* sta) : sta_(sta), db_(nullptr), interp_(nullptr)
{
    if (sta == nullptr)
    {
        initializeDatabase();
        initializeSta();
    }
    db_         = sta_->db();
    settings_   = new DesignSettings();
    db_handler_ = new DatabaseHandler(sta_);
}

void
Psn::initialize(sta::DatabaseSta* sta, bool load_transforms, Tcl_Interp* interp,
                bool init_flute)
{
    psn_instance_ = new Psn(sta);
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
    is_initialized_ = true;
}

Psn::~Psn()
{
    delete settings_;
    delete db_handler_;
#ifndef OPENROAD_BUILD
    delete sta_;
    if (db_ != nullptr)
    {
        Database::destroy(db_);
    }
#endif
}

int
Psn::readDef(const char* path)
{
    DefReader reader(db_);
    try
    {
        int rc = reader.read(path);
        sta_->readDefAfter();
        // sta_->findDelays();
        return rc;
    }
    catch (FileException& e)
    {
        PsnLogger::instance().error(e.what());
        return -1;
    }
    catch (NoTechException& e)
    {
        PsnLogger::instance().error(e.what());
        return -1;
    }
}

int
Psn::readLib(const char* path)
{
    LibertyReader reader(db_, sta_);
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
        PsnLogger::instance().error(e.what());
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
                sta_->readLefAfter(library);
            }
        }
        else if (import_library)
        {
            library = reader.readLib(path);
            if (library)
            {
                sta_->readLefAfter(library);
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
        PsnLogger::instance().error(e.what());
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
        PsnLogger::instance().error(e.what());
        return -1;
    }
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

DesignSettings*
Psn::settings() const
{
    return settings_;
}

Psn&
Psn::instance()
{
    if (!is_initialized_)
    {
        PsnLogger::instance().critical("OpenPhySyn is not initialized!");
    }
    return *psn_instance_;
}
Psn*
Psn::instancePtr()
{
#ifndef OPENROAD_BUILD
    if (!is_initialized_)
    {
        PsnLogger::instance().critical("OpenPhySyn is not initialized!");
    }
#endif
    return psn_instance_;
}

int
Psn::loadTransforms()
{

    std::vector<psn::TransformHandler> handlers;
    std::string                        transforms_paths(
        FileUtils::joinPath(FileUtils::homePath().c_str(),
                            ".OpenPhySyn/transforms") +
        ":./transforms");
    const char* env_path   = std::getenv("PSN_TRANSFORM_PATH");
    int         load_count = 0;

    if (env_path)
    {
        transforms_paths = transforms_paths + ":" + std::string(env_path);
    }

    std::vector<std::string> transforms_dirs =
        StringUtils::split(transforms_paths, ":");

    for (auto& transform_parent_path : transforms_dirs)
    {
        if (!transform_parent_path.length())
        {
            continue;
        }
        if (!FileUtils::isDirectory(transform_parent_path.c_str()))
        {
            continue;
        }
        std::vector<std::string> transforms_paths =
            FileUtils::readDirectory(transform_parent_path.c_str());
        for (auto& path : transforms_paths)
        {
            PsnLogger::instance().debug("Reading transform {}", path);
            handlers.push_back(psn::TransformHandler(path));
        }

        PsnLogger::instance().debug("Found {} transforms under {}.",
                                    transforms_paths.size(),
                                    transform_parent_path);
    }

    for (auto tr : handlers)
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
            PsnLogger::instance().debug(
                "Transform {} was already loaded, discarding subsequent loads",
                tr_name);
        }
    }
#ifndef OPENROAD_BUILD
    psn::PsnLogger::instance().info("Loaded {} transforms.", load_count);
#endif
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
    try
    {
        if (!transforms_.count(transform_name))
        {
            throw TransformNotFoundException();
        }
        if (args.size() && args[0] == "version")
        {
            PsnLogger::instance().info(
                "{}", transforms_info_[transform_name].version());
            return 0;
        }
        else if (args.size() && args[0] == "help")
        {
            PsnLogger::instance().info("{}",
                                       transforms_info_[transform_name].help());
            return 0;
        }
        else
        {

            PsnLogger::instance().info("Invoking {} transform", transform_name);
            int rc = transforms_[transform_name]->run(this, args);
            sta_->ensureLevelized();
            handler()->resetDelays();
            PsnLogger::instance().info("Finished {} transform ({})",
                                       transform_name, rc);
            return rc;
        }
    }
    catch (PsnException& e)
    {
        PsnLogger::instance().error(e.what());
        return -1;
    }
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
        PsnLogger::instance().warn("Multiple interpreter initialization!");
    }
    interp_ = interp;

    if (Psn_Init(interp) == TCL_ERROR)
    {
        return TCL_ERROR;
    }

#ifndef OPENROAD_BUILD
    sta_->setTclInterp(interp_);
    if (setup_sta)
    {
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
#endif // OPENROAD_BUILD

    const char* tcl_define_cmds =
#include "Tcl/DefinePSNCommands.tcl"
        ;
    if (evaluateTclCommands(tcl_define_cmds) != TCL_OK)
    {
        return TCL_ERROR;
    }

#ifndef OPENROAD_BUILD
    if (!setup_sta)
    {
        const char* tcl_psn_setup =
#include "Tcl/SetupPsn.tcl"
            ;
        if (evaluateTclCommands(tcl_psn_setup) != TCL_OK)
        {
            return TCL_ERROR;
        }
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
#endif // OPENROAD_BUILD

    return TCL_OK;
}
int
Psn::evaluateTclCommands(const char* commands) const
{
    if (!interp_)
    {
        PsnLogger::instance().error("Tcl Interpreter is not initialized");
        return TCL_ERROR;
    }
    return Tcl_Eval(interp_, commands);
}
void
Psn::printVersion(bool raw_str)
{
    if (raw_str)
    {

        PsnLogger::instance().raw("OpenPhySyn: {}.{}.{}", PROJECT_VERSION_MAJOR,
                                  PROJECT_VERSION_MINOR, PROJECT_VERSION_PATCH);
    }
    else
    {

        PsnLogger::instance().info("OpenPhySyn: {}.{}.{}",
                                   PROJECT_VERSION_MAJOR, PROJECT_VERSION_MINOR,
                                   PROJECT_VERSION_PATCH);
    }
}
void
Psn::printUsage(bool raw_str, bool print_transforms, bool print_commands)
{
    PsnLogger::instance().raw("");
    if (raw_str)
    {
        PsnLogger::instance().raw(programOptions().usage());
    }
    else
    {
        PsnLogger::instance().info(programOptions().usage());
    }
    if (print_commands)
    {
        printCommands(true);
    }
    PsnLogger::instance().raw("");
    if (print_transforms)
    {
        printTransforms(true);
    }
}
void
Psn::printCommands(bool raw_str)
{
    if (raw_str)
    {
        PsnLogger::instance().raw("Available command: ");
    }
    else
    {

        PsnLogger::instance().info("Available commands: ");
    }
    PsnLogger::instance().raw("");
    std::string commands_str;
    commands_str +=
        "print_version                         print version\n"
        "version                               print version\n"
        "help                                  print help\n"
        "print_usage                           print help\n"
        "print_transforms                      list loaded transforms\n"
        "import_lef <file path>                load LEF file\n"
        "import_def <file path>                load DEF file\n"
        "import_lib <file path>                load a liberty file\n"
        "import_liberty <file path>            load a liberty file\n"
        "export_def <output file>              Write DEF file\n"
        "set_wire_rc <res> <cap>               Set resistance & capacitance "
        "per micron\n"
        "set_max_area <area>                   Set maximum design area\n"
        "optimize_design [<options>]           Perform timing optimization on "
        "the design\n"
        "optimize_fanout <options>             Buffer high-fanout nets\n"
        "transform <transform name> <args>     Run transform on the loaded "
        "design\n"
        "link <design name>                    Link design top module\n"
        "link_design <design name>             Link design top module\n"
        "sta <OpenSTA commands>                Run OpenSTA commands\n"
        "make_steiner_tree <net>               Construct steiner tree for the "
        "provided net\n"
        "set_log <log level>                   Set log level [trace, debug, "
        "info, "
        "warn, error, critical, off]\n"
        "set_log_level <log level>             Set log level [trace, debug, "
        "info, warn, error, critical, off]\n"
        "set_log_pattern <pattern>             Set log printing pattern, refer "
        "to spdlog logger for pattern formats";
    PsnLogger::instance().raw("{}", commands_str);
}
void
Psn::printTransforms(bool raw_str)
{
    if (raw_str)
    {
        PsnLogger::instance().raw("Loaded transforms: ");
    }
    else
    {

        PsnLogger::instance().info("Loaded transforms: ");
    }
    PsnLogger::instance().raw("");
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
            PsnLogger::instance().raw(transform_str);
        }
        else
        {
            PsnLogger::instance().info(transform_str);
        }
        PsnLogger::instance().raw("");
    }

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
        PsnLogger::instance().error("Invalid log level {}", level);
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
        PsnLogger::instance().error("Failed to open {}", script_path);
        return -1;
    }
    if (interp_ == nullptr)
    {
        PsnLogger::instance().error("Tcl Interpreter is not initialized");
        return -1;
    }
    std::string script_content;
    try
    {
        script_content = FileUtils::readFile(script_path);
    }
    catch (FileException& e)
    {
        PsnLogger::instance().error("Failed to open {}", script_path);
        PsnLogger::instance().error("{}", e.what());
        return -1;
    }
    if (evaluateTclCommands(script_content.c_str()) == TCL_ERROR)
    {
        return -1;
    }
    return 1;
}
void
Psn::setWireRC(float res_per_micon, float cap_per_micron)
{
    handler()->resetDelays();
    settings()
        ->setResistancePerMicron(res_per_micon)
        ->setCapacitancePerMicron(cap_per_micron);
}
int
Psn::linkDesign(const char* design_name)
{
    int rc = sta_->linkDesign(design_name);
    sta_->readDbAfter();
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
#ifndef OPENROAD_BUILD
    sta::initSta();
    sta_ = new sta::DatabaseSta(db_);
    sta::Sta::setSta(sta_);
    sta_->makeComponents();
#else
    if (interp == nullptr)
    {
        // This is a very bad solution! but temporarily until
        // dbSta can take a database without interp..
        interp = Tcl_CreateInterp();
        Tcl_Init(interp);
        // evalTclInit(interp, sta::dbsta_tcl_inits);
    }
    sta_ = new sta::DatabaseSta;
    sta_->init(interp, db_);
#endif
    return 0;
}

void
Psn::clearDatabase()
{
    handler()->clear();
}

int
Psn::initializeFlute(const char* flue_init_dir)
{
    bool        lut_found = false;
    std::string flute_dir, powv_file_path, post_file_path;
    if (flue_init_dir)
    {
        flute_dir      = std::string(flute_dir);
        powv_file_path = FileUtils::joinPath(flute_dir.c_str(), "POWV9.dat");
        post_file_path = FileUtils::joinPath(flute_dir.c_str(), "POST9.dat");
        if (FileUtils::isDirectory(flute_dir.c_str()) &&
            FileUtils::pathExists(powv_file_path.c_str()) &&
            FileUtils::pathExists(post_file_path.c_str()))
        {
            lut_found = true;
        }
    }
    else
    {
        for (auto& s :
             std::vector<std::string>({"../external/flute/etc", "../etc", "."}))
        {
            flute_dir = s;
            powv_file_path =
                FileUtils::joinPath(flute_dir.c_str(), "POWV9.dat");
            post_file_path =
                FileUtils::joinPath(flute_dir.c_str(), "POST9.dat");
            if (FileUtils::isDirectory(flute_dir.c_str()) &&
                FileUtils::pathExists(powv_file_path.c_str()) &&
                FileUtils::pathExists(post_file_path.c_str()))
            {
                lut_found = true;
                break;
            }
        }
    }
    if (!lut_found)
    {
        PsnLogger::instance().error("Flute initialization failed");
        return -1;
    }

    char* cwd = getcwd(NULL, 0);
    chdir(flute_dir.c_str());
    Flute::readLUT();
    chdir(cwd);
    free(cwd);
    return 1;
}
} // namespace psn