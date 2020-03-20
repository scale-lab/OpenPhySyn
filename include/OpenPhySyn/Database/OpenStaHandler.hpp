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
#ifndef __PSN_OPEN_STA_HANDLER__
#define __PSN_OPEN_STA_HANDLER__

#include <OpenPhySyn/Database/Types.hpp>
#include <OpenPhySyn/Sta/DatabaseSta.hpp>
#include <OpenPhySyn/Sta/DatabaseStaNetwork.hpp>
#include <OpenPhySyn/Sta/PathPoint.hpp>
#include <OpenPhySyn/SteinerTree/SteinerTree.hpp>
#include <OpenPhySyn/Utils/PsnGlobal.hpp>
#include <PsnLogger/PsnLogger.hpp>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace psn
{
class OpenStaHandler
{
public:
    OpenStaHandler(Psn* psn_inst, DatabaseSta* sta);

#include <OpenPhySyn/Database/DatabaseHandler.in>

    DatabaseStaNetwork* network() const;
    DatabaseSta*        sta() const;
    virtual ~OpenStaHandler();

    virtual int evaluateFunctionExpression(
        sta::FuncExpr*                         func,
        std::unordered_map<LibraryTerm*, int>& inputs) const;

private:
    sta::LibertyLibrarySeq allLibs() const;
    DatabaseSta*           sta_;
    Database*              db_;

    const sta::MinMax*        min_max_;
    bool                      has_buffer_inverter_seq_;
    bool                      has_equiv_cells_;
    bool                      has_target_loads_;
    float                     res_per_micron_;
    float                     cap_per_micron_;
    bool                      has_wire_rc_;
    Psn*                      psn_;
    std::vector<LibraryCell*> buffer_inverter_seq_;

    std::unordered_map<LibraryCell*, float> buffer_penalty_map_;
    std::unordered_map<LibraryCell*, float> inverting_buffer_penalty_map_;
    std::unordered_set<LibraryCell*>        non_inverting_buffer_;
    std::unordered_set<LibraryCell*>        inverting_buffer_;

    std::unordered_map<float, float>
        penalty_cache_; // TODO Convert into indexing table

    std::unordered_map<LibraryCell*, float> target_load_map_;

    sta::Vertex* vertex(InstanceTerm* term) const;

    void computeBuffersDelayPenalty(bool include_inverting = true);

    /* The following code is borrowed from James Cherry's Resizer Code */
    const sta::Corner*              corner_;
    const sta::DcalcAnalysisPt*     dcalc_ap_;
    const sta::Pvt*                 pvt_;
    const sta::ParasiticAnalysisPt* parasitics_ap_;
    sta::Slew                       target_slews_[sta::RiseFall::index_count];
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

    void      findTargetLoads(sta::LibertyLibrarySeq* resize_libs);
    void      findTargetLoads(Liberty* library, sta::Slew slews[]);
    void      findTargetLoad(LibraryCell* cell, sta::Slew slews[]);
    float     findTargetLoad(LibraryCell* cell, sta::TimingArc* arc,
                             sta::Slew in_slew, sta::Slew out_slew);
    sta::Slew targetSlew(const sta::RiseFall* rf);
    void      findBufferTargetSlews(sta::LibertyLibrarySeq* resize_libs);
    void      findBufferTargetSlews(Liberty* library, sta::Slew slews[],
                                    int counts[]);
    void      slewLimit(InstanceTerm* pin, sta::MinMax* min_max, float& limit,
                        bool& exists) const;
    sta::ParasiticNode* findParasiticNode(std::unique_ptr<SteinerTree>& tree,
                                          sta::Parasitic*     parasitic,
                                          const Net*          net,
                                          const InstanceTerm* pin,
                                          SteinerPoint        pt);
};

} // namespace psn
#endif
#endif