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

#include <OpenPhySyn/Database/DatabaseHandler.hpp>
#include <OpenPhySyn/Database/Types.hpp>
#include <OpenPhySyn/Psn/Psn.hpp>
#include <OpenPhySyn/SteinerTree/SteinerTree.hpp>
#include <OpenPhySyn/Transform/PsnTransform.hpp>
#include <cstring>
#include <memory>
#include <unordered_set>
#include "BufferTree.hpp"

namespace psn
{
class TimingBufferTransform : public PsnTransform
{

private:
    int  buffer_count_;
    int  net_index_;
    int  buff_index_;
    void bufferPin(Psn* psn_inst, InstanceTerm* pin,
                   std::unordered_set<LibraryCell*>& buffer_lib,
                   std::unordered_set<LibraryCell*>& inverter_lib,
                   bool                              resize_gates = false);
    std::shared_ptr<BufferSolution>
         bottomUp(Psn* psn_inst, SteinerPoint pt, SteinerPoint prev,
                  std::unordered_set<LibraryCell*>& buffer_lib,
                  std::unordered_set<LibraryCell*>& inverter_lib,
                  std::shared_ptr<SteinerTree> st_tree, bool resize_gates = false);
    void topDown(Psn* psn_inst, InstanceTerm* pin,
                 std::shared_ptr<BufferTree> tree);
    void topDown(Psn* psn_inst, Net* net, std::shared_ptr<BufferTree> tree);
    std::string generateBufferName(Psn* psn_inst);
    std::string generateNetName(Psn* psn_inst);

    int timingBuffer(Psn* psn_inst, bool fix_cap = true, bool fix_slew = true,
                     std::unordered_set<std::string> buffer_lib_names =
                         std::unordered_set<std::string>(),
                     std::unordered_set<std::string> inverter_lib_names =
                         std::unordered_set<std::string>(),
                     bool resize_gates = false, bool use_inverter_pair = false);
    int fixCapacitanceViolations(Psn*                              psn_inst,
                                 std::vector<InstanceTerm*>        driver_pins,
                                 std::unordered_set<LibraryCell*>& buffer_lib,
                                 std::unordered_set<LibraryCell*>& inverter_lib,
                                 bool resize_gates);
    int fixTransitionViolations(Psn*                              psn_inst,
                                std::vector<InstanceTerm*>        driver_pins,
                                std::unordered_set<LibraryCell*>& buffer_lib,
                                std::unordered_set<LibraryCell*>& inverter_lib,
                                bool                              resize_gates);

public:
    TimingBufferTransform();

    int run(Psn* psn_inst, std::vector<std::string> args) override;
};

DEFINE_TRANSFORM(
    TimingBufferTransform, "timing_buffer", "1.0.0",
    "Performs several variations of buffering and resizing to fix timing "
    "violations",
    "Usage: transform timing_buffer buffers -all|<set of buffers> [inverters "
    "-all|<set of inverters>] [enable_gate_resize] [enable_inverter_pair]")

} // namespace psn
