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

#include <cstring>
#include <memory>
#include <unordered_set>
#include "BufferTree.hpp"
#include "OpenPhySyn/Database/DatabaseHandler.hpp"
#include "OpenPhySyn/Database/Types.hpp"
#include "OpenPhySyn/Psn/Psn.hpp"
#include "OpenPhySyn/SteinerTree/SteinerTree.hpp"
#include "OpenPhySyn/Transform/PsnTransform.hpp"
#include "OpenPhySyn/Utils/IntervalMap.hpp"

namespace psn
{

enum TimingRepairPhase
{
    PostGlobalPlace,
    PostDetailedPlace,
    PostRoue
};
enum TimingRepairTarget
{
    RepairMaxCapacitance,
    RepairMaxTransition,
    RepairSlack,
};

class TimingBufferTransformOptions
{
public:
    TimingBufferTransformOptions()
        : buffer_lib_lookup(0), inverter_lib_lookup(0)
    {
        max_iterations                    = 1;
        min_gain                          = 0;
        area_penalty                      = 0.0;
        cluster_buffers                   = false;
        cluster_inverters                 = false;
        minimize_cluster_buffers          = false;
        cluster_threshold                 = 0.0;
        resize_gates                      = false;
        repair_capacitance_violations     = false;
        repair_transition_violations      = false;
        timerless                         = false;
        timerless_rebuffer                = false;
        timerless_slew_limit_factor       = 0.9;
        timerless_maximum_violation_ratio = 0.7;
        cirtical_path                     = false;
        maximize_slack                    = false;
        ripup_existing_buffer_max_levels  = 3;
        use_library_lookup                = true;
        legalization_frequency            = 0;
        phase                             = TimingRepairPhase::PostGlobalPlace;
        use_best_solution_threshold       = true;
        best_solution_threshold           = 10E-12; // 10ps
        best_solution_threshold_range     = 3;      // Check the top 3 solutions
        minimum_upstream_resistance       = 120;
    }
    float                          initial_area;
    int                            max_iterations;
    float                          min_gain;
    float                          area_penalty;
    bool                           cluster_buffers;
    bool                           cluster_inverters;
    bool                           minimize_cluster_buffers;
    float                          cluster_threshold;
    bool                           resize_gates;
    bool                           repair_capacitance_violations;
    bool                           repair_transition_violations;
    bool                           timerless;
    bool                           timerless_rebuffer;
    float                          timerless_maximum_violation_ratio;
    float                          timerless_slew_limit_factor;
    bool                           cirtical_path;
    bool                           maximize_slack;
    int                            ripup_existing_buffer_max_levels;
    bool                           use_library_lookup;
    int                            legalization_frequency;
    std::vector<LibraryCell*>      buffer_lib;
    std::vector<LibraryCell*>      inverter_lib;
    IntervalMap<int, LibraryCell*> buffer_lib_lookup;
    IntervalMap<int, LibraryCell*> inverter_lib_lookup;
    TimingRepairPhase              phase;
    bool                           use_best_solution_threshold;
    float                          best_solution_threshold;
    size_t                         best_solution_threshold_range;
    float                          minimum_upstream_resistance;
};

class TimingBufferTransform : public PsnTransform
{

private:
    int   buffer_count_;
    int   resize_count_;
    int   timerless_rebuffer_count_;
    int   net_count_;
    int   net_index_;
    int   buff_index_;
    int   transition_violations_;
    int   capacitance_violations_;
    float current_area_;
    int   hasViolation(Psn* psn_inst, InstanceTerm* pin);
    std::unordered_set<Instance*>
    bufferPin(Psn* psn_inst, InstanceTerm* pin, TimingRepairTarget target,
              std::unique_ptr<TimingBufferTransformOptions>& options);
    std::shared_ptr<BufferSolution>
    bottomUp(Psn* psn_inst, InstanceTerm* driver_pin, SteinerPoint pt,
             SteinerPoint prev, std::shared_ptr<SteinerTree> st_tree,
             TimingRepairTarget                             target,
             std::unique_ptr<TimingBufferTransformOptions>& options);
    std::shared_ptr<BufferSolution>
         bottomUpTimerless(Psn* psn_inst, InstanceTerm* driver_pin, SteinerPoint pt,
                           SteinerPoint prev, std::shared_ptr<SteinerTree> st_tree,
                           TimingRepairTarget                             target,
                           std::unique_ptr<TimingBufferTransformOptions>& options);
    void topDown(Psn* psn_inst, Net* net, std::shared_ptr<BufferTree> tree,
                 std::unordered_set<Instance*>& added_buffers);
    void topDown(Psn* psn_inst, InstanceTerm* pin,
                 std::shared_ptr<BufferTree>    tree,
                 std::unordered_set<Instance*>& added_buffers);

    int timingBuffer(Psn*                                           psn_inst,
                     std::unique_ptr<TimingBufferTransformOptions>& options,
                     std::unordered_set<std::string> buffer_lib_names =
                         std::unordered_set<std::string>(),
                     std::unordered_set<std::string> inverter_lib_names =
                         std::unordered_set<std::string>());
    int fixCapacitanceViolations(
        Psn* psn_inst, std::vector<InstanceTerm*>& driver_pins,
        std::unique_ptr<TimingBufferTransformOptions>& options);
    int fixTransitionViolations(
        Psn* psn_inst, std::vector<InstanceTerm*>& driver_pins,
        std::unique_ptr<TimingBufferTransformOptions>& options);

public:
    TimingBufferTransform();

    int run(Psn* psn_inst, std::vector<std::string> args) override;
};

DEFINE_TRANSFORM(
    TimingBufferTransform, "timing_buffer", "1.5",
    "Performs several variations of buffering and resizing to fix timing "
    "violations",
    "Usage: transform timing_buffer [-maximum_capacitance] "
    "[-maximum_transition] [-auto_buffer_library "
    "<single|small|medium|large|all>] [-minimize_buffer_library] "
    "[-use_inverting_buffer_library] [-buffers "
    "<buffer library>] [-inverters "
    "<inverters library>] [-timerless] [-cirtical_path] [-iterations <# "
    "iterations=1>] [-postGlobalPlace|-postDetailedPlace|-postRoute] "
    "[-legalization_frequency <numBuffer>]"
    "[-min_gain "
    "<gain=0ps>] [-enable_gate_resize] [-area_penalty <penalty=0ps/um>]")

} // namespace psn
