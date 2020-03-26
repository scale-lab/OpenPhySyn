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

#include "TimingBufferTransform.hpp"
#include <OpenPhySyn/PsnLogger/PsnLogger.hpp>
#include <OpenPhySyn/Sta/PathPoint.hpp>
#include <OpenPhySyn/Utils/PsnGlobal.hpp>
#include <OpenPhySyn/Utils/StringUtils.hpp>
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
// Objectives:
// * Standard Van Ginneken buffering (with pruning). [Done]
// * Multiple buffer sizes. [Done]
// * Inverter pair instead of buffer pairs. [Done]
// * Simultaneous buffering and gate sizing. [Done]
// * Buffer library pruning. [Done]
// * Buffer library lookup. [TODO]
// * Squeeze pruning. [TODO]
// * Preslack pruning. [Done]
// * Timerless buffering. [InProgress]
// * Layout aware buffering. [TODO]
// * Logic aware buffering. [TODO]

using namespace psn;

TimingBufferTransform::TimingBufferTransform()
    : buffer_count_(0),
      resize_count_(0),
      net_index_(0),
      buff_index_(0),
      transition_violations_(0),
      capacitance_violations_(0),
      current_area_(0.0)
{
}

void
TimingBufferTransform::bufferPin(
    Psn* psn_inst, InstanceTerm* pin,
    std::unique_ptr<TimingBufferTransformOptions>& options)
{
    DatabaseHandler& handler = *(psn_inst->handler());
    if (handler.isTopLevel(pin))
    {
        PSN_LOG_WARN("Not handled yet!");
        return;
    }
    auto pin_net = handler.net(pin);
    auto st_tree = SteinerTree::create(pin_net, psn_inst);
    if (!st_tree)
    {
        PSN_LOG_ERROR("Failed to create steiner tree for {}",
                      handler.name(pin));
        return;
    }

    auto driver_point = st_tree->driverPoint();
    auto driver_pin   = st_tree->pin(driver_point);
    PSN_LOG_DEBUG("{} Slew Limit {}", handler.name(driver_pin),
                  handler.pinSlewLimit(driver_pin));
    auto top_point = st_tree->top();
    auto buff_sol =
        bottomUp(psn_inst, top_point, driver_point, options->buffer_lib,
                 options->inverter_lib, std::move(st_tree),
                 options->minimum_upstresm_resistance);
    if (buff_sol->bufferTrees().size())
    {
        std::shared_ptr<BufferTree> buff_tree    = nullptr;
        auto                        no_buff_tree = buff_sol->bufferTrees()[0];
        auto                        driver_cell  = handler.instance(pin);
        auto driver_lib = handler.libraryCell(driver_cell);
        if (options->resize_gates && driver_cell &&
            handler.outputPins(driver_cell).size() == 1)
        {
            buff_tree = buff_sol->optimalDriverTree(psn_inst, pin);
            auto driver_types =
                handler.equivalentCells(handler.libraryCell(driver_cell));
            if (driver_types.size() == 1)
            {
                buff_tree = buff_sol->optimalDriverTree(psn_inst, pin);
            }
            else
            {
                buff_tree = buff_sol->optimalDriverTree(
                    psn_inst, pin, driver_types, options->area_penalty);
            }
        }
        else
        {
            buff_tree = buff_sol->optimalDriverTree(psn_inst, pin);
        }

        if (buff_tree)
        {
            if (options->use_best_solution_threshold)
            {
                float buff_tree_delay =
                    handler.gateDelay(pin, buff_tree->totalCapacitance());
                float buff_tree_slack =
                    buff_tree->totalRequired() - buff_tree_delay;
                for (size_t i = 1; i < buff_sol->bufferTrees().size() &&
                                   i < options->best_solution_threshold_range;
                     i++)
                {
                    auto& tr = buff_sol->bufferTrees()[i];
                    float tr_delay =
                        handler.gateDelay(pin, tr->totalCapacitance());
                    float tr_slack = tr->totalRequired() - tr_delay;
                    if (buff_tree_slack - tr_slack <
                            options->best_solution_threshold &&
                        tr->cost() < buff_tree->cost())
                    {
                        buff_tree       = tr;
                        buff_tree_delay = tr_delay;
                        buff_tree_slack = tr_slack;
                    }
                }
            }

            auto replace_driver = (buff_tree->hasDriverCell() &&
                                   buff_tree->driverCell() != driver_lib)
                                      ? buff_tree->driverCell()
                                      : nullptr;
            float old_delay =
                handler.gateDelay(pin, no_buff_tree->totalCapacitance());
            float old_slack = no_buff_tree->totalRequired() - old_delay;

            float new_delay =
                handler.gateDelay(pin, buff_tree->totalCapacitance());
            float new_slack = buff_tree->totalRequired() - new_delay;

            float gain = new_slack - old_slack;

            if (buff_tree->cost() <= std::numeric_limits<float>::epsilon() ||
                std::fabs(gain - options->min_gain) >=
                    -std::numeric_limits<float>::epsilon())
            {
                topDown(psn_inst, pin, buff_tree);
                if (replace_driver)
                {
                    handler.replaceInstance(driver_cell, replace_driver);
                    current_area_ -= handler.area(driver_lib);
                    current_area_ += handler.area(replace_driver);
                    resize_count_++;
                }
            }
            else
            {
                PSN_LOG_DEBUG("Weak solution: ");
                buff_tree->logDebug();
            }
        }
    }
}

