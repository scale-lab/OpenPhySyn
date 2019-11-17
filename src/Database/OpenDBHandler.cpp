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

#ifdef USE_OPENDB_DB_HANDLER

#include <OpenPhySyn/Database/OpenDBHandler.hpp>
#include <OpenPhySyn/PsnLogger/PsnLogger.hpp>
#include <set>

namespace psn
{
OpenDBHandler::OpenDBHandler(sta::DatabaseSta* sta) : sta_(sta), db_(sta->db())
{
}

std::vector<InstanceTerm*>
OpenDBHandler::pins(Net* net) const
{
    std::vector<InstanceTerm*> terms;
    InstanceTermSet            term_set = net->getITerms();
    for (InstanceTermSet::iterator itr = term_set.begin();
         itr != term_set.end(); itr++)
    {
        InstanceTerm* inst_term = (*itr);
        terms.push_back(inst_term);
    }
    return terms;
}
std::vector<InstanceTerm*>
OpenDBHandler::pins(Instance* inst) const
{
    std::vector<InstanceTerm*> terms;
    InstanceTermSet            term_set = inst->getITerms();
    for (InstanceTermSet::iterator itr = term_set.begin();
         itr != term_set.end(); itr++)
    {
        InstanceTerm* inst_term = (*itr);
        terms.push_back(inst_term);
    }
    return terms;
}
Net*
OpenDBHandler::net(InstanceTerm* term) const
{
    return term->getNet();
}
std::vector<InstanceTerm*>
OpenDBHandler::connectedPins(Net* net) const
{
    HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, connectedPins)
    return std::vector<InstanceTerm*>();
}

std::vector<InstanceTerm*>
OpenDBHandler::inputPins(Instance* inst) const
{
    auto         inst_pins = pins(inst);
    PinDirection dir       = PinDirection::OUTPUT;
    return filterPins(inst_pins, &dir);
}

std::vector<InstanceTerm*>
OpenDBHandler::outputPins(Instance* inst) const
{
    auto         inst_pins = pins(inst);
    PinDirection dir       = PinDirection::INPUT;

    return filterPins(inst_pins, &dir);
}

std::vector<InstanceTerm*>
OpenDBHandler::fanoutPins(Net* net) const
{
    auto         inst_pins = pins(net);
    PinDirection dir       = PinDirection::INPUT;
    return filterPins(inst_pins, &dir);
}
std::vector<InstanceTerm*>
OpenDBHandler::levelDriverPins() const
{
    HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, levelDriverPins)
    return std::vector<InstanceTerm*>();
}

InstanceTerm*
OpenDBHandler::faninPin(Net* net) const
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
OpenDBHandler::fanoutInstances(Net* net) const
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
OpenDBHandler::driverInstances() const
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
OpenDBHandler::fanoutCount(Net* net) const
{
    return fanoutPins(net).size();
}

Point
OpenDBHandler::location(InstanceTerm* term)
{
    int x, y;
    term->getInst()->getOrigin(x, y);
    return Point(x, y);
}
Point
OpenDBHandler::location(BlockTerm* term)
{
    int x, y;
    if (term->getFirstPinLocation(x, y))
        return Point(x, y);
    return Point(0, 0);
}

Point
OpenDBHandler::location(Instance* inst)
{
    int x, y;
    inst->getOrigin(x, y);
    return Point(x, y);
}
float
OpenDBHandler::area(Instance* inst)
{
    HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, area)
    return 0;
}
float
OpenDBHandler::area()
{
    HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, area)
    return 0;
}
void
OpenDBHandler::setLocation(Instance* inst, Point pt)
{
    inst->setPlacementStatus(odb::dbPlacementStatus::PLACED);
    inst->setLocation(pt.getX(), pt.getY());
}
LibraryTerm*
OpenDBHandler::libraryPin(InstanceTerm* term) const
{
    return term->getMTerm();
}
bool
OpenDBHandler::isClocked(InstanceTerm* term) const
{
    return term->isClocked();
}
bool
OpenDBHandler::isPrimary(Net* net) const
{
    return net->getBTerms().size() > 0;
}

LibraryCell*
OpenDBHandler::libraryCell(InstanceTerm* term) const
{
    LibraryTerm* lterm = libraryPin(term);
    if (lterm)
    {
        return lterm->getMaster();
    }
    return nullptr;
}

LibraryCell*
OpenDBHandler::libraryCell(Instance* inst) const
{
    return inst->getMaster();
}

