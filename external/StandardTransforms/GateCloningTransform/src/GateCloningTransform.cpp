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

#include "GateCloningTransform.hpp"
#include <OpenPhySyn/PsnLogger/PsnLogger.hpp>
#include <OpenPhySyn/SteinerTree/SteinerTree.hpp>

#include <algorithm>
#include <cmath>
#include <sstream>

using namespace psn;

int
GateCloningTransform::gateClone(Psn* psn_inst, float cap_factor,
                                bool clone_largest_only)
{
    PsnLogger&       logger  = PsnLogger::instance();
    DatabaseHandler& handler = *(psn_inst->handler());
    logger.info("Clone {} {}", cap_factor, clone_largest_only);
    std::vector<InstanceTerm*> level_drvrs = handler.levelDriverPins();
    for (auto& pin : level_drvrs)
    {
        Instance* inst = handler.instance(pin);
        cloneTree(psn_inst, inst, cap_factor, clone_largest_only);
        // if (handler.area() > psn_inst->settings()->maxArea())
        // {
        //     logger.warn("Max utilization reached!");
        //     break;
        // }
    }
    return 1;
}
void
GateCloningTransform::cloneTree(Psn* psn_inst, Instance* inst, float cap_factor,
                                bool clone_largest_only)
{
    PsnLogger&       logger  = PsnLogger::instance();
    DatabaseHandler& handler = *(psn_inst->handler());

    auto output_pins = handler.outputPins(inst);
    if (!output_pins.size())
    {
        return;
    }
    InstanceTerm* output_pin = *(output_pins.begin());
    Net*          net        = handler.net(output_pin);
    if (!net)
    {
        return;
    }
    auto tree = SteinerTree::create(net, psn_inst);
    if (tree == nullptr)
    {
        return;
    }
}
bool
GateCloningTransform::isNumber(const std::string& s)
{
    std::istringstream iss(s);
    float              f;
    iss >> std::noskipws >> f;
    return iss.eof() && !iss.fail();
}
int
GateCloningTransform::run(Psn* psn_inst, std::vector<std::string> args)
{
    if (args.size() > 2)
    {
        PsnLogger::instance().error(
            "Usage: transform gate_clone "
            "<float: max-cap-factor> <boolean: clone-gates-only>");
        return -1;
    }
    float cap_factor         = 1.4;
    bool  clone_largest_only = false;
    if (args.size() >= 1)
    {
        if (!isNumber(args[0]))
        {
            PsnLogger::instance().error(
                "Expected number for max-cap-factor, got {}", args[0]);
            return -1;
        }
        cap_factor = std::stof(args[0].c_str());
        if (args.size() >= 2)
        {
            std::transform(args[1].begin(), args[1].end(), args[1].begin(),
                           ::tolower);
            if (args[1] == "true" || args[1] == "1")
            {
                clone_largest_only = true;
            }
            else if (args[1] == "false" || args[1] == "0")
            {
                clone_largest_only = false;
            }
            else
            {
                PsnLogger::instance().error(
                    "Expected boolean for clone-gates-only, got {}", args[0]);
                return -1;
            }
        }
    }
    return gateClone(psn_inst, cap_factor, clone_largest_only);
}