std::shared_ptr<BufferSolution>
TimingBufferTransform::bottomUp(Psn* psn_inst, SteinerPoint pt,
                                SteinerPoint                 prev,
                                std::vector<LibraryCell*>&   buffer_lib,
                                std::vector<LibraryCell*>&   inverter_lib,
                                std::shared_ptr<SteinerTree> st_tree,
                                float minimum_upstream_resistance)
{
    DatabaseHandler& handler = *(psn_inst->handler());
    if (pt != SteinerNull)
    {
        auto  pt_pin = st_tree->pin(pt);
        float wire_length =
            psn_inst->handler()->dbuToMeters(st_tree->distance(prev, pt));
        float wire_res =
            wire_length * psn_inst->handler()->resistancePerMicron();
        float wire_cap =
            wire_length * psn_inst->handler()->capacitancePerMicron();
        float wire_delay    = wire_cap * wire_res;
        auto  location      = st_tree->location(pt);
        auto  prev_location = st_tree->location(prev);
        PSN_LOG_DEBUG("Point: ({}, {})", location.getX(), location.getY());
        PSN_LOG_DEBUG("Prev: ({}, {})", prev_location.getX(),
                      prev_location.getY());

        if (pt_pin && handler.isLoad(pt_pin))
        {
            PSN_LOG_DEBUG("{} ({}, {}) bottomUp leaf", handler.name(pt_pin),
                          location.getX(), location.getY());
            float cap = handler.pinCapacitance(pt_pin);
            float req = handler.required(pt_pin);
            auto  base_buffer_tree =
                std::make_shared<BufferTree>(cap, req, 0, location, pt_pin);
            std::shared_ptr<BufferSolution> buff_sol =
                std::make_shared<BufferSolution>();
            buff_sol->addTree(base_buffer_tree);
            buff_sol->addWireDelayAndCapacitance(wire_delay, wire_cap);
            buff_sol->addLeafTrees(psn_inst, prev_location, buffer_lib,
                                   inverter_lib);
            buff_sol->addUpstreamReferences(psn_inst, base_buffer_tree);

            return buff_sol;
        }
        else if (!pt_pin)
        {
            PSN_LOG_DEBUG("({}, {}) bottomUp ---> left", location.getX(),
                          location.getY());
            auto left =
                bottomUp(psn_inst, st_tree->left(pt), pt, buffer_lib,
                         inverter_lib, st_tree, minimum_upstream_resistance);
            PSN_LOG_DEBUG("({}, {}) bottomUp ---> right", location.getX(),
                          location.getY());
            auto right =
                bottomUp(psn_inst, st_tree->right(pt), pt, buffer_lib,
                         inverter_lib, st_tree, minimum_upstream_resistance);

            PSN_LOG_DEBUG("({}, {}) bottomUp merging", location.getX(),
                          location.getY());
            std::shared_ptr<BufferSolution> buff_sol =
                std::make_shared<BufferSolution>(
                    psn_inst, left, right, location,
                    buffer_lib[buffer_lib.size() / 2],
                    minimum_upstream_resistance);
            buff_sol->addWireDelayAndCapacitance(wire_delay, wire_cap);

            buff_sol->addLeafTrees(psn_inst, prev_location, buffer_lib,
                                   inverter_lib);
            return buff_sol;
        }
    }
    return nullptr;
}
void
TimingBufferTransform::topDown(Psn* psn_inst, InstanceTerm* pin,
                               std::shared_ptr<BufferTree> tree)
{
    auto net = psn_inst->handler()->net(pin);
    if (!net)
    {
        net = psn_inst->handler()->net(psn_inst->handler()->term(pin));
    }
    if (!net)
    {
        PSN_LOG_ERROR("No net for {} !", psn_inst->handler()->name(pin));
    }
    topDown(psn_inst, net, tree);
}
void
TimingBufferTransform::topDown(Psn* psn_inst, Net* net,
                               std::shared_ptr<BufferTree> tree)
{
    DatabaseHandler& handler = *(psn_inst->handler());
    if (!net)
    {
        PSN_LOG_WARN("topDown buffering without target net!");
        return;
    }
    if (!tree)
    {
        return;
    }
    if (tree->isUnbuffered())
    {
        PSN_LOG_DEBUG("{}: unbuffered at ({}, {})", handler.name(net),
                      tree->location().getX(), tree->location().getY());
        auto tree_pin = tree->pin();
        auto tree_net = handler.net(tree_pin);
        if (!tree_net)
        {
            // Top-level pin
            tree_net = handler.net(handler.term(tree_pin));
        }
        if (tree_net != net)
        {
            auto inst = handler.instance(tree_pin);
            handler.disconnect(tree_pin);
            auto lib_pin = handler.libraryPin(tree_pin);
            if (!lib_pin)
            {
                handler.connect(net, inst, handler.topPort(tree_pin));
            }
            else
            {
                handler.connect(net, inst, handler.libraryPin(tree_pin));
            }
        }
    }
    else if (tree->isBuffered())
    {

        PSN_LOG_DEBUG("{}: adding buffer [{}] at ({}, {})..", handler.name(net),
                      handler.name(tree->bufferCell()), tree->location().getX(),
                      tree->location().getY());
        auto buf_inst = handler.createInstance(
            handler.generateInstanceName("buff_", net_index_).c_str(),
            tree->bufferCell());
        auto buf_net =
            handler.createNet(handler.generateNetName(buff_index_).c_str());
        auto buff_in_port  = handler.bufferInputPin(tree->bufferCell());
        auto buff_out_port = handler.bufferOutputPin(tree->bufferCell());
        handler.connect(net, buf_inst, buff_in_port);
        handler.connect(buf_net, buf_inst, buff_out_port);
        handler.setLocation(buf_inst, tree->location());
        handler.calculateParasitics(net);
        handler.calculateParasitics(buf_net);
        current_area_ += handler.area(tree->bufferCell());
        topDown(psn_inst, buf_net, tree->left());
        buffer_count_++;
    }
    else if (tree->isBranched())
    {
        PSN_LOG_DEBUG("{}: Buffering left..", handler.name(net));
        topDown(psn_inst, net, tree->left());
        PSN_LOG_DEBUG("{}: Buffering right..", handler.name(net));
        topDown(psn_inst, net, tree->right());
    }
}

int
TimingBufferTransform::hasViolation(Psn* psn_inst, InstanceTerm* pin)
{
    DatabaseHandler& handler  = *(psn_inst->handler());
    auto             pin_net  = handler.net(pin);
    auto             net_pins = handler.pins(pin_net);
    for (auto connected_pin : net_pins)
    {
        if (handler.violatesMaximumTransition(connected_pin))
        {
            return 1;
        }
        else if (handler.violatesMaximumCapacitance(connected_pin))
        {
            return 2;
        }
    }
    return 0;
}

int
TimingBufferTransform::fixCapacitanceViolations(
    Psn* psn_inst, std::vector<InstanceTerm*>& driver_pins,
    std::unique_ptr<TimingBufferTransformOptions>& options)
{
    PSN_LOG_DEBUG("Fixing capacitance violations");
    DatabaseHandler& handler    = *(psn_inst->handler());
    auto             clock_nets = handler.clockNets();
    for (auto& pin : driver_pins)
    {
        auto pin_net = handler.net(pin);
        if (pin_net && !clock_nets.count(pin_net))
        {
            auto net_pins = handler.pins(pin_net);
            bool fix      = false;
            for (auto connected_pin : net_pins)
            {
                if (handler.violatesMaximumCapacitance(connected_pin))
                {
                    capacitance_violations_++;
                    PSN_LOG_DEBUG("Violating pin {}", handler.name(pin));
                    fix = true;
                    break;
                }
            }
            if (fix)
            {
                PSN_LOG_DEBUG("Fixing cap. violations for pin {}",
                              handler.name(pin));
                bufferPin(psn_inst, pin, options);
                if (handler.hasMaximumArea() &&
                    current_area_ > handler.maximumArea())
                {
                    PSN_LOG_WARN("Maximum utilization reached");
                    return buffer_count_ + resize_count_;
                }
            }
        }
    }

    return buffer_count_;
}
int
TimingBufferTransform::fixTransitionViolations(
    Psn* psn_inst, std::vector<InstanceTerm*>& driver_pins,
    std::unique_ptr<TimingBufferTransformOptions>& options)
{
    PSN_LOG_DEBUG("Fixing transition violations");
    DatabaseHandler& handler = *(psn_inst->handler());
    handler.resetDelays();
    auto clock_nets = handler.clockNets();
    for (auto& pin : driver_pins)
    {
        auto pin_net = handler.net(pin);

        if (pin_net && !clock_nets.count(pin_net))
        {
            auto net_pins = handler.pins(pin_net);
            bool fix      = false;
            for (auto connected_pin : net_pins)
            {
                if (handler.violatesMaximumTransition(connected_pin))
                {
                    PSN_LOG_DEBUG("Violating pin {}", handler.name(pin));
                    transition_violations_++;
                    fix = true;
                    break;
                }
            }
            if (fix)
            {
                PSN_LOG_DEBUG("Fixing transition violations for pin {}",
                              handler.name(pin));
                bufferPin(psn_inst, pin, options);
                if (handler.hasMaximumArea() &&
                    current_area_ > handler.maximumArea())
                {
                    PSN_LOG_WARN("Maximum utilization reached");
                    return buffer_count_ + resize_count_;
                }
            }
        }
    }
    return buffer_count_;
}

int
TimingBufferTransform::timingBuffer(
    Psn* psn_inst, std::unique_ptr<TimingBufferTransformOptions>& options,
    std::unordered_set<std::string> buffer_lib_names,
    std::unordered_set<std::string> inverter_lib_names)
{
    DatabaseHandler& handler = *(psn_inst->handler());

    if (options->cluster_buffers)
    {
        PSN_LOG_DEBUG("Generating buffer library");
        auto cluster_libs = handler.bufferClusters(
            options->cluster_threshold, options->minimize_cluster_buffers,
            options->cluster_inverters);
        options->buffer_lib   = cluster_libs.first;
        options->inverter_lib = cluster_libs.second;
        buffer_lib_names.clear();
        inverter_lib_names.clear();
        for (auto& b : options->buffer_lib)
        {
            buffer_lib_names.insert(handler.name(b));
        }
        for (auto& b : options->inverter_lib)
        {
            inverter_lib_names.insert(handler.name(b));
        }
        PSN_LOG_INFO("Using {} Buffers and {} Inverters",
                     options->buffer_lib.size(), options->inverter_lib.size());
    }
    for (auto& buf_name : buffer_lib_names)
    {
        auto lib_cell = handler.libraryCell(buf_name.c_str());
        if (!lib_cell)
        {
            PSN_LOG_ERROR("Buffer cell {} not found in the library.", buf_name);
            return -1;
        }
        options->buffer_lib.push_back(lib_cell);
    }

    for (auto& inv_name : inverter_lib_names)
    {
        auto lib_cell = handler.libraryCell(inv_name.c_str());
        if (!lib_cell)
        {
            PSN_LOG_ERROR("Inverter cell {} not found in the library.",
                          inv_name);
            return -1;
        }
        options->inverter_lib.push_back(lib_cell);
    }
    std::unique(options->buffer_lib.begin(), options->buffer_lib.end());
    std::unique(options->inverter_lib.begin(), options->inverter_lib.end());

    std::sort(options->buffer_lib.begin(), options->buffer_lib.end(),
              [&](LibraryCell* a, LibraryCell* b) -> bool {
                  return handler.area(a) < handler.area(b);
              });
    std::sort(options->inverter_lib.begin(), options->inverter_lib.end(),
              [&](LibraryCell* a, LibraryCell* b) -> bool {
                  return handler.area(a) < handler.area(b);
              });

    auto buf_names_vec = std::vector<std::string>(buffer_lib_names.begin(),
                                                  buffer_lib_names.end());
    auto inv_names_vec = std::vector<std::string>(inverter_lib_names.begin(),
                                                  inverter_lib_names.end());
    PSN_LOG_INFO("Buffer library: {}",
                 buffer_lib_names.size()
                     ? StringUtils::join(buf_names_vec, ", ")
                     : "None");
    PSN_LOG_INFO("Inverter library: {}",
                 inverter_lib_names.size()
                     ? StringUtils::join(inv_names_vec, ", ")
                     : "None");
    PSN_LOG_INFO("Driver sizing {}",
                 options->resize_gates ? "enabled" : "disabled");

    for (int i = 0; i < options->max_iterations; i++)
    {
        PSN_LOG_INFO("Iteration {}", i + 1);
        auto driver_pins = handler.levelDriverPins();
        std::reverse(driver_pins.begin(), driver_pins.end());
        bool hasVio         = false;
        int  pre_fix_buff   = buffer_count_;
        int  pre_fix_resize = resize_count_;
        if (options->repair_capacitance_violations)
        {
            fixCapacitanceViolations(psn_inst, driver_pins, options);
            if (buffer_count_ != pre_fix_buff ||
                resize_count_ != pre_fix_resize)
            {
                hasVio = true;
            }
        }
        if (options->repair_transition_violations)
        {
            fixTransitionViolations(psn_inst, driver_pins, options);
            if (buffer_count_ != pre_fix_buff ||
                resize_count_ != pre_fix_resize)
            {
                hasVio = true;
            }
        }
        if (!hasVio)
        {
            PSN_LOG_DEBUG("No more violations or cannot buffer");
            break;
        }
    }
    PSN_LOG_INFO("Initial area: {}", (int)(options->initial_area * 10E12));
    PSN_LOG_INFO("New area: {}", (int)(current_area_ * 10E12));
    if (options->repair_capacitance_violations)
    {
        PSN_LOG_INFO("Found {} maximum capacitance violations",
                     capacitance_violations_);
    }
    if (options->repair_transition_violations)
    {
        PSN_LOG_INFO("Found {} maximum transition violations",
                     transition_violations_);
    }
    PSN_LOG_INFO("Placed {} buffers", buffer_count_);
    PSN_LOG_INFO("Resized {} gates", resize_count_);
    return buffer_count_ + resize_count_;
}

int
TimingBufferTransform::run(Psn* psn_inst, std::vector<std::string> args)
{
    buffer_count_           = 0;
    resize_count_           = 0;
    transition_violations_  = 0;
    capacitance_violations_ = 0;
    current_area_           = psn_inst->handler()->area();

    std::unique_ptr<TimingBufferTransformOptions> options(
        new TimingBufferTransformOptions);

    options->initial_area                  = current_area_;
    options->max_iterations                = 1;
    options->min_gain                      = 0;
    options->area_penalty                  = 0.0;
    options->cluster_buffers               = false;
    options->cluster_inverters             = false;
    options->minimize_cluster_buffers      = false;
    options->cluster_threshold             = 0.0;
    options->resize_gates                  = false;
    options->repair_capacitance_violations = false;
    options->repair_transition_violations  = false;
    options->timerless                     = false;
    options->cirtical_path                 = false;
    options->phase                         = "postGlobalPlace";
    options->use_best_solution_threshold   = true;
    options->best_solution_threshold       = 10E-12; // 10ps
    options->best_solution_threshold_range = 3; // Check the top 3 solutions
    options->minimum_upstresm_resistance   = 120;

    std::unordered_set<std::string> buffer_lib_names;
    std::unordered_set<std::string> inverter_lib_names;
    std::unordered_set<std::string> keywords(
        {"-buffers", "-inverters", "-enable_gate_resize", "-iterations",
         "-min_gain", "-area_penalty", "-auto_buffer_library",
         "-minimize_buffer_library", "-use_inverting_buffer_library",
         "-timerless", "-cirtical_path", "-postGlobalPlace",
         "-postDetailedPlace", "-postRoute"});

    if (args.size() < 2)
    {
        PSN_LOG_ERROR(help());
        return -1;
    }
    for (size_t i = 0; i < args.size(); i++)
    {
        if (args[i].size() > 2 && args[i][0] == '-' && args[i][1] == '-')
        {
            args[i] = args[i].substr(1);
        }
    }
    for (size_t i = 0; i < args.size(); i++)
    {
        if (args[i] == "-buffers")
        {
            i++;
            while (i < args.size())
            {
                if (args[i] == "-buffers")
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
        else if (args[i] == "-inverters")
        {
            i++;
            while (i < args.size())
            {
                if (args[i] == "-inverters")
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
        else if (args[i] == "-auto_buffer_library")
        {
            options->cluster_buffers = true;
            i++;
            if (i >= args.size())
            {
                PSN_LOG_ERROR(help());
                return -1;
            }
            else
            {
                if (args[i] == "single")
                {
                    options->cluster_threshold = 1.0;
                }
                else if (args[i] == "small")
                {
                    options->cluster_threshold = 3.0 / 4.0;
                }
                else if (args[i] == "medium")
                {
                    options->cluster_threshold = 1.0 / 4.0;
                }
                else if (args[i] == "large")
                {
                    options->cluster_threshold = 1.0 / 12.0;
                }
                else if (args[i] == "all")
                {
                    options->cluster_threshold = 0.0;
                }
                else
                {
                    PSN_LOG_ERROR(help());
                    return -1;
                }
            }
        }
        else if (args[i] == "-iterations")
        {
            i++;
            if (i >= args.size() || !StringUtils::isNumber(args[i]))
            {
                PSN_LOG_ERROR(help());
                return -1;
            }
            else
            {
                options->max_iterations = atoi(args[i].c_str());
            }
        }
        else if (args[i] == "-min_gain")
        {
            i++;
            if (i >= args.size() || !StringUtils::isNumber(args[i]))
            {
                PSN_LOG_ERROR(help());
                return -1;
            }
            else
            {
                options->min_gain = atof(args[i].c_str());
            }
        }
        else if (args[i] == "-area_penalty")
        {
            i++;
            if (i >= args.size() || !StringUtils::isNumber(args[i]))
            {
                PSN_LOG_ERROR(help());
                return -1;
            }
            else
            {
                options->area_penalty = atof(args[i].c_str());
            }
        }
        else if (args[i] == "-enable_gate_resize")
        {
            options->resize_gates = true;
        }
        else if (args[i] == "-maximum_capacitance")
        {
            options->repair_capacitance_violations = true;
        }
        else if (args[i] == "-maximum_transition")
        {
            options->repair_transition_violations = true;
        }
        else if (args[i] == "-timerless")
        {
            options->timerless = true;
        }
        else if (args[i] == "-cirtical_path")
        {
            options->cirtical_path = true;
        }
        else if (args[i] == "-minimize_buffer_library")
        {
            options->minimize_cluster_buffers = true;
        }
        else if (args[i] == "-use_inverting_buffer_library")
        {
            options->cluster_inverters = true;
        }
        else if (args[i] == "-postGlobalPlace")
        {
            options->phase = "postGlobalPlace";
        }
        else if (args[i] == "-postDetailedPlace")
        {
            PSN_LOG_ERROR(
                "Post detailed placement timing repair is not supported");
            options->phase = "postDetailedPlace";
            return -1;
        }
        else if (args[i] == "-postRoute")
        {
            PSN_LOG_ERROR("Post routing timing repair is not supported");
            options->phase = "postRoute";
            return -1;
        }
        else
        {
            PSN_LOG_ERROR(help());
            return -1;
        }
    }

    if (!options->repair_capacitance_violations &&
        !options->repair_transition_violations)
    {
        options->repair_transition_violations  = true;
        options->repair_capacitance_violations = true;
    }

    return timingBuffer(psn_inst, options, buffer_lib_names,
                        inverter_lib_names);
}
