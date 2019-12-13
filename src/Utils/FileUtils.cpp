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

#ifndef __has_include
static_assert(false, "__has_include not supported");
#endif
#if __cplusplus > 201402L
#include <filesystem>
namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#elif __has_include(<boost/filesystem.hpp>)
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#else
#include <Utils/FilesystemLegacyHelpers.hpp>
namespace fs = psn::filesystem;
#endif

#include <fstream>
#include <libgen.h>
#include <sys/stat.h>
#include "FileUtils.hpp"
#include "PsnException/FileException.hpp"
namespace psn
{
bool
FileUtils::pathExists(const char* path)
{
    struct stat buf;
    return (stat(path, &buf) == 0);
}
bool
FileUtils::isDirectory(const char* path)
{
    struct stat buf;
    if (stat(path, &buf) == 0)
    {
        if (buf.st_mode & S_IFDIR)
        {
            return true;
        }
    }
    return false;
}
bool
FileUtils::createDirectory(const char* path)
{
    return fs::create_directory(path);
}
bool
FileUtils::createDirectoryIfNotExists(const char* path)
{
    if (pathExists(path))
    {
        if (isDirectory(path))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return createDirectory(path);
    }
}
std::vector<std::string>
FileUtils::readDirectory(const char* path)
{
    std::vector<std::string> paths;
    for (const auto& entry : fs::directory_iterator(path))
    {
        paths.push_back(entry.path());
    }
    return paths;
}
std::string
FileUtils::readFile(const char* path)
{
    std::ifstream infile{path};
    if (!infile.is_open())
    {
        throw FileException();
    }
    std::string file_contents{std::istreambuf_iterator<char>(infile),
                              std::istreambuf_iterator<char>()};
    return file_contents;
}
std::string
FileUtils::homePath()
{
    std::string home_path = std::getenv("HOME");
    if (!home_path.length())
    {
        home_path = std::getenv("USERPROFILE");
    }
    if (!home_path.length())
    {
        home_path = std::getenv("HOMEDRIVE");
    }
    if (!home_path.length())
    {
        home_path = "/home/OpenPhySyn";
    }
    return home_path;
}
std::string
FileUtils::joinPath(const char* first_path, const char* second_path)
{
    fs::path base(first_path);
    fs::path appendee(second_path);
    fs::path full_path = base / appendee;
    return full_path.generic_string();
}

std::string
FileUtils::baseName(const char* path)
{
    return std::string(basename(const_cast<char*>(path)));
}
} // namespace psn
