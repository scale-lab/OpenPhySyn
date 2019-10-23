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

#ifndef __PHY_PHY_LOGGER__
#define __PHY_PHY_LOGGER__

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
namespace phy
{
class PhyLogger
{
public:
    template<typename... Args>
    void
    trace(Args&&... args)
    {
        spdlog::trace(args...);
    }
    template<typename... Args>
    void
    debug(Args&&... args)
    {
        spdlog::debug(args...);
    }
    template<typename... Args>
    void
    info(Args&&... args)
    {
        spdlog::info(args...);
    }
    template<typename... Args>
    void
    warn(Args&&... args)
    {
        spdlog::warn(args...);
    }

    template<typename... Args>
    void
    critical(Args&&... args)
    {
        spdlog::critical(args...);
    }
    template<typename... Args>
    void
    error(Args&&... args)
    {
        spdlog::error(args...);
    }
    static PhyLogger& instance();

private:
    PhyLogger();
};
} // namespace phy
#endif //__PHY_DEMO__