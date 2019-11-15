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
#include <PhyKnight/Database/OpenStaHandler.hpp>
#include <PhyKnight/PhyLogger/PhyLogger.hpp>
#include <set>

namespace phy
{
OpenStaHandler::OpenStaHandler(Database* db) : db_(db)
{
}

std::vector<InstanceTerm*>
OpenStaHandler::inputPins(Instance* inst) const
{
    InstanceTermSet terms = inst->getITerms();
    return filterPins(terms, PinDirection::OUTPUT);
}

std::vector<InstanceTerm*>
OpenStaHandler::outputPins(Instance* inst) const
{
    InstanceTermSet terms = inst->getITerms();
    return filterPins(terms, PinDirection::INPUT);
}

std::vector<InstanceTerm*>
OpenStaHandler::fanoutPins(Net* net) const
{
    InstanceTermSet terms = net->getITerms();
    return filterPins(terms, PinDirection::INPUT);
}

InstanceTerm*
OpenStaHandler::faninPin(Net* net) const
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
OpenStaHandler::fanoutInstances(Net* net) const
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
OpenStaHandler::driverInstances() const
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
OpenStaHandler::fanoutCount(Net* net) const
{
    return fanoutPins(net).size();
}

LibraryTerm*
OpenStaHandler::libraryPin(InstanceTerm* term) const
{
    return term->getMTerm();
}
bool
OpenStaHandler::isClocked(InstanceTerm* term) const
{
    return term->isClocked();
}
bool
OpenStaHandler::isPrimary(Net* net) const
{
    return net->getBTerms().size() > 0;
}

LibraryCell*
OpenStaHandler::libraryCell(InstanceTerm* term) const
{
    LibraryTerm* lterm = libraryPin(term);
    if (lterm)
    {
        return lterm->getMaster();
    }
    return nullptr;
}

LibraryCell*
OpenStaHandler::libraryCell(Instance* inst) const
{
    return inst->getMaster();
}

LibraryCell*
OpenStaHandler::libraryCell(const char* name) const
{
    auto lib = library();
    if (!lib)
    {
        return nullptr;
    }
    return lib->findMaster(name);
}

Instance*
OpenStaHandler::instance(const char* name) const
{
    Block* block = top();
    if (!block)
    {
        return nullptr;
    }
    return block->findInst(name);
}
Net*
OpenStaHandler::net(const char* name) const
{
    Block* block = top();
    if (!block)
    {
        return nullptr;
    }
    return block->findNet(name);
}

LibraryTerm*
OpenStaHandler::libraryPin(const char* cell_name, const char* pin_name) const
{
    LibraryCell* cell = libraryCell(cell_name);
    if (!cell)
    {
        return nullptr;
    }
    return libraryPin(cell, pin_name);
}
LibraryTerm*
OpenStaHandler::libraryPin(LibraryCell* cell, const char* pin_name) const
{
    return cell->findMTerm(top(), pin_name);
}

std::vector<InstanceTerm*>
OpenStaHandler::filterPins(InstanceTermSet& terms, PinDirection direction) const
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
OpenStaHandler::del(Net* net) const
{
    Net::destroy(net);
}
int
OpenStaHandler::disconnectAll(Net* net) const
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
OpenStaHandler::connect(Net* net, InstanceTerm* term) const
{
    return InstanceTerm::connect(term, net);
}

void
OpenStaHandler::disconnect(InstanceTerm* term) const
{
    InstanceTerm::disconnect(term);
}

Instance*
OpenStaHandler::createInstance(const char* inst_name, LibraryCell* cell)
{
    return Instance::create(top(), cell, inst_name);
}

Net*
OpenStaHandler::createNet(const char* net_name)
{
    return Net::create(top(), net_name);
}

InstanceTerm*
OpenStaHandler::connect(Net* net, Instance* inst, LibraryTerm* port) const
{
    return InstanceTerm::connect(inst, net, port);
}

std::vector<Net*>
OpenStaHandler::nets() const
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
OpenStaHandler::topName() const
{
    Block* block = top();
    if (!block)
    {
        return "";
    }
    return name(block);
}
std::string
OpenStaHandler::name(Block* object) const
{
    return std::string(object->getConstName());
}
std::string
OpenStaHandler::name(Net* object) const
{
    return std::string(object->getConstName());
}
std::string
OpenStaHandler::name(Instance* object) const
{
    return std::string(object->getConstName());
}
std::string
OpenStaHandler::name(BlockTerm* object) const
{
    return std::string(object->getConstName());
}
std::string
OpenStaHandler::name(Library* object) const
{
    return std::string(object->getConstName());
}
std::string
OpenStaHandler::name(LibraryCell* object) const
{
    return std::string(object->getConstName());
}
std::string
OpenStaHandler::name(LibraryTerm* object) const
{
    return std::string(object->getConstName());
}

Library*
OpenStaHandler::library() const
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
OpenStaHandler::technology() const
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
OpenStaHandler::top() const
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
OpenStaHandler::clear() const
{
    db_->clear();
}
} // namespace phy
