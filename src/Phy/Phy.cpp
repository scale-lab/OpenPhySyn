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
#include <OpenSTA/network/ConcreteNetwork.hh>
#include <OpenSTA/search/Sta.hh>
#include <Phy/Phy.hpp>
#include <PhyKnight/PhyLogger/PhyLogger.hpp>
#include <boost/algorithm/string.hpp>
#include <tcl.h>
#include "DefReader/DefReader.hpp"
#include "DefWriter/DefWriter.hpp"
#include "LefReader/LefReader.hpp"
#include "LibertyReader/LibertyReader.hpp"
#include "PhyException/FileException.hpp"
#include "PhyException/NoTechException.hpp"
#include "PhyException/ParseLibertyException.hpp"
#include "PhyException/TransformNotFoundException.hpp"
#include "Transform/TransformHandler.hpp"
#include "Utils/FileUtils.hpp"

namespace phy
{

Phy::Phy()
    : db_(nullptr),
      liberty_(nullptr),
      sta_network_(new sta::ConcreteNetwork),
      interp_(nullptr)
{
    initializeDatabase();
    sta::initSta();
}

Phy::~Phy()
{
    delete sta_network_;
}

int
Phy::readDef(const char* path)
{
    DefReader reader(db_);
    try
    {
        return reader.read(path);
    }
    catch (FileException& e)
    {
        PhyLogger::instance().error(e.what());
        return -1;
    }
    catch (NoTechException& e)
    {
        PhyLogger::instance().error(e.what());
        return -1;
    }
}

int
Phy::readLib(const char* path)
{
    LibertyReader reader(db_, sta_network_);
    try
    {
        liberty_ = reader.read(path);
        sta::LibertyCellIterator cell_iter(liberty_);
        if (liberty_)
        {
            return 1;
        }
        return -1;
    }
    catch (PhyException& e)
    {
        PhyLogger::instance().error(e.what());
        return -1;
    }
}

int
Phy::readLef(const char* path)
{
    LefReader reader(db_);
    try
    {
        return reader.read(path);
    }
    catch (PhyException& e)
    {
        PhyLogger::instance().error(e.what());
        return -1;
    }
}

int
Phy::writeDef(const char* path)
{
    DefWriter writer(db_);
    try
    {
        return writer.write(path);
    }
    catch (PhyException& e)
    {
        PhyLogger::instance().error(e.what());
        return -1;
    }
}

Database*
Phy::database()
{
    return db_;
}
Liberty*
Phy::liberty()
{
    return liberty_;
}
ProgramOptions&
Phy::programOptions()
{
    return program_options_;
}

Phy&
Phy::instance()
{
    static Phy phySingelton;
    return phySingelton;
}

int
Phy::loadTransforms()
{

    std::vector<phy::TransformHandler> handlers;
    std::vector<std::string>           transforms_dirs;
    std::string                        transforms_paths(FileUtils::joinPath(
        FileUtils::homePath().c_str(), ".phyknight/transforms"));
    const char* env_path   = std::getenv("PHY_TRANSFORM_PATH");
    int         load_count = 0;

    if (env_path)
    {
        transforms_paths = std::string(env_path);
    }

    boost::split(transforms_dirs, transforms_paths, boost::is_any_of(":"));

    for (auto& transform_parent_path : transforms_dirs)
    {
        if (!FileUtils::isDirectory(transform_parent_path.c_str()))
        {
            phy::PhyLogger::instance().warn(
                "{} is not a directory, skipping transforms under this path.",
                transform_parent_path);
            continue;
        }
        std::vector<std::string> transforms_paths =
            FileUtils::readDir(transform_parent_path.c_str());
        for (auto& path : transforms_paths)
        {
            phy::PhyLogger::instance().debug("Reading transform {}", path);
            handlers.push_back(phy::TransformHandler(path));
        }

        PhyLogger::instance().debug("Found {} transforms under {}.",
                                    transforms_paths.size(),
                                    transform_parent_path);

        for (auto tr : handlers)
        {
            auto transform                  = tr.load();
            transforms_[tr.name()]          = transform;
            transforms_versions_[tr.name()] = tr.version();
            transforms_help_[tr.name()]     = tr.help();
            load_count++;
        }
    }

    phy::PhyLogger::instance().info("Loaded {} transforms.", load_count);

    return load_count;
}

int
Phy::runTransform(std::string transform_name, std::vector<std::string> args)
{
    try
    {
        if (!transforms_.count(transform_name))
        {
            throw TransformNotFoundException();
        }
        if (args.size() && args[0] == "version")
        {
            PhyLogger::instance().info("{}",
                                       transforms_versions_[transform_name]);
            return 0;
        }
        else if (args.size() && args[0] == "help")
        {
            PhyLogger::instance().info("{}", transforms_help_[transform_name]);
            return 0;
        }
        else
        {

            PhyLogger::instance().info("Invoking {} transform", transform_name);
            return transforms_[transform_name]->run(this, db_, args);
        }
    }
    catch (PhyException& e)
    {
        PhyLogger::instance().error(e.what());
        return -1;
    }
}

int
Phy::setupInterpreter(Tcl_Interp* interp)
{
    if (interp_)
    {
        PhyLogger::instance().warn("Multiple interpreter initialization!");
    }
    interp_ = interp;

    const char* tcl_setup =
#include "Tcl/Setup.tcl"
        ;
    return Tcl_Eval(interp_, tcl_setup);
}
void
Phy::printVersion(bool raw_str)
{
    if (raw_str)
    {

        PhyLogger::instance().raw("PhyKnight: {}.{}.{}", PROJECT_VERSION_MAJOR,
                                  PROJECT_VERSION_MINOR, PROJECT_VERSION_PATCH);
    }
    else
    {

        PhyLogger::instance().info("PhyKnight: {}.{}.{}", PROJECT_VERSION_MAJOR,
                                   PROJECT_VERSION_MINOR,
                                   PROJECT_VERSION_PATCH);
    }
}
void
Phy::printUsage(bool raw_str)
{
    if (raw_str)
    {
        PhyLogger::instance().raw(programOptions().usage());
    }
    else
    {
        PhyLogger::instance().info(programOptions().usage());
    }
}
void
Phy::processStartupProgramOptions()
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
        PhyLogger::instance().setLogFile(programOptions().logFile());
    }
    if (programOptions().hasFile())
    {
        sourceTclScript(programOptions().file().c_str());
    }
}
void
Phy::setProgramOptions(int argc, char* argv[])
{
    program_options_ = ProgramOptions(argc, argv);
}

int
Phy::setLogLevel(const char* level)
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
        PhyLogger::instance().error("Invalid log level {}", level);
        return false;
    }
    return true;
} // namespace phy
int
Phy::setLogPattern(const char* pattern)
{
    PhyLogger::instance().setPattern(pattern);

    return true;
}
int
Phy::setLogLevel(LogLevel level)
{
    PhyLogger::instance().setLevel(level);
    return true;
}
int
Phy::setupInterpreterReadline()
{
    if (interp_ == nullptr)
    {
        PhyLogger::instance().error("Tcl Interpreter is not initialized");
        return -1;
    }
    const char* rl_setup =
#include "Tcl/Readline.tcl"
        ;
    return Tcl_Eval(interp_, rl_setup);
}

int
Phy::sourceTclScript(const char* script_path)
{
    if (!FileUtils::pathExists(script_path))
    {
        PhyLogger::instance().error("Failed to open {}", script_path);
        return -1;
    }
    if (interp_ == nullptr)
    {
        PhyLogger::instance().error("Tcl Interpreter is not initialized");
        return -1;
    }
    std::string script_content;
    try
    {
        script_content = FileUtils::readFile(script_path);
    }
    catch (FileException& e)
    {
        PhyLogger::instance().error("Failed to open {}", script_path);
        PhyLogger::instance().error("{}", e.what());
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
Phy::initializeDatabase()
{
    db_ = odb::dbDatabase::create();

    return 0;
}

} // namespace phy