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

#include "ConstantPropagationTransform.hpp"
#include <OpenPhySyn/PsnLogger/PsnLogger.hpp>
#include <OpenPhySyn/Sta/PathPoint.hpp>
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

ConstantPropagationTransform::ConstantPropagationTransform() : prop_count_(0)
{
}

// 1: Tied to the input.
// 0: Not tied.
// -1: TODO tied to the negated input.
int
ConstantPropagationTransform::isTiedToInput(psn::Psn*          psn_inst,
                                            psn::InstanceTerm* input_term,
                                            psn::InstanceTerm* constant_term,
                                            bool               constant_val)

{
    DatabaseHandler& handler               = *(psn_inst->handler());
    LibraryTerm*     constant_library_term = handler.libraryPin(constant_term);
    LibraryTerm*     input_library_term    = handler.libraryPin(input_term);
    psn::Instance*   inst                  = handler.instance(constant_term);

    InstanceTerm* out_pin = handler.outputPins(inst)[0];

    for (int i = 0; i < 2; ++i)
    {
        std::unordered_map<LibraryTerm*, int> sim_vals;
        sim_vals[constant_library_term] = constant_val;
        sim_vals[input_library_term]    = i;
        // TODO: check if tied to the inverse of input_term
        if (handler.evaluateFunctionExpression(out_pin, sim_vals) != i)
        {
            return false;
        }
    }
    return true;
}

