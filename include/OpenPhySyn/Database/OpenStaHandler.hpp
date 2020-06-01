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
#ifdef USE_OPENSTA_DB_HANDLER
#pragma once

#include "OpenPhySyn/Database/Types.hpp"
#include "OpenPhySyn/Sta/PathPoint.hpp"

#include <bitset>
#include <functional>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
namespace sta
{
class TimingArc;
class RiseFall;
class ParasiticAnalysisPt;
class Pvt;
class Corner;
class Path;
class Vertex;
class FuncExpr;
class MinMax;
class PathEnd;
class ParasiticNode;
class DcalcAnalysisPt;
class Parasitic;
} // namespace sta

namespace psn
{
class Psn;
class SteinerTree;
class LibraryCellMapping;
typedef int                      SteinerPoint;
typedef std::function<bool(int)> Legalizer;
class OpenStaHandler
{

public:
    OpenStaHandler(Psn* psn_inst, DatabaseSta* sta);

#include "OpenPhySyn/Database/DatabaseHandler.in"

    DatabaseStaNetwork* network() const;
    DatabaseSta*        sta() const;
    virtual ~OpenStaHandler();

    virtual int evaluateFunctionExpression(
        sta::FuncExpr*                         func,
        std::unordered_map<LibraryTerm*, int>& inputs) const;
    Vertex* vertex(InstanceTerm* term) const;

private:
    std::vector<Liberty*> allLibs() const;

    DatabaseSta* sta_;
    Database*    db_;

    const sta::MinMax*        min_max_;
    bool                      has_buffer_inverter_seq_;
    bool                      has_equiv_cells_;
    bool                      has_target_loads_;
    float                     res_per_micron_;
    float                     cap_per_micron_;
    bool                      has_wire_rc_;
    Psn*                      psn_;
    std::vector<LibraryCell*> buffer_inverter_seq_;
    float                     maximum_area_;
    bool                      maximum_area_valid_;

    std::unordered_set<LibraryCell*> nand_cells_;
    std::unordered_set<LibraryCell*> and_cells_;
    std::unordered_set<LibraryCell*> nor_cells_;
    std::unordered_set<LibraryCell*> or_cells_;
    std::unordered_set<LibraryCell*> xor_cells_;
    std::unordered_set<LibraryCell*> xnor_cells_;

    std::unordered_set<LibraryCell*> dont_use_;

    std::unordered_map<LibraryCell*, float> buffer_penalty_map_;
    std::unordered_map<LibraryCell*, float> inverting_buffer_penalty_map_;
    std::unordered_set<LibraryCell*>        non_inverting_buffer_;
    std::unordered_set<LibraryCell*>        inverting_buffer_;
    std::unordered_map<LibraryTerm*, std::unordered_set<LibraryTerm*>>
        commutative_pins_cache_;

    std::unordered_map<float, float>
        penalty_cache_; // TODO Convert into indexing table

    std::unordered_map<std::string, std::shared_ptr<LibraryCellMapping>>
                                                  library_cell_mappings_;
    std::unordered_map<LibraryCell*, std::string> truth_tables_;
    std::unordered_map<std::string, std::unordered_set<LibraryCell*>>
        function_to_cell_; // Mapping from truth table to cells

    bool has_library_cell_mappings_;

    void populatePrimitiveCellCache();

    int computeTruthTable(LibraryCell* cell);

    std::unordered_map<LibraryCell*, float> target_load_map_;

    // Vertex* vertex(InstanceTerm* term) const;

    void computeBuffersDelayPenalty(bool include_inverting = true);

    /* The following code is borrowed from James Cherry's Resizer Code */
    const sta::Corner*              corner_;
    const sta::DcalcAnalysisPt*     dcalc_ap_;
    const sta::Pvt*                 pvt_;
    const sta::ParasiticAnalysisPt* parasitics_ap_;
    float                           target_slews_[2];
    float pinTableAverage(LibraryTerm* from, LibraryTerm* to,
                          bool is_delay = true, bool is_rise = true) const;
    float pinTableLookup(LibraryTerm* from, LibraryTerm* to, float slew,
                         float cap, bool is_delay = true,
                         bool is_rise = true) const;
    std::vector<std::vector<PathPoint>> getPaths(bool get_max,
                                                 int  path_count = 1) const;
    std::vector<PathPoint>              expandPath(sta::PathEnd* path_end,
                                                   bool          enumed = false) const;
    std::vector<PathPoint>              expandPath(sta::Path* path,
                                                   bool       enumed = false) const;
    void                                findTargetLoads();
    void                                makeEquivalentCells();

    void  findTargetLoads(std::vector<Liberty*>* resize_libs);
    void  findTargetLoads(Liberty* library, float slews[]);
    void  findTargetLoad(LibraryCell* cell, float slews[]);
    float findTargetLoad(LibraryCell* cell, sta::TimingArc* arc, float in_slew,
                         float out_slew);
    float targetSlew(const sta::RiseFall* rf);
    void  findBufferTargetSlews(std::vector<Liberty*>* resize_libs);
    void  findBufferTargetSlews(Liberty* library, float slews[], int counts[]);
    void  slewLimit(InstanceTerm* pin, sta::MinMax* min_max, float& limit,
                    bool& exists) const;
    sta::ParasiticNode* findParasiticNode(std::unique_ptr<SteinerTree>& tree,
                                          sta::Parasitic*     parasitic,
                                          const Net*          net,
                                          const InstanceTerm* pin,
                                          SteinerPoint        pt);
    std::function<bool(float)> legalizer_;
};

} // namespace psn
#endif