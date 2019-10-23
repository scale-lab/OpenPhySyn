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

#include "Exports.hpp"
#include <Phy/Phy.hpp>
#include <config.hpp>
#include <iostream>

namespace phy
{

int
read_def(const char* def_path)
{
    return Phy::instance().readDef(def_path);
}
int
read_lef(const char* def_path)
{
    return Phy::instance().readLef(def_path);
}
int
read_lib(const char* def_path)
{
    return Phy::instance().readLib(def_path);
}
int
write_def(const char* def_path)
{
    return Phy::instance().writeDef(def_path);
}

void
version()
{
    print_version();
}

int
transform_internal(std::string transform_name, std::vector<std::string> args)
{
    return Phy::instance().runTransform(transform_name, args);
}

void
print_version()
{
    PhyLogger::instance().info("PhyKnight: {}.{}.{}", PROJECT_VERSION_MAJOR,
                               PROJECT_VERSION_MINOR, PROJECT_VERSION_PATCH);
}

} // namespace phy
