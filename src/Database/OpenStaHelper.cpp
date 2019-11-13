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
#include <PhyKnight/Database/OpenStaHelper.hpp>
#include <PhyKnight/PhyLogger/PhyLogger.hpp>
#include <set>

namespace phy
{
OpenStaHelper::OpenStaHelper(Database* db) : db_(db)
{
}

std::vector<InstanceTerm*>
OpenStaHelper::inputPins(Instance* inst) const
{
    InstanceTermSet terms = inst->getITerms();
    return filterPins(terms, PinDirection::OUTPUT);
}

std::vector<InstanceTerm*>
OpenStaHelper::outputPins(Instance* inst) const
{
    InstanceTermSet terms = inst->getITerms();
    return filterPins(terms, PinDirection::INPUT);
}

std::vector<InstanceTerm*>
OpenStaHelper::fanoutPins(Net* net) const
{
    InstanceTermSet terms = net->getITerms();
    return filterPins(terms, PinDirection::INPUT);
}

InstanceTerm*
OpenStaHelper::faninPin(Net* net) const
{
    InstanceTermSet terms = net->getITerms();
    for (InstanceTermSet::iterator itr = terms.begin(); itr != terms.end();
         itr++)
    {
        InstanceTerm* inst_term = (*itr);
        Instance*     inst      = itr->getInst();
        if (inst)
        {
            if (inst_term->getIoType() == PinDirection::OUTPUT)
            {
                return inst_term;
            }
        }
    }
    return nullptr;
}

std::vector<Instance*>
OpenStaHelper::fanoutInstances(Net* net) const
{
    std::vector<Instance*>     insts;
    std::vector<InstanceTerm*> pins = fanoutPins(net);
    for (auto& term : pins)
    {
        insts.push_back(term->getInst());
    }
    return insts;
}

std::vector<Instance*>
OpenStaHelper::driverInstances() const
{
    std::set<Instance*> insts_set;
    for (auto& net : nets())
    {
        InstanceTerm* driverPin = faninPin(net);
        if (driverPin)
        {
            Instance* inst = driverPin->getInst();
            if (inst)
            {
                insts_set.insert(inst);
            }
        }
    }
    return std::vector<Instance*>(insts_set.begin(), insts_set.end());
}

unsigned int
OpenStaHelper::fanoutCount(Net* net) const
{
    return fanoutPins(net).size();
}

LibraryTerm*
OpenStaHelper::libraryPin(InstanceTerm* term) const
{
    return term->getMTerm();
}
bool
OpenStaHelper::isClocked(InstanceTerm* term) const
{
    return term->isClocked();
}
bool
OpenStaHelper::isPrimary(Net* net) const
{
    return net->getBTerms().size() > 0;
}

LibraryCell*
OpenStaHelper::libraryCell(InstanceTerm* term) const
{
    LibraryTerm* lterm = libraryPin(term);
    if (lterm)
    {
        return lterm->getMaster();
    }
    return nullptr;
}

LibraryCell*
OpenStaHelper::libraryCell(Instance* inst) const
{
    return inst->getMaster();
}

LibraryCell*
OpenStaHelper::libraryCell(const char* name) const
{
    auto lib = library();
    if (!lib)
    {
        return nullptr;
    }
    return lib->findMaster(name);
}

Instance*
OpenStaHelper::instance(const char* name) const
{
    Block* block = top();
    if (!block)
    {
        return nullptr;
    }
    return block->findInst(name);
}
Net*
OpenStaHelper::net(const char* name) const
{
    Block* block = top();
    if (!block)
    {
        return nullptr;
    }
    return block->findNet(name);
}

LibraryTerm*
OpenStaHelper::libraryPin(const char* cell_name, const char* pin_name) const
{
    LibraryCell* cell = libraryCell(cell_name);
    if (!cell)
    {
        return nullptr;
    }
    return libraryPin(cell, pin_name);
}
LibraryTerm*
OpenStaHelper::libraryPin(LibraryCell* cell, const char* pin_name) const
{
    return cell->findMTerm(top(), pin_name);
}

std::vector<InstanceTerm*>
OpenStaHelper::filterPins(InstanceTermSet& terms, PinDirection direction) const
{
    std::vector<InstanceTerm*> inst_terms;
    for (InstanceTermSet::iterator itr = terms.begin(); itr != terms.end();
         itr++)
    {
        InstanceTerm* inst_term = (*itr);
        Instance*     inst      = itr->getInst();
        if (inst)
        {
            if (inst_term && inst_term->getIoType() == direction)
            {
                inst_terms.push_back(inst_term);
            }
        }
    }
    return inst_terms;
}

void
OpenStaHelper::del(Net* net) const
{
    Net::destroy(net);
}
int
OpenStaHelper::disconnectAll(Net* net) const
{
    int             count   = 0;
    InstanceTermSet net_set = net->getITerms();
    for (InstanceTermSet::iterator itr = net_set.begin(); itr != net_set.end();
         itr++)
    {
        InstanceTerm::disconnect(*itr);
        count++;
    }
    return count;
}

void
OpenStaHelper::connect(Net* net, InstanceTerm* term) const
{
    return InstanceTerm::connect(term, net);
}

void
OpenStaHelper::disconnect(InstanceTerm* term) const
{
    InstanceTerm::disconnect(term);
}

Instance*
OpenStaHelper::createInstance(const char* inst_name, LibraryCell* cell)
{
    return Instance::create(top(), cell, inst_name);
}

Net*
OpenStaHelper::createNet(const char* net_name)
{
    return Net::create(top(), net_name);
}

InstanceTerm*
OpenStaHelper::connect(Net* net, Instance* inst, LibraryTerm* port) const
{
    return InstanceTerm::connect(inst, net, port);
}

std::vector<Net*>
OpenStaHelper::nets() const
{
    std::vector<Net*> nets;
    Block*            block = top();
    if (!block)
    {
        return std::vector<Net*>();
    }
    NetSet net_set = block->getNets();
    for (NetSet::iterator itr = net_set.begin(); itr != net_set.end(); itr++)
    {
        nets.push_back(*itr);
    }
    return nets;
}

std::string
OpenStaHelper::topName() const
{
    Block* block = top();
    if (!block)
    {
        return "";
    }
    return name(block);
}
std::string
OpenStaHelper::name(Block* object) const
{
    return std::string(object->getConstName());
}
std::string
OpenStaHelper::name(Net* object) const
{
    return std::string(object->getConstName());
}
std::string
OpenStaHelper::name(Instance* object) const
{
    return std::string(object->getConstName());
}
std::string
OpenStaHelper::name(BlockTerm* object) const
{
    return std::string(object->getConstName());
}
std::string
OpenStaHelper::name(Library* object) const
{
    return std::string(object->getConstName());
}
std::string
OpenStaHelper::name(LibraryCell* object) const
{
    return std::string(object->getConstName());
}
std::string
OpenStaHelper::name(LibraryTerm* object) const
{
    return std::string(object->getConstName());
}

Library*
OpenStaHelper::library() const
{
    LibrarySet libs = db_->getLibs();

    if (!libs.size())
    {
        return nullptr;
    }
    Library* lib = *(libs.begin());
    return lib;
}

LibraryTechnology*
OpenStaHelper::technology() const
{
    Library* lib = library();
    if (!lib)
    {
        return nullptr;
    }
    LibraryTechnology* tech = lib->getTech();
    return tech;
}

Block*
OpenStaHelper::top() const
{
    Chip* chip = db_->getChip();
    if (!chip)
    {
        return nullptr;
    }

    Block* block = chip->getBlock();
    return block;
}

void
OpenStaHelper::clear() const
{
    db_->clear();
}
} // namespace phy