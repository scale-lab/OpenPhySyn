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

#include <PhyKnight/Phy/ProgramOptions.hpp>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <sstream>
#include "PhyException/ProgramOptionsException.hpp"
namespace po = boost::program_options;

namespace phy
{
ProgramOptions::ProgramOptions(int argc, char** argv)
    : argc_(argc),
      argv_(argv),
      help_(false),
      version_(false),
      has_file_(false),
      verbose_(false),
      quiet_(false),
      has_log_level_(false),
      has_log_file_(false)
{

    po::command_line_style::style_t style = po::command_line_style::style_t(
        po::command_line_style::unix_style |
        po::command_line_style::case_insensitive |
        po::command_line_style::allow_long_disguise);

    po::options_description description("");
    description.add_options()("help,h", "Display this help message and exit")(
        "file", po::value<std::string>(), "Load TCL script")(
        "log-file", po::value<std::string>(), "Write output to log file")(
        "log-level", po::value<std::string>(),
        "Default log level [info, warn, error, critical]")(
        "version,v", "Display the version number")("verbose", "Verbose output")(
        "quiet", "Disable output");

    po::positional_options_description p;
    p.add("file", 1);

    if (argc)
    {
        std::ostringstream stream;
        stream << "Usage: " << argv_[0] << " [options] "
               << "[file]" << std::endl
               << description;
        usage_ = stream.str();
    }

    po::variables_map vm;
    try
    {

        po::store(po::command_line_parser(argc_, argv_)
                      .options(description)
                      .positional(p)
                      .style(style)
                      .run(),
                  vm);

        po::notify(vm);

        if (vm.count("help"))
        {
            help_ = true;
        }
        if (vm.count("version"))
        {
            version_ = true;
        }
        if (vm.count("verbose"))
        {
            verbose_ = true;
        }
        if (vm.count("quiet"))
        {
            quiet_ = true;
        }
        if (vm.count("file"))
        {
            has_file_ = true;
            file_     = vm["file"].as<std::string>();
        }
        if (vm.count("log-file"))
        {
            has_log_file_ = true;
            log_file_     = vm["log-file"].as<std::string>();
        }
        if (vm.count("log-level"))
        {
            has_log_level_ = true;
            log_level_     = vm["log-level"].as<std::string>();
        }
    }
    catch (po::error& e)
    {
        throw ProgramOptionsException(e.what());
    }
}
std::string
ProgramOptions::usage() const
{
    return usage_;
}

bool
ProgramOptions::help() const
{
    return help_;
}
bool
ProgramOptions::version() const
{
    return version_;
}
bool
ProgramOptions::verbose() const
{
    return version_;
}
bool
ProgramOptions::quiet() const
{
    return quiet_;
}
bool
ProgramOptions::hasFile() const
{
    return has_file_;
}
std::string
ProgramOptions::file() const
{
    return file_;
}
bool
ProgramOptions::hasLogFile() const
{
    return has_log_file_;
}
std::string
ProgramOptions::logFile() const
{
    return log_file_;
}
bool
ProgramOptions::hasLogLevel() const
{
    return has_log_level_;
}
std::string
ProgramOptions::logLevel() const
{
    return log_level_;
}

} // namespace phy
