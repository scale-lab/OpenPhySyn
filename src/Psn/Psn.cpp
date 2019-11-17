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
#include <GraphDelayCalc.hh>
#include <OpenPhySyn/PsnLogger/PsnLogger.hpp>
#include <OpenPhySyn/Sta/DatabaseStaNetwork.hpp>
#include <OpenSTA/network/ConcreteNetwork.hh>
#include <OpenSTA/search/Search.hh>
#include <OpenSTA/search/Sta.hh>
#include <Psn/Psn.hpp>
#include <boost/algorithm/string.hpp>
#include <flute/flute.h>
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
namespace psn
{

Psn::Psn(Database* db)
    : db_(db),
      liberty_(nullptr),
      library_(nullptr),
      tech_(nullptr),
      interp_(nullptr)
{
    initializeDatabase();
    settings_   = new DesignSettings();
    sta_        = new sta::DatabaseSta(db_);
    db_handler_ = new DatabaseHandler(sta_);
    sta::initSta();
    sta::Sta::setSta(sta_);
    sta_->makeComponents();
    initalizeFlute("../external/flute/etc");
}

Psn::~Psn()
{
    delete settings_;
    delete db_handler_;
    delete sta_;
}

int
Psn::readDef(const char* path)
{
    DefReader reader(db_);
    try
    {
        int rc = reader.read(path);
        sta_->readDefAfter();
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
Psn::readLef(const char* path, bool read_library, bool read_tech)
{
    LefReader reader(db_);
    try
    {
        if (read_library && read_tech)
        {
            if (tech_ == nullptr)
            {
                library_ = reader.readLibAndTech(path);
                tech_    = library_->getTech();
            }
            else
            {
                // Might consider adding a warning here
                library_ = reader.readLib(path);
            }
            if (library_)
            {
                sta_->readLefAfter(library_);
            }
        }
        else if (read_library)
        {
            library_ = reader.readLib(path);
            if (library_)
            {
                sta_->readLefAfter(library_);
            }
        }
        else if (read_tech)
        {
            tech_ = reader.readTech(path);
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
Library*
Psn::library() const
{
    return library_;
}
LibraryTechnology*
Psn::tech() const
{
    return tech_;
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
    static Psn psnSingelton;
    return psnSingelton;
}

int
Psn::loadTransforms()
{

    std::vector<psn::TransformHandler> handlers;
    std::vector<std::string>           transforms_dirs;
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

    boost::split(transforms_dirs, transforms_paths, boost::is_any_of(":"));

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
            psn::PsnLogger::instance().debug("Reading transform {}", path);
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
    psn::PsnLogger::instance().info("Loaded {} transforms.", load_count);

    return load_count;
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
            return transforms_[transform_name]->run(this, args);
        }
    }
    catch (PsnException& e)
    {
        PsnLogger::instance().error(e.what());
        return -1;
    }
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
    sta_->setTclInterp(interp_);
    const char* tcl_psn_setup =
#include "Tcl/SetupPsn.tcl"
        ;
    if (Tcl_Eval(interp_, tcl_psn_setup) != TCL_OK)
    {
        return TCL_ERROR;
    }
    if (import_psn_namespace)
    {
        const char* tcl_psn_import =
#include "Tcl/ImportNS.tcl"
            ;
        if (Tcl_Eval(interp_, tcl_psn_import) != TCL_OK)
        {
            return TCL_ERROR;
        }
    }
    if (print_psn_version)
    {
        const char* tcl_print_version =
#include "Tcl/PrintVersion.tcl"
            ;
        if (Tcl_Eval(interp_, tcl_print_version) != TCL_OK)
        {
            return TCL_ERROR;
        }
    }
    if (setup_sta)
    {
        const char* tcl_setup_sta =
#include "Tcl/SetupSta.tcl"
            ;
        if (Tcl_Eval(interp_, tcl_setup_sta) != TCL_OK)
        {
            return TCL_ERROR;
        }
    }
    return TCL_OK;
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
        "print                                 print version\n"
        "help                                  print help\n"
        "print_usage                           print help\n"
        "print_transforms                      list loaded transforms\n"
        "read_lef <file path>                  load LEF file\n"
        "read_def <file path>                  load DEF file\n"
        "read_lib <file path>                  load a liberty file\n"
        "read_liberty <file path>              load a liberty file\n"
        "write_def <output file>               Write DEF file\n"
        "set_wire_rc <res> <cap>               Set resistance & capacitance "
        "per micron\n"
        "set_max_area <area>                   Set maximum design area\n"
        "transform <transform name> <args>     Run transform on the loaded "
        "design\n"
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
    if (interp_ == nullptr)
    {
        PsnLogger::instance().error("Tcl Interpreter is not initialized");
        return -1;
    }
    const char* rl_setup =
#include "Tcl/Readline.tcl"
        ;
    return Tcl_Eval(interp_, rl_setup);
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
    if (Tcl_Eval(interp_, script_content.c_str()) == TCL_ERROR)
    {
        return -1;
    }
    return 1;
}
void
Psn::setWireRC(float res_per_micon, float cap_per_micron)
{
    sta_->graphDelayCalc()->delaysInvalid();
    sta_->search()->arrivalsInvalid();

    settings()
        ->setResistancePerMicron(res_per_micon)
        ->setCapacitancePerMicron(cap_per_micron);
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

void
Psn::clearDatabase()
{
    handler()->clear();
    library_ = nullptr;
    tech_    = nullptr;
}

int
Psn::initalizeFlute(const char* flue_init_dir)
{
    std::string powv_file_path = std::string(flue_init_dir) + "/POWV9.dat";
    std::string post_file_path = std::string(flue_init_dir) + "/POST9.dat";
    if (!FileUtils::isDirectory(flue_init_dir) ||
        !FileUtils::pathExists(powv_file_path.c_str()) ||
        !FileUtils::pathExists(post_file_path.c_str()))
    {
        PsnLogger::instance().error("Flute initialization failed");
        return -1;
    }
    char* cwd = getcwd(NULL, 0);
    chdir(flue_init_dir);
    Flute::readLUT();
    chdir(cwd);
    free(cwd);
    return 1;
}
} // namespace psn