// 1: Tied to logic 1
// 0: Tied to logic 0
// -1: Not tied constant value
int
ConstantPropagationTransform::isTiedToConstant(psn::Psn*          psn_inst,
                                               psn::InstanceTerm* constant_term,
                                               bool               constant_val)
{
    DatabaseHandler&           handler  = *(psn_inst->handler());
    psn::Instance*             inst     = handler.instance(constant_term);
    std::vector<InstanceTerm*> in_pins  = handler.inputPins(inst);
    InstanceTerm*              out_pin  = handler.outputPins(inst)[0];
    int                        last_val = -1;
    LibraryTerm* constant_library_term  = handler.libraryPin(constant_term);
    int          in_pin_size            = in_pins.size();
    int          premuts                = std::pow(2, in_pin_size - 1);
    for (int i = 0; i < premuts; ++i)
    {
        std::unordered_map<LibraryTerm*, int> sim_vals;
        sim_vals[constant_library_term] = constant_val;
        int temp                        = i;
        for (InstanceTerm* in_pin : in_pins)
        {
            if (in_pin == constant_term)
            {
                continue;
            }
            LibraryTerm* library_pin = handler.libraryPin(in_pin);
            sim_vals[library_pin]    = temp & 1;
            temp >>= 1;
        }
        int result = handler.evaluateFunctionExpression(out_pin, sim_vals);
        if (last_val == -1)
        {
            last_val = result;
        }
        else if (result != last_val)
        {
            return -1;
        }
    }
    return last_val;
}
void
ConstantPropagationTransform::propagateTieHiLoCell(
    psn::Psn* psn_inst, bool is_tiehi, psn::InstanceTerm* constant_term,
    int max_depth, psn::Instance* tiehi_cell, psn::Instance* tielo_cell,
    std::unordered_set<Instance*>&      visited,
    std::unordered_set<psn::Instance*>& deleted)
{
    DatabaseHandler& handler       = *(psn_inst->handler());
    Instance*        inst          = handler.instance(constant_term);
    auto             inst_out_pins = handler.outputPins(inst);
    if (max_depth == 0 || inst_out_pins.size() != 1 || visited.count(inst))
    {
        return;
    }

    visited.insert(inst);

    auto inst_out_pin = inst_out_pins[0];
    auto fanout_pins  = handler.fanoutPins(handler.net(inst_out_pin), true);
    PSN_LOG_DEBUG("propagateTieHiLoCell({} / {}: {}) [{}]", handler.name(inst),
                  handler.name(handler.libraryCell(inst)),
                  handler.name(inst_out_pin), is_tiehi);

    for (auto& pin : fanout_pins)
    {
        auto fanout_inst            = handler.instance(pin);
        auto fanout_inst_input_pins = handler.inputPins(fanout_inst);
        if (!handler.isSingleOutputCombinational(fanout_inst))
        {
            continue;
        }
        auto fanout_inst_output_pin = handler.outputPins(fanout_inst)[0];
        auto is_const               = isTiedToConstant(psn_inst, pin, is_tiehi);
        if (is_const >= 0)
        {
            if (tiehi_cell && is_const == 1)
            {
                auto tiehi_net = handler.net(handler.outputPins(tiehi_cell)[0]);
                PSN_LOG_DEBUG("{} is tied to 1.", handler.name(fanout_inst));
                // for cell in fanout cells
                // propagateTieHiLoCell(psn_inst, true, fanout_inst, max_depth?
                // max_depth-1: 0, tiehi_cell, tielo_cell);
                // Remove fanout_inst and Connect all fanouts to tiehi;
                propagateTieHiLoCell(psn_inst, true, fanout_inst_output_pin,
                                     max_depth == -1 ? max_depth
                                                     : max_depth - 1,
                                     tiehi_cell, tielo_cell, visited, deleted);
                if (deleted.count(fanout_inst))
                {
                    continue;
                }
                auto fanout_sink_pins = handler.fanoutPins(
                    handler.net(fanout_inst_output_pin), false);
                for (auto& sink_pin : fanout_sink_pins)
                {
                    handler.disconnect(sink_pin);
                    handler.connect(tiehi_net, sink_pin);
                    PSN_LOG_DEBUG("Connected {} to tiehi",
                                  handler.name(sink_pin));
                }
                assert(
                    handler
                        .fanoutPins(handler.net(fanout_inst_output_pin), false)
                        .size() == 0);
                handler.del(fanout_inst);
                deleted.insert(fanout_inst);
                prop_count_++;
            }
            else if (tielo_cell && is_const == 0)
            {
                auto tielo_net = handler.net(handler.outputPins(tielo_cell)[0]);
                PSN_LOG_DEBUG("{} is tied to 0.", handler.name(fanout_inst));
                // for cell in fanout cells
                // propagateTieHiLoCell(psn_inst, false, fanout_inst, max_depth?
                // max_depth-1: 0, tiehi_cell, tielo_cell); Remove fanout_inst
                // and Connect all fanouts to tiehi; Remove fanout_inst and
                // Connect all fanouts to tielo;

                propagateTieHiLoCell(psn_inst, false, fanout_inst_output_pin,
                                     max_depth == -1 ? max_depth
                                                     : max_depth - 1,
                                     tiehi_cell, tielo_cell, visited, deleted);
                if (deleted.count(fanout_inst))
                {
                    continue;
                }
                auto fanout_sink_pins = handler.fanoutPins(
                    handler.net(fanout_inst_output_pin), false);
                PSN_LOG_DEBUG("Removing {} (constant 0)",
                              handler.name(fanout_inst));
                for (auto& sink_pin : fanout_sink_pins)
                {
                    PSN_LOG_DEBUG("Connected {} to tielo",
                                  handler.name(sink_pin));
                    handler.disconnect(sink_pin);
                    handler.connect(tielo_net, sink_pin);
                }
                assert(
                    handler
                        .fanoutPins(handler.net(fanout_inst_output_pin), false)
                        .size() == 0);
                handler.del(fanout_inst);
                deleted.insert(fanout_inst);
                prop_count_++;
            }
        }
        else if (fanout_inst_input_pins.size() == 2)
        {
            InstanceTerm* other_pin = nullptr;
            for (auto& p : fanout_inst_input_pins)
            {
                if (p != pin)
                {
                    other_pin = p;
                    break;
                }
            }
            if (other_pin && isTiedToInput(psn_inst, other_pin, pin, is_tiehi))
            {
                auto fanout_sink_pins = handler.fanoutPins(
                    handler.net(fanout_inst_output_pin), false);
                PSN_LOG_DEBUG("{} is tied to input {}. Constant pin: {}",
                              handler.name(fanout_inst),
                              handler.name(other_pin), handler.name(pin));
                auto other_pin_net    = handler.net(other_pin);
                auto other_pin_driver = handler.faninPin(other_pin_net);
                for (auto& sink_pin : fanout_sink_pins)
                {
                    PSN_LOG_DEBUG("Connect {} to driver of {} [{} <- {}]",
                                  handler.name(sink_pin),
                                  handler.name(other_pin),
                                  handler.name(other_pin_net),
                                  handler.name(other_pin_driver));
                    handler.disconnect(sink_pin);
                    handler.connect(other_pin_net, sink_pin);
                }
                assert(
                    handler
                        .fanoutPins(handler.net(fanout_inst_output_pin), false)
                        .size() == 0);
                handler.del(fanout_inst);
                deleted.insert(fanout_inst);
            }
        }
    }
}

