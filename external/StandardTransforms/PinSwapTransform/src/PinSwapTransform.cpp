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

#include "PinSwapTransform.hpp"
#include <OpenPhySyn/PsnLogger/PsnLogger.hpp>
#include <OpenPhySyn/Utils/PsnGlobal.hpp>
#include <OpenSTA/dcalc/ArcDelayCalc.hh>
#include <OpenSTA/dcalc/GraphDelayCalc.hh>
#include <OpenSTA/liberty/TimingArc.hh>
#include <OpenSTA/liberty/TimingModel.hh>
#include <OpenSTA/liberty/TimingRole.hh>
#include <OpenSTA/search/Corner.hh>

#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>

using namespace psn;

PinSwapTransform::PinSwapTransform() : swap_count_(0)
{
}
void
PinSwapTransform::swapPins(psn::Psn* psn_inst, psn::InstanceTerm* first,
                           psn::InstanceTerm* second)
{
    // PsnLogger&       logger     = PsnLogger::instance();
    DatabaseHandler& handler    = *(psn_inst->handler());
    auto             first_net  = handler.net(first);
    auto             second_net = handler.net(second);
    handler.disconnect(first);
    handler.disconnect(second);
    handler.connect(first_net, second);
    handler.connect(second_net, first);
    handler.resetDelays();
}

int
PinSwapTransform::pinSwap(psn::Psn* psn_inst)
{
    PsnLogger&       logger  = PsnLogger::instance();
    DatabaseHandler& handler = *(psn_inst->handler());
    auto             cp      = handler.criticalPath();
    auto             bp      = handler.bestPath();
    // std::reverse(cp.begin(), cp.end());
    // std::reverse(bp.begin(), bp.end());
    int br = 2;
    for (auto& point : cp)
    {

        auto pin      = std::get<0>(point);
        auto is_rise  = std::get<1>(point);
        is_rise       = true;
        auto inst     = handler.instance(pin);
        auto lib_cell = handler.libraryCell(inst);
        if (!handler.isInput(pin))
        {
            continue;
        }
        auto input_pins  = handler.inputPins(inst);
        auto output_pins = handler.outputPins(inst);
        if (input_pins.size() < 2 || output_pins.size() != 1)
        {
            continue;
        }
        auto out_pin = output_pins[0];
        logger.info("{} ({})", handler.name(lib_cell), handler.name(pin));
        InstanceTerm* swap_target = nullptr;
        // float         best_avg_delay =
        //     (handler.pinAverageRise(handler.libraryPin(pin),
        //                             handler.libraryPin(out_pin)) +
        //      handler.pinAverageFall(handler.libraryPin(pin),
        //                             handler.libraryPin(out_pin))) /
        //     2;
        float best_avg_delay =
            is_rise ? handler.pinAverageRise(handler.libraryPin(pin),
                                             handler.libraryPin(out_pin))
                    : handler.pinAverageFall(handler.libraryPin(pin),
                                             handler.libraryPin(out_pin));
        logger.info("Current Avg. {}", best_avg_delay);
        for (auto& in_pin : input_pins)
        {
            if (in_pin != pin && handler.isCommutative(in_pin, pin))
            {
                float pin_delay =
                    is_rise
                        ? handler.pinAverageRise(handler.libraryPin(in_pin),
                                                 handler.libraryPin(out_pin))
                        : handler.pinAverageFall(handler.libraryPin(in_pin),
                                                 handler.libraryPin(out_pin));
                // float pin_delay =
                //     (handler.pinAverageRise(handler.libraryPin(in_pin),
                //                             handler.libraryPin(out_pin)) +
                //      handler.pinAverageFall(handler.libraryPin(in_pin),
                //                             handler.libraryPin(out_pin))) /
                //     2;
                logger.info("New Avg. {}", pin_delay);
                if (pin_delay < best_avg_delay)
                {
                    best_avg_delay = pin_delay;
                    swap_target    = in_pin;
                }
            }
        }
        if (swap_target)
        {
            logger.info("Confirmed Swap..{} {}", handler.name(pin),
                        handler.name(swap_target));
            swapPins(psn_inst, pin, swap_target);
            if (!br)
            {
                break;
            }
            br--;
        }
    }
    return 0;
}

bool
PinSwapTransform::isNumber(const std::string& s)
{
    std::istringstream iss(s);
    float              f;
    iss >> std::noskipws >> f;
    return iss.eof() && !iss.fail();
}
int
PinSwapTransform::run(Psn* psn_inst, std::vector<std::string> args)
{
    if (args.size() > 1)
    {
        PsnLogger::instance().error(help());
        return -1;
    }
    else if (args.size() && !isNumber(args[0]))
    {
        PsnLogger::instance().error(help());
        return -1;
    }

    return pinSwap(psn_inst);
}
