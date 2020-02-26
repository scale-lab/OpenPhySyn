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

#include "BufferResizeTransform.hpp"
#include <OpenPhySyn/PsnLogger/PsnLogger.hpp>
#include <OpenPhySyn/Sta/PathPoint.hpp>
#include <OpenPhySyn/Utils/PsnGlobal.hpp>
#include <OpenSTA/dcalc/ArcDelayCalc.hh>
#include <OpenSTA/dcalc/GraphDelayCalc.hh>
#include <OpenSTA/liberty/TimingArc.hh>
#include <OpenSTA/liberty/TimingModel.hh>
#include <OpenSTA/liberty/TimingRole.hh>
#include <OpenSTA/search/Corner.hh>
#include "Utils/StringUtils.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>

// Objectives:
// 1- Standard Van Ginneken buffering (with pruning) [TODO].
// 2- Multiple buffer sizes [TODO].
// 3- Inverter pair instead of buffer pairs [TODO].
// 4- Simultaneous buffering and gate sizing [TODO].

using namespace psn;

BufferResizeTransform::BufferResizeTransform() : buffer_count_(0)
{
}

void
BufferResizeTransform::fixCapacitanceViolations(
    Psn* psn_inst, std::unordered_set<psn::LibraryCell*>& buffer_lib,
    std::unordered_set<psn::LibraryCell*>& inverter_lib, bool resize_gates,
    bool use_inverter_pair)
{
}

void
BufferResizeTransform::fixSlewViolations(
    Psn* psn_inst, std::unordered_set<psn::LibraryCell*>& buffer_lib,
    std::unordered_set<psn::LibraryCell*>& inverter_lib, bool resize_gates,
    bool use_inverter_pair)
{
}

int
BufferResizeTransform::fixViolations(
    Psn* psn_inst, bool fix_cap, bool fix_slew,
    std::unordered_set<std::string> buffer_lib_names,
    std::unordered_set<std::string> inverter_lib_names, bool resize_gates,
    bool use_inverter_pair)
{
    std::unordered_set<psn::LibraryCell*> buffer_lib;
    std::unordered_set<psn::LibraryCell*> inverter_lib;
    DatabaseHandler&                      handler = *(psn_inst->handler());

    if (!buffer_lib_names.size())
    {
        for (auto& lib_cell : handler.bufferCells())
        {
            buffer_lib.insert(lib_cell);
        }
    }
    else
    {
        for (auto& buf_name : buffer_lib_names)
        {
            auto lib_cell = handler.libraryCell(buf_name.c_str());
            if (!lib_cell)
            {
                PSN_LOG_ERROR("Buffer cell {} not found in the library.",
                              buf_name);
                return -1;
            }
            buffer_lib.insert(lib_cell);
        }
    }
    if (use_inverter_pair)
    {
        if (!inverter_lib_names.size())
        {
            for (auto& lib_cell : handler.inverterCells())
            {
                inverter_lib.insert(lib_cell);
            }
        }
        else
        {
            for (auto& inv_name : inverter_lib_names)
            {
                auto lib_cell = handler.libraryCell(inv_name.c_str());
                if (!lib_cell)
                {
                    PSN_LOG_ERROR("Inverter cell {} not found in the library.",
                                  inv_name);
                    return -1;
                }
                inverter_lib.insert(lib_cell);
            }
        }
    }

    if (fix_cap)
    {
        fixCapacitanceViolations(psn_inst, buffer_lib, inverter_lib,
                                 resize_gates, use_inverter_pair);
    }
    if (fix_slew)
    {
        fixSlewViolations(psn_inst, buffer_lib, inverter_lib, resize_gates,
                          use_inverter_pair);
    }
    return buffer_count_;
}

int
BufferResizeTransform::run(Psn* psn_inst, std::vector<std::string> args)
{
    buffer_count_                                     = 0;
    bool                            resize_gates      = false;
    bool                            use_inverter_pair = false;
    bool                            use_all_buffers   = false;
    bool                            use_all_inverters = false;
    std::unordered_set<std::string> buffer_lib_names;
    std::unordered_set<std::string> inverter_lib_names;
    std::unordered_set<std::string> keywords(
        {"-buffers", "--buffers", "-inverters", "--inverters",
         "-enable_gate_resize", "--enable_gate_resize", "-enable_inverter_pair",
         "--enable_inverter_pair"});
    if (args.size() < 2)
    {
        PSN_LOG_ERROR(help());
        return -1;
    }
    for (size_t i = 0; i < args.size(); i++)
    {
        if (args[i] == "-buffers" || args[i] == "--buffers")
        {
            i++;
            while (i < args.size())
            {
                if (args[i] == "-all" || args[i] == "--all")
                {
                    if (buffer_lib_names.size())
                    {
                        PSN_LOG_ERROR(help());
                        return -1;
                    }
                    else
                    {
                        use_all_buffers = true;
                    }
                }
                else if (args[i] == "-buffers" || args[i] == "--buffers")
                {
                    PSN_LOG_ERROR(help());
                    return -1;
                }
                else if (keywords.count(args[i]))
                {
                    break;
                }
                else if (args[i][0] == '-')
                {
                    PSN_LOG_ERROR(help());
                    return -1;
                }
                else
                {
                    buffer_lib_names.insert(args[i]);
                }
                i++;
            }
            i--;
        }
        else if (args[i] == "-inverters" || args[i] == "--inverters")
        {
            i++;
            while (i < args.size())
            {
                if (args[i] == "-all" || args[i] == "--all")
                {
                    if (inverter_lib_names.size())
                    {
                        PSN_LOG_ERROR(help());
                        return -1;
                    }
                    else
                    {
                        use_all_inverters = true;
                    }
                }
                else if (args[i] == "-inverters" || args[i] == "--inverters")
                {
                    PSN_LOG_ERROR(help());
                    return -1;
                }
                else if (keywords.count(args[i]))
                {
                    break;
                }
                else if (args[i][0] == '-')
                {
                    PSN_LOG_ERROR(help());
                    return -1;
                }
                else
                {
                    inverter_lib_names.insert(args[i]);
                }
                i++;
            }
            i--;
        }
        else if (args[i] == "-enable_gate_resize" ||
                 args[i] == "--enable_gate_resize")
        {
            resize_gates = true;
        }
        else if (args[i] == "-enable_inverter_pair" ||
                 args[i] == "--enable_inverter_pair")
        {
            use_inverter_pair = true;
        }
        else
        {
            PSN_LOG_ERROR(help());
            return -1;
        }
    }
    if ((!buffer_lib_names.size() && !use_all_buffers) ||
        (buffer_lib_names.size() && use_all_buffers))
    {
        PSN_LOG_ERROR(help());
        return -1;
    }
    if (use_inverter_pair)
    {
        if ((!inverter_lib_names.size() && !use_all_inverters) ||
            (inverter_lib_names.size() && use_all_inverters))
        {
            PSN_LOG_ERROR(help());
            return -1;
        }
    }
    return fixViolations(psn_inst, true, true, buffer_lib_names,
                         inverter_lib_names, resize_gates, use_inverter_pair);
}