int
ConstantPropagationTransform::propagateConstants(psn::Psn*   psn_inst,
                                                 std::string tiehi_cell_name,
                                                 std::string tielo_cell_name,
                                                 int         max_depth)
{
    DatabaseHandler& handler = *(psn_inst->handler());

    std::unordered_set<LibraryCell*> tiehi_cells;
    std::unordered_set<LibraryCell*> tielo_cells;

    if (tiehi_cell_name.length())
    {
        LibraryCell* cell = handler.libraryCell(tiehi_cell_name.c_str());
        if (!cell)
        {
            PSN_LOG_ERROR("TieHi {} not found!", tiehi_cell_name);
            return -1;
        }
        tiehi_cells.insert(cell);
    }
    else
    {
        auto lib_cells = handler.tiehiCells();
        tiehi_cells.insert(lib_cells.begin(), lib_cells.end());
    }

    if (tielo_cell_name.length())
    {
        LibraryCell* cell = handler.libraryCell(tielo_cell_name.c_str());
        if (!cell)
        {
            PSN_LOG_ERROR("TieLo {} not found!", tielo_cell_name);
            return -1;
        }
        tielo_cells.insert(cell);
    }
    else
    {
        auto lib_cells = handler.tieloCells();
        tielo_cells.insert(lib_cells.begin(), lib_cells.end());
    }
    Instance* first_tihi = nullptr;
    Instance* first_tilo = nullptr;
    for (auto instance : handler.instances())
    {
        auto instance_lib_cell = handler.libraryCell(instance);
        if (tiehi_cells.count(instance_lib_cell))
        {
            first_tihi = instance;
            if (first_tilo)
            {
                break;
            }
        }
        else if (tielo_cells.count(instance_lib_cell))
        {
            first_tilo = instance;
            if (first_tihi)
            {
                break;
            }
        }
    }
    const int iteration_count = 1;
    for (int i = 0; i < iteration_count; ++i)
    {
        PSN_LOG_DEBUG("Constant Propagation Iteration {}/{}", i + 1,
                      iteration_count);
        std::unordered_set<Instance*>      visited;
        std::unordered_set<psn::Instance*> deleted;
        auto                               instances = handler.instances();
        for (auto instance : instances)
        {
            if (deleted.count(instance))
            {
                continue;
            }
            auto instance_lib_cell = handler.libraryCell(instance);
            if (tiehi_cells.count(instance_lib_cell))
            {
                auto output_pin = handler.outputPins(instance)[0];
                PSN_LOG_DEBUG("TieHi Instance {}", handler.name(instance));
                propagateTieHiLoCell(psn_inst, true, output_pin, max_depth,
                                     instance, first_tilo, visited, deleted);
            }
            else if (tielo_cells.count(instance_lib_cell))
            {
                auto output_pin = handler.outputPins(instance)[0];
                PSN_LOG_DEBUG("TieLo Instance {}", handler.name(instance));
                propagateTieHiLoCell(psn_inst, false, output_pin, max_depth,
                                     first_tihi, instance, visited, deleted);
            }
        }
    }

    return prop_count_;
}
bool
ConstantPropagationTransform::isNumber(const std::string& s)
{
    std::istringstream iss(s);
    float              f;
    iss >> std::noskipws >> f;
    return iss.eof() && !iss.fail();
}

int
ConstantPropagationTransform::run(Psn* psn_inst, std::vector<std::string> args)
{
    std::string tiehi_cell_name, tielo_cell_name;
    int         max_depth = -1;
    if (args.size() > 3)
    {
        PSN_LOG_ERROR(help());
        return -1;
    }
    if (args.size() >= 1)
    {
        if (!isNumber(args[0]))
        {
            PSN_LOG_ERROR(help());
            return -1;
        }
        max_depth = atoi(args[0].c_str());
        if (max_depth < -1)
        {
            PSN_LOG_ERROR(help());
            return -1;
        }
    }
    if (args.size() >= 2)
    {
        tielo_cell_name = args[1];
    }
    if (args.size() >= 3)
    {
        tiehi_cell_name = args[2];
    }
    prop_count_ = 0;

    return propagateConstants(psn_inst, tiehi_cell_name, tielo_cell_name,
                              max_depth);
}
