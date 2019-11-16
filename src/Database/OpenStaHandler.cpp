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

// Temproary fix for OpenSTA
#define THROW_DCL throw()

#include <OpenSTA/graph/Graph.hh>
#include <OpenSTA/network/PortDirection.hh>
#include <OpenSTA/search/Search.hh>
#include <OpenSTA/util/PatternMatch.hh>
#include <PhyKnight/Database/OpenStaHandler.hpp>
#include <PhyKnight/PhyLogger/PhyLogger.hpp>
#include <PhyKnight/Sta/DatabaseSta.hpp>
#include <PhyKnight/Sta/DatabaseStaNetwork.hpp>

#include <set>

namespace phy
{
OpenStaHandler::OpenStaHandler(sta::DatabaseSta* sta)
    : sta_(sta), db_(sta->db())
{
}

std::vector<InstanceTerm*>
OpenStaHandler::pins(Net* net) const
{
    std::vector<InstanceTerm*> terms;
    auto                       pin_iter = network()->pinIterator(net);
    while (pin_iter->hasNext())
    {
        InstanceTerm* pin = pin_iter->next();
        terms.push_back(pin);
    }
    return terms;
}
std::vector<InstanceTerm*>
OpenStaHandler::pins(Instance* inst) const
{
    std::vector<InstanceTerm*> terms;
    auto                       pin_iter = network()->pinIterator(inst);
    while (pin_iter->hasNext())
    {
        InstanceTerm* pin = pin_iter->next();
        terms.push_back(pin);
    }
    return terms;
}
std::vector<InstanceTerm*>
OpenStaHandler::connectedPins(Net* net) const
{
    std::vector<InstanceTerm*> terms;
    auto                       pin_iter = network()->connectedPinIterator(net);
    while (pin_iter->hasNext())
    {
        InstanceTerm* pin = pin_iter->next();
        terms.push_back(pin);
    }
    return terms;
}

std::vector<InstanceTerm*>
OpenStaHandler::inputPins(Instance* inst) const
{
    auto inst_pins = pins(inst);
    return filterPins(inst_pins, PinDirection::output());
}

std::vector<InstanceTerm*>
OpenStaHandler::outputPins(Instance* inst) const
{
    auto inst_pins = pins(inst);
    return filterPins(inst_pins, PinDirection::input());
}

std::vector<InstanceTerm*>
OpenStaHandler::fanoutPins(Net* net) const
{
    auto inst_pins = pins(net);
    return filterPins(inst_pins, PinDirection::input());
}

InstanceTerm*
OpenStaHandler::faninPin(Net* net) const
{
    auto net_pins = pins(net);
    for (auto& pin : net_pins)
    {
        Instance* inst = network()->instance(pin);
        if (inst)
        {
            if (network()->direction(pin)->isOutput())
            {
                return pin;
            }
        }
    }

    return nullptr;
}

std::vector<Instance*>
OpenStaHandler::fanoutInstances(Net* net) const
{
    std::vector<Instance*>     insts;
    std::vector<InstanceTerm*> net_pins = fanoutPins(net);
    for (auto& term : net_pins)
    {
        insts.push_back(network()->instance(term));
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
            Instance* inst = network()->instance(driverPin);
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

Point
OpenStaHandler::location(InstanceTerm* term)
{
    odb::dbITerm* iterm;
    odb::dbBTerm* bterm;
    network()->staToDb(term, iterm, bterm);
    if (iterm)
    {
        int x, y;
        iterm->getInst()->getOrigin(x, y);
        return Point(x, y);
    }
    if (bterm)
    {
        int x, y;
        if (bterm->getFirstPinLocation(x, y))
            return Point(x, y);
    }
    return Point(0, 0);
}

Point
OpenStaHandler::location(Instance* inst)
{
    odb::dbInst* dinst = network()->staToDb(inst);
    int          x, y;
    dinst->getOrigin(x, y);
    return Point(x, y);
}
void
OpenStaHandler::setLocation(Instance* inst, Point pt)
{
    odb::dbInst* dinst = network()->staToDb(inst);
    dinst->setPlacementStatus(odb::dbPlacementStatus::PLACED);
    dinst->setLocation(pt.getX(), pt.getY());
}

LibraryTerm*
OpenStaHandler::libraryPin(InstanceTerm* term) const
{
    return network()->libertyPort(term);
}
bool
OpenStaHandler::isClocked(InstanceTerm* term) const
{
    sta::Vertex *vertex, *bidirect_drvr_vertex;
    sta_->graph()->pinVertices(term, vertex, bidirect_drvr_vertex);
    return sta_->search()->isClock(vertex);
}
bool
OpenStaHandler::isPrimary(Net* net) const
{
    for (auto& pin : pins(net))
    {
        if (network()->isTopLevelPort(pin))
        {
            return true;
        }
    }
    return false;
}

LibraryCell*
OpenStaHandler::libraryCell(InstanceTerm* term) const
{
    auto inst = network()->instance(term);
    if (inst)
    {
        return network()->libertyCell(inst);
    }
    return nullptr;
}

LibraryCell*
OpenStaHandler::libraryCell(Instance* inst) const
{
    return network()->libertyCell(inst);
}

LibraryCell*
OpenStaHandler::libraryCell(const char* name) const
{
    return network()->findLibertyCell(name);
}

Instance*
OpenStaHandler::instance(const char* name) const
{
    return network()->findInstance(name);
}
Net*
OpenStaHandler::net(const char* name) const
{
    return network()->findNet(name);
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
    return cell->findLibertyPort(pin_name);
}

std::vector<InstanceTerm*>
OpenStaHandler::filterPins(std::vector<InstanceTerm*>& terms,
                           PinDirection*               direction) const
{
    std::vector<InstanceTerm*> inst_terms;
    for (auto& term : terms)
    {
        Instance* inst = network()->instance(term);
        if (inst)
        {
            if (term && network()->direction(term) == direction)
            {
                inst_terms.push_back(term);
            }
        }
    }
    return inst_terms;
}
void
OpenStaHandler::del(Net* net) const
{
    network()->deleteNet(net);
}
int
OpenStaHandler::disconnectAll(Net* net) const
{
    int count = 0;
    for (auto& pin : pins(net))
    {
        network()->disconnectPin(pin);
        count++;
    }
    return count;
}

void
OpenStaHandler::connect(Net* net, InstanceTerm* term) const
{
    network()->connect(network()->instance(term), network()->port(term), net);
}

void
OpenStaHandler::disconnect(InstanceTerm* term) const
{
    network()->disconnectPin(term);
}

Instance*
OpenStaHandler::createInstance(const char* inst_name, LibraryCell* cell)
{
    return network()->makeInstance(cell, inst_name, network()->topInstance());
}

Net*
OpenStaHandler::createNet(const char* net_name)
{
    return network()->makeNet(net_name, network()->topInstance());
}

InstanceTerm*
OpenStaHandler::connect(Net* net, Instance* inst, LibraryTerm* port) const
{
    return network()->connect(inst, port, net);
}

std::vector<Net*>
OpenStaHandler::nets() const
{
    sta::NetSeq       all_nets;
    sta::PatternMatch pattern("*");
    network()->findNetsMatching(network()->topInstance(), &pattern, &all_nets);
    return static_cast<std::vector<Net*>>(all_nets);
}

std::string
OpenStaHandler::topName() const
{
    return std::string(network()->name(network()->topInstance()));
}
std::string
OpenStaHandler::name(Block* object) const
{
    return std::string(object->getConstName());
}
std::string
OpenStaHandler::name(Net* object) const
{
    return std::string(network()->name(object));
}
std::string
OpenStaHandler::name(Instance* object) const
{
    return std::string(network()->name(object));
}
std::string
OpenStaHandler::name(BlockTerm* object) const
{
    return std::string(network()->name(object));
}
std::string
OpenStaHandler::name(Library* object) const
{
    return std::string(object->getConstName());
}
std::string
OpenStaHandler::name(LibraryCell* object) const
{
    return std::string(network()->name(network()->cell(object)));
}
std::string
OpenStaHandler::name(LibraryTerm* object) const
{
    return object->name();
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

OpenStaHandler::~OpenStaHandler()
{
}
void
OpenStaHandler::clear() const
{
    db_->clear();
}
sta::DatabaseStaNetwork*
OpenStaHandler::network() const
{
    return sta_->getDbNetwork();
}
sta::DatabaseSta*
OpenStaHandler::sta() const
{
    return sta_;
}
} // namespace phy
#endif