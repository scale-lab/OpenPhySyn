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

#ifndef __PHY_HELPER__
#define __PHY_HELPER__

#include <PhyKnight/Database/Types.hpp>
#include <vector>

namespace phy
{
class DatabaseHelper
{
public:
    DatabaseHelper(Database* db);

    std::vector<InstanceTerm*> inputPins(Instance* inst) const;
    std::vector<InstanceTerm*> outputPins(Instance* inst) const;
    std::vector<InstanceTerm*> filterPins(InstanceTermSet& terms,
                                          PinDirection     direction) const;
    std::vector<InstanceTerm*>
                           fanoutPins(Net* net) const; // Does not include top level pins
    std::vector<Instance*> fanoutInstances(Net* net) const;
    std::vector<Instance*> driverInstances() const;
    InstanceTerm*          faninPin(Net* net) const;
    LibraryTerm*           libraryPin(InstanceTerm* term) const;
    LibraryCell*           libraryCell(InstanceTerm* term) const;
    LibraryCell*           libraryCell(Instance* inst) const;
    LibraryCell*           libraryCell(const char* name) const;
    Instance*              instance(const char* name) const;
    Net*                   net(const char* name) const;
    LibraryTerm* libraryPin(const char* cell_name, const char* pin_name) const;
    LibraryTerm* libraryPin(LibraryCell* cell, const char* pin_name) const;
    bool         isClocked(InstanceTerm* term) const;
    bool         isPrimary(Net* net) const;

    Instance* createInstance(const char* inst_name, LibraryCell* cell);
    Net*      createNet(const char* net_name);

    void          connect(Net* net, InstanceTerm* term) const;
    InstanceTerm* connect(Net* net, Instance* inst, LibraryTerm* port) const;
    void          disconnect(InstanceTerm* term) const;
    int           disconnectAll(Net* net) const;
    void          del(Net* net) const;
    void          clear() const;

    unsigned int fanoutCount(Net* net) const; // Does not include top level pins
    std::vector<Net*>  nets() const;          // Get all database net objects
    Block*             top() const;
    Library*           library() const;
    LibraryTechnology* technology() const;
    std::string        topName() const;

private:
    Database* db_;
};

} // namespace phy
#endif