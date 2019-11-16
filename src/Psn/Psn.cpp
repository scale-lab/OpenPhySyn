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
#include <OpenSTA/network/ConcreteNetwork.hh>
#include <OpenSTA/search/Sta.hh>
#include <Psn/Psn.hpp>
#include <boost/algorithm/string.hpp>
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
      interp_(nullptr),
      library_(nullptr),
      tech_(nullptr)
{
    initializeDatabase();
    sta_        = new sta::DatabaseSta(db_);
    db_handler_ = new DatabaseHandler(sta_);
    sta::initSta();
    sta::Sta::setSta(sta_);
    sta_->makeComponents();
}

Psn::~Psn()
{
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
Psn::database()
{
    return db_;
}
Liberty*
Psn::liberty()
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
            auto transform                = tr.load();
            transforms_[tr_name]          = transform;
            transforms_versions_[tr_name] = tr.version();
            transforms_help_[tr_name]     = tr.help();
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
            PsnLogger::instance().info("{}",
                                       transforms_versions_[transform_name]);
            return 0;
        }
        else if (args.size() && args[0] == "help")
        {
            PsnLogger::instance().info("{}", transforms_help_[transform_name]);
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
                      bool setup_sta)
{
    if (interp_)
    {
        PsnLogger::instance().warn("Multiple interpreter initialization!");
    }
    interp_ = interp;
    sta_->setTclInterp(interp_);
    const char* tcl_psn_setup =
#include "Tcl/SetupPhy.tcl"
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
Psn::printUsage(bool raw_str)
{
    if (raw_str)
    {
        PsnLogger::instance().raw(programOptions().usage());
    }
    else
    {
        PsnLogger::instance().info(programOptions().usage());
    }
}
void
Psn::printTransforms(bool raw_str)
{
    std::string transform_str;
    for (auto it = transforms_.begin(); it != transforms_.end(); ++it)
    {
        transform_str = it->first;
        transform_str += ": ";
        transform_str += transforms_versions_[it->first];
        if (raw_str)
        {
            PsnLogger::instance().raw(transform_str);
        }
        else
        {
            PsnLogger::instance().info(transform_str);
        }
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

} // namespace psn