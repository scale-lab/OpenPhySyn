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

#include <Phy/Phy.hpp>
#include <PhyKnight/PhyLogger/PhyLogger.hpp>
#include <boost/algorithm/string.hpp>
#include <config.hpp>
#include <tcl.h>
#include "DefReader/DefReader.hpp"
#include "DefWriter/DefWriter.hpp"
#include "LefReader/LefReader.hpp"
#include "PhyException/FileException.hpp"
#include "PhyException/NoTechException.hpp"
#include "PhyException/TransformNotFoundException.hpp"
#include "Transform/TransformHandler.hpp"
#include "Utils/FileUtils.hpp"

namespace phy
{

Phy::Phy() : interp_(nullptr)
{
    initializeDatabase();
    loadTransforms();
}

Phy::~Phy()
{
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
    return 0;
}

int
Phy::readLef(const char* path)
{
    LefReader reader(db_);
    try
    {
        return reader.read(path);
    }
    catch (FileException& e)
    {
        PhyLogger::instance().error(e.what());
        return -1;
    }
    catch (odb::ZException& e)
    {
        PhyLogger::instance().error(e._msg);
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
    catch (FileException& e)
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
    std::string                        transforms_paths("/usr/share/phyknight");
    const char* env_path   = std::getenv("PHY_TRANSFORM_PATH");
    int         load_count = 0;

    if (env_path)
    {
        transforms_paths = std::string(env_path);
    }

    boost::split(transforms_dirs, transforms_paths, boost::is_any_of(":"));

    for (auto& transform_parent_path : transforms_dirs)
    {
        if (!phy::FileUtils::isDirectory(transform_parent_path.c_str()))
        {
            phy::PhyLogger::instance().warn(
                "{} is not a directory, skipping transforms under this path.",
                transform_parent_path);
            continue;
        }
        std::vector<std::string> transforms_paths =
            phy::FileUtils::readDir(transform_parent_path.c_str());
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
    catch (TransformNotFoundException& e)
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
// Private methods:

int
Phy::initializeDatabase()
{
    db_ = odb::dbDatabase::create();

    return 0;
}
} // namespace phy