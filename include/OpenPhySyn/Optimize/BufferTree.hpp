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

#include <unordered_set>
#include "OpenPhySyn//Utils/IntervalMap.hpp"
#include "OpenPhySyn/Database/Types.hpp"
#include "OpenPhySyn/Liberty/LibraryMapping.hpp"
#include "OpenPhySyn/Optimize/SteinerTree.hpp"
#include "opendb/geom.h"

#include <memory>

namespace psn
{

class Psn;

enum BufferMode
{
    TimingDriven,
    Timerless
};

enum RepairTarget
{
    RepairMaxCapacitance,
    RepairMaxTransition,
    RepairSlack,
};
enum DesignPhase
{
    PostGlobalPlace,
    PostDetailedPlace,
    PostRoute
};

class BufferTree
{
    float capacitance_;
    float required_or_slew_; // required time for timing-driven and slew for
                             // timerless
    float                                   wire_capacitance_;
    float                                   wire_delay_or_slew_;
    float                                   cost_;
    Point                                   location_;
    std::shared_ptr<BufferTree>             left_, right_;
    LibraryCell*                            buffer_cell_;
    InstanceTerm*                           pin_;
    LibraryTerm*                            library_pin_;
    LibraryCell*                            upstream_buffer_cell_;
    LibraryCell*                            driver_cell_;
    int                                     polarity_;
    int                                     buffer_count_;
    BufferMode                              mode_;
    std::shared_ptr<LibraryCellMappingNode> library_mapping_node_;
    Point                                   driver_location_;

public:
    BufferTree(float cap = 0.0, float req = 0.0, float cost = 0.0,
               Point location = Point(0, 0), LibraryTerm* library_pin = nullptr,
               InstanceTerm* pin = nullptr, LibraryCell* buffer_cell = nullptr,
               int        polarity    = 0,
               BufferMode buffer_mode = BufferMode::TimingDriven);
    BufferTree(Psn* psn_inst, std::shared_ptr<BufferTree> left,
               std::shared_ptr<BufferTree> right, Point location);
    float         totalCapacitance() const;
    float         capacitance() const;
    float         requiredOrSlew() const;
    float         totalRequiredOrSlew() const;
    float         wireCapacitance() const;
    float         wireDelayOrSlew() const;
    float         cost() const;
    int           polarity() const;
    InstanceTerm* pin() const;
    bool checkLimits(Psn* psn_inst, LibraryTerm* driver_pin, float slew_limit,
                     float cap_limit);

    void setDriverLocation(Point loc);

    Point driverLocation() const;

    void setLibraryMappingNode(std::shared_ptr<LibraryCellMappingNode> node);

    std::shared_ptr<LibraryCellMappingNode> libraryMappingNode() const;

    void setCapacitance(float cap);
    void setRequiredOrSlew(float req);
    void setWireCapacitance(float cap);
    void setWireDelayOrSlew(float delay);
    void setCost(float cost);
    void setPolarity(int polarity);
    void setPin(InstanceTerm* pin);
    void setLibraryPin(LibraryTerm* library_pin);

    void setMode(BufferMode buffer_mode);

    BufferMode mode() const;

    float bufferSlew(Psn* psn_inst, LibraryCell* buffer_cell,
                     float tr_slew = 0.0);
    float bufferFixedInputSlew(Psn* psn_inst, LibraryCell* buffer_cell);

    float bufferRequired(Psn* psn_inst, LibraryCell* buffer_cell) const;
    float bufferRequired(Psn* psn_inst) const;
    float upstreamBufferRequired(Psn* psn_inst) const;

    bool hasDownstreamSlewViolation(Psn* psn_inst, float slew_limit,
                                    float tr_slew = 0.0);

    LibraryTerm*                 libraryPin() const;
    std::shared_ptr<BufferTree>& left();
    std::shared_ptr<BufferTree>& right();
    void                         setLeft(std::shared_ptr<BufferTree> left);
    void                         setRight(std::shared_ptr<BufferTree> right);
    bool                         hasUpstreamBufferCell() const;
    bool                         hasBufferCell() const;
    bool                         hasDriverCell() const;

    LibraryCell* bufferCell() const;
    LibraryCell* upstreamBufferCell() const;
    LibraryCell* driverCell() const;
    void         setBufferCell(LibraryCell* buffer_cell);
    void         setUpstreamBufferCell(LibraryCell* buffer_cell);
    void         setDriverCell(LibraryCell* driver_cell);

    Point location() const;

    bool isBufferNode() const;
    bool isLoadNode() const;
    bool isBranched() const;
    bool operator<(const BufferTree& other) const;
    bool operator>(const BufferTree& other) const;
    bool operator<=(const BufferTree& other) const;
    bool operator>=(const BufferTree& other) const;
    void setBufferCount(int buffer_count);
    int  bufferCount() const;
    int  count();
    int  branchCount() const;
    void logInfo() const;
    void logDebug() const;
    bool isTimerless() const;
};

class TimerlessBufferTree : public BufferTree
{
public:
    TimerlessBufferTree(float cap = 0.0, float req = 0.0, float cost = 0.0,
                        Point         location    = Point(0, 0),
                        LibraryTerm*  library_pin = nullptr,
                        InstanceTerm* pin         = nullptr,
                        LibraryCell* buffer_cell = nullptr, int polarity = 0);
    TimerlessBufferTree(Psn* psn_inst, std::shared_ptr<BufferTree> left,
                        std::shared_ptr<BufferTree> right, Point location);
};

class OptimizationOptions
{
public:
    OptimizationOptions() : buffer_lib_lookup(0), inverter_lib_lookup(0)
    {
        max_iterations                    = 1;
        min_gain                          = 0;
        area_penalty                      = 0.0;
        cluster_buffers                   = false;
        cluster_inverters                 = false;
        minimize_cluster_buffers          = false;
        cluster_threshold                 = 0.0;
        driver_resize                     = false;
        repair_capacitance_violations     = false;
        repair_transition_violations      = false;
        repair_negative_slack             = false;
        timerless                         = false;
        timerless_rebuffer                = false;
        timerless_slew_limit_factor       = 0.9;
        timerless_maximum_violation_ratio = 0.7;
        ripup_existing_buffer_max_levels  = 0;
        use_library_lookup                = true;
        legalization_frequency            = 0;
        repair_by_resize                  = false;
        repair_by_clone                   = false;
        repair_by_resynthesis             = false;
        phase                             = DesignPhase::PostGlobalPlace;
        use_best_solution_threshold       = true;
        best_solution_threshold           = 10E-12; // 10ps
        best_solution_threshold_range     = 3;      // Check the top 3 solutions
        minimum_upstream_resistance       = 120;
    }
    float                            initial_area;
    int                              max_iterations;
    float                            min_gain;
    float                            area_penalty;
    bool                             cluster_buffers;
    bool                             cluster_inverters;
    bool                             minimize_cluster_buffers;
    float                            cluster_threshold;
    bool                             driver_resize;
    bool                             repair_capacitance_violations;
    bool                             repair_transition_violations;
    bool                             repair_negative_slack;
    bool                             timerless;
    bool                             timerless_rebuffer;
    float                            timerless_maximum_violation_ratio;
    float                            timerless_slew_limit_factor;
    int                              ripup_existing_buffer_max_levels;
    bool                             use_library_lookup;
    int                              legalization_frequency;
    std::vector<LibraryCell*>        buffer_lib;
    std::vector<LibraryCell*>        inverter_lib;
    std::unordered_set<LibraryCell*> buffer_lib_set;
    std::unordered_set<LibraryCell*> inverter_lib_set;
    IntervalMap<int, LibraryCell*>   buffer_lib_lookup;
    IntervalMap<int, LibraryCell*>   inverter_lib_lookup;
    bool                             repair_by_resize;
    bool                             repair_by_clone;
    bool                             repair_by_resynthesis;
    DesignPhase                      phase;
    bool                             use_best_solution_threshold;
    float                            best_solution_threshold;
    size_t                           best_solution_threshold_range;
    float                            minimum_upstream_resistance;
};

class BufferSolution
{
    std::vector<std::shared_ptr<BufferTree>> buffer_trees_;
    BufferMode                               mode_;

public:
    BufferSolution(BufferMode buffer_mode = BufferMode::TimingDriven);
    BufferSolution(Psn* psn_inst, std::shared_ptr<BufferSolution>& left,
                   std::shared_ptr<BufferSolution>& right, Point location,
                   LibraryCell* upstream_res_cell,
                   float        minimum_upstream_res_or_max_slew,
                   BufferMode   buffer_mode = BufferMode::TimingDriven);

    static std::shared_ptr<BufferSolution>
    bottomUp(Psn* psn_inst, InstanceTerm* driver_pin, SteinerPoint pt,
             SteinerPoint prev, std::shared_ptr<SteinerTree> st_tree,
             std::unique_ptr<OptimizationOptions>& options);

    static std::shared_ptr<BufferSolution> bottomUpWithResynthesis(
        Psn* psn_inst, InstanceTerm* driver_pin, SteinerPoint pt,
        SteinerPoint prev, std::shared_ptr<SteinerTree> st_tree,
        std::unique_ptr<OptimizationOptions>&                 options,
        std::vector<std::shared_ptr<LibraryCellMappingNode>>& mapping_terminals);

    static void topDown(Psn* psn_inst, Net* net,
                        std::shared_ptr<BufferTree> tree, float& area,
                        int& net_index, int& buff_index,
                        std::unordered_set<Instance*>& added_buffers,
                        std::unordered_set<Net*>&      affected_nets);
    static void topDown(Psn* psn_inst, InstanceTerm* pin,
                        std::shared_ptr<BufferTree> tree, float& area,
                        int& net_index, int& buff_index,
                        std::unordered_set<Instance*>& added_buffers,
                        std::unordered_set<Net*>&      affected_nets);

    void mergeBranches(Psn* psn_inst, std::shared_ptr<BufferSolution>& left,
                       std::shared_ptr<BufferSolution>& right, Point location,
                       LibraryCell* upstream_res_cell,
                       float        minimum_upstream_res_or_max_slew);
    void addTree(std::shared_ptr<BufferTree>& tree);
    std::vector<std::shared_ptr<BufferTree>>& bufferTrees();
    void addWireDelayAndCapacitance(float wire_res, float wire_cap);
    void addWireSlewAndCapacitance(float wire_res, float wire_cap);

    void addLeafTrees(Psn* psn_inst, InstanceTerm*, Point pt,
                      std::vector<LibraryCell*>& buffer_lib,
                      std::vector<LibraryCell*>& inverter_lib,
                      float                      slew_limit = 0.0);
    void addLeafTreesWithResynthesis(
        Psn* psn_inst, InstanceTerm*, Point pt,
        std::vector<LibraryCell*>& buffer_lib,
        std::vector<LibraryCell*>& inverter_lib,
        std::vector<std::shared_ptr<LibraryCellMappingNode>>&
            mappings_terminals);
    void addUpstreamReferences(Psn*                        psn_inst,
                               std::shared_ptr<BufferTree> base_buffer_tree);

    std::shared_ptr<BufferTree> optimalRequiredTree(Psn* psn_inst);
    std::shared_ptr<BufferTree>
    optimalDriverTreeWithResize(Psn* psn_inst, InstanceTerm* driver_pin,
                                std::vector<LibraryCell*> driver_types,
                                float                     area_penalty);
    std::shared_ptr<BufferTree>
    optimalDriverTreeWithResynthesis(Psn* psn_inst, InstanceTerm* driver_pin,
                                     float  area_penalty,
                                     float* tree_slack = nullptr);

    std::shared_ptr<BufferTree>
    optimalTimerlessDriverTree(Psn* psn_inst, InstanceTerm* driver_pin);
    std::shared_ptr<BufferTree>
    optimalDriverTree(Psn* psn_inst, InstanceTerm* driver_pin,
                      std::shared_ptr<BufferTree>& inverted_sol,
                      float*                       tree_slack = nullptr);
    std::shared_ptr<BufferTree>
    optimalCapacitanceTree(Psn* psn_inst, InstanceTerm* driver_pin,
                           std::shared_ptr<BufferTree>& inverted_sol,
                           float                        cap_limit);
    std::shared_ptr<BufferTree>
    optimalSlewTree(Psn* psn_inst, InstanceTerm* driver_pin,
                    std::shared_ptr<BufferTree>& inverted_sol,
                    float                        slew_limit);

    std::shared_ptr<BufferTree>
    optimalCostTree(Psn* psn_inst, InstanceTerm* driver_pin,
                    std::shared_ptr<BufferTree>& inverted_sol, float slew_limit,
                    float cap_limit);

    static bool isGreater(float first, float second, float threshold = 1E-6F);
    static bool isLess(float first, float second, float threshold = 1E-6F);
    static bool isEqual(float first, float second, float threshold = 1E-6F);
    static bool isLessOrEqual(float first, float second, float threshold);
    static bool isGreaterOrEqual(float first, float second, float threshold);

    void prune(Psn* psn_inst, LibraryCell* upstream_res_cell,
               float       minimum_upstream_res_or_max_slew,
               const float cap_prune_threshold  = 1E-6F,
               const float cost_prune_threshold = 1E-6F);
    void setMode(BufferMode buffer_mode);

    BufferMode mode() const;

    bool isTimerless() const;
}; // namespace psn

class TimerlessBufferSolution : public BufferSolution
{

public:
    TimerlessBufferSolution();
    TimerlessBufferSolution(Psn*                             psn_inst,
                            std::shared_ptr<BufferSolution>& left,
                            std::shared_ptr<BufferSolution>& right,
                            Point location, LibraryCell* upstream_res_cell,
                            float      minimum_upstream_res_or_max_slew,
                            BufferMode buffer_mode = BufferMode::TimingDriven);
};
} // namespace psn