LibraryCell*
OpenDBHandler::libraryCell(const char* name) const
{
    auto lib = library();
    if (!lib)
    {
        return nullptr;
    }
    return lib->findMaster(name);
}
LibraryCell*
OpenDBHandler::largestLibraryCell(LibraryCell* cell) const
{
    HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, largestLibraryCell)
    return nullptr;
}
double
OpenDBHandler::dbuToMeters(uint dist) const
{
    return dist * 1E-9;
}
bool
OpenDBHandler::isPlaced(InstanceTerm* term) const
{

    odb::dbPlacementStatus status = term->getInst()->getPlacementStatus();
    return status == odb::dbPlacementStatus::PLACED ||
           status == odb::dbPlacementStatus::LOCKED ||
           status == odb::dbPlacementStatus::FIRM ||
           status == odb::dbPlacementStatus::COVER;
}
bool
OpenDBHandler::isPlaced(BlockTerm* term) const
{

    odb::dbPlacementStatus status = term->getFirstPinPlacementStatus();
    return status == odb::dbPlacementStatus::PLACED ||
           status == odb::dbPlacementStatus::LOCKED ||
           status == odb::dbPlacementStatus::FIRM ||
           status == odb::dbPlacementStatus::COVER;
}
bool
OpenDBHandler::isPlaced(Instance* inst) const
{
    odb::dbPlacementStatus status = inst->getPlacementStatus();

    return status == odb::dbPlacementStatus::PLACED ||
           status == odb::dbPlacementStatus::LOCKED ||
           status == odb::dbPlacementStatus::FIRM ||
           status == odb::dbPlacementStatus::COVER;
}
bool
OpenDBHandler::isDriver(InstanceTerm* term) const
{
    HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, isDriver)
    return false;
}

float
OpenDBHandler::pinCapacitance(InstanceTerm* term) const
{
    HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, pinCapacitance)
    return 0.0;
}

float
OpenDBHandler::pinCapacitance(LibraryTerm* term) const
{
    HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, pinCapacitance)
    return 0.0;
}

Instance*
OpenDBHandler::instance(const char* name) const
{
    Block* block = top();
    if (!block)
    {
        return nullptr;
    }
    return block->findInst(name);
}
Instance*
OpenDBHandler::instance(InstanceTerm* term) const
{
    return term->getInst();
}
Net*
OpenDBHandler::net(const char* name) const
{
    Block* block = top();
    if (!block)
    {
        return nullptr;
    }
    return block->findNet(name);
}

LibraryTerm*
OpenDBHandler::libraryPin(const char* cell_name, const char* pin_name) const
{
    LibraryCell* cell = libraryCell(cell_name);
    if (!cell)
    {
        return nullptr;
    }
    return libraryPin(cell, pin_name);
}
LibraryTerm*
OpenDBHandler::libraryPin(LibraryCell* cell, const char* pin_name) const
{
    return cell->findMTerm(top(), pin_name);
}

std::vector<InstanceTerm*>
OpenDBHandler::filterPins(std::vector<InstanceTerm*>& terms,
                          PinDirection*               direction) const
{
    std::vector<InstanceTerm*> inst_terms;
    for (auto& term : terms)
    {
        Instance* inst = term->getInst();
        if (inst)
        {
            if (term && term->getIoType() == *direction)
            {
                inst_terms.push_back(term);
            }
        }
    }
    return inst_terms;
}

void
OpenDBHandler::del(Net* net) const
{
    Net::destroy(net);
}
int
OpenDBHandler::disconnectAll(Net* net) const
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
OpenDBHandler::connect(Net* net, InstanceTerm* term) const
{
    return InstanceTerm::connect(term, net);
}

void
OpenDBHandler::disconnect(InstanceTerm* term) const
{
    InstanceTerm::disconnect(term);
}

Instance*
OpenDBHandler::createInstance(const char* inst_name, LibraryCell* cell)
{
    return Instance::create(top(), cell, inst_name);
}

Net*
OpenDBHandler::createNet(const char* net_name)
{
    return Net::create(top(), net_name);
}

InstanceTerm*
OpenDBHandler::connect(Net* net, Instance* inst, LibraryTerm* port) const
{
    return InstanceTerm::connect(inst, net, port);
}

std::vector<Net*>
OpenDBHandler::nets() const
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
OpenDBHandler::topName() const
{
    Block* block = top();
    if (!block)
    {
        return "";
    }
    return name(block);
}
std::string
OpenDBHandler::name(Block* object) const
{
    return std::string(object->getConstName());
}
std::string
OpenDBHandler::name(Net* object) const
{
    return std::string(object->getConstName());
}
std::string
OpenDBHandler::name(Instance* object) const
{
    return std::string(object->getConstName());
}
std::string
OpenDBHandler::name(BlockTerm* object) const
{
    return std::string(object->getConstName());
}
std::string
OpenDBHandler::name(Library* object) const
{
    return std::string(object->getConstName());
}
std::string
OpenDBHandler::name(LibraryCell* object) const
{
    return std::string(object->getConstName());
}
std::string
OpenDBHandler::name(LibraryTerm* object) const
{
    return std::string(object->getConstName());
}

Library*
OpenDBHandler::library() const
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
OpenDBHandler::technology() const
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
OpenDBHandler::top() const
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
OpenDBHandler::clear() const
{
    db_->clear();
}
OpenDBHandler::~OpenDBHandler()
{
}
} // namespace psn
#endif