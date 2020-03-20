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
#include <OpenPhySyn/Sta/PathPoint.hpp>

#include <set>

namespace psn
{
OpenDBHandler::OpenDBHandler(Psn* psn_inst, sta::DatabaseSta* sta)
    : sta_(sta), db_(sta->db())
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
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, connectedPins)
    return std::vector<InstanceTerm*>();
}
std::set<InstanceTerm*>
OpenDBHandler::clockPins() const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, clockPins)
    return std::set<InstanceTerm*>();
}
std::set<Net*>
OpenDBHandler::clockNets() const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, clockNets)
    return std::set<Net*>();
}

void
OpenDBHandler::setWireRC(float res_per_micon, float cap_per_micron)
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, clockPins)
}
bool
OpenDBHandler::hasWireRC()
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, clockPins)
    return false;
}

void
OpenDBHandler::calculateParasitics()
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, clockPins)
}

void
OpenDBHandler::calculateParasitics(Net* net){
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, clockPins)}

std::vector<InstanceTerm*> OpenDBHandler::inputPins(
    Instance* inst, bool include_top_level) const
{
    auto         inst_pins = pins(inst);
    PinDirection dir       = PinDirection::INPUT;
    return filterPins(inst_pins, &dir, include_top_level);
}
int
OpenDBHandler::evaluateFunctionExpression(
    InstanceTerm* term, std::unordered_map<LibraryTerm*, int>& inputs) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, evaluateFunctionExpression);
    return 0;
}
int
OpenDBHandler::evaluateFunctionExpression(
    LibraryTerm* term, std::unordered_map<LibraryTerm*, int>& inputs) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, evaluateFunctionExpression);
    return 0;
}

std::vector<InstanceTerm*>
OpenDBHandler::outputPins(Instance* inst, bool include_top_level) const
{
    auto         inst_pins = pins(inst);
    PinDirection dir       = PinDirection::OUTPUT;

    return filterPins(inst_pins, &dir, include_top_level);
}

std::vector<InstanceTerm*>
OpenDBHandler::fanoutPins(Net* net, bool include_top_level) const
{
    auto         inst_pins = pins(net);
    PinDirection dir       = PinDirection::INPUT;
    return filterPins(inst_pins, &dir, include_top_level);
}
std::vector<LibraryCell*>
OpenDBHandler::tiehiCells() const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, tiehiCells)
    return std::vector<LibraryCell*>();
}

std::vector<LibraryCell*>
OpenDBHandler::inverterCells() const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, inverterCells)
    return std::vector<LibraryCell*>();
}
std::vector<LibraryCell*>
OpenDBHandler::bufferCells() const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, bufferCells)
    return std::vector<LibraryCell*>();
}
LibraryCell*
OpenDBHandler::smallestInverterCell() const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, smallestInverterCell)
    return nullptr;
}
LibraryCell*
OpenDBHandler::smallestBufferCell() const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, smallestBufferCell)
    return nullptr;
}
std::vector<LibraryCell*>
OpenDBHandler::tieloCells() const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, tieloCells)
    return std::vector<LibraryCell*>();
}
std::vector<InstanceTerm*>
OpenDBHandler::levelDriverPins() const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, levelDriverPins)
    return std::vector<InstanceTerm*>();
}

InstanceTerm*
OpenDBHandler::faninPin(InstanceTerm* term) const
{
    return faninPin(net(term));
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
    std::vector<InstanceTerm*> pins = fanoutPins(net, false);
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
OpenDBHandler::fanoutCount(Net* net, bool include_top_level) const
{
    return fanoutPins(net, include_top_level).size();
}
std::vector<PathPoint>
OpenDBHandler::criticalPath(int path_count) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, criticalPath)
    return std::vector<PathPoint>();
}
std::vector<std::vector<PathPoint>>
OpenDBHandler::bestPath(int path_count) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, bestPath)
    return std::vector<std::vector<PathPoint>>();
}
std::vector<PathPoint>
OpenDBHandler::worstSlackPath(InstanceTerm* term) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, worstSlackPath)
    return std::vector<PathPoint>();
}
std::vector<PathPoint>
OpenDBHandler::worstArrivalPath(InstanceTerm* term) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, worstArrivalPath)
    return std::vector<PathPoint>();
}
std::vector<PathPoint>
OpenDBHandler::bestSlackPath(InstanceTerm* term) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, bestSlackPath)
    return std::vector<PathPoint>();
}
std::vector<PathPoint>
OpenDBHandler::bestArrivalPath(InstanceTerm* term) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, bestArrivalPath)
    return std::vector<PathPoint>();
}
float
OpenDBHandler::slack(InstanceTerm* term, bool is_rise, bool worst) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, slack)
    return 0;
}
float
OpenDBHandler::slack(InstanceTerm* term, bool worst) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, slack)
    return 0;
}
float
OpenDBHandler::arrival(InstanceTerm* term, int ap_index, bool is_rise) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, arrival)
    return 0;
}
float
OpenDBHandler::slew(InstanceTerm* term, bool is_rise) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, slew)
    return 0;
}
float
OpenDBHandler::required(InstanceTerm* term, bool worst) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, required)
    return 0;
}

bool
OpenDBHandler::isCommutative(InstanceTerm* first, InstanceTerm* second) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, isCommutative)
    return false;
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
OpenDBHandler::area(Instance* inst) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, area)
    return 0;
}
float
OpenDBHandler::area(LibraryCell* cell) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, area)
    return 0;
}
float
OpenDBHandler::area() const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, area)
    return 0;
}
float
OpenDBHandler::power(std::vector<Instance*>& insts)
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, power)
    return 0;
}
float
OpenDBHandler::power()
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, area)
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

std::vector<LibraryTerm*>
OpenDBHandler::libraryPins(Instance* inst) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, libraryPins);
    return std::vector<LibraryTerm*>();
}
std::vector<LibraryTerm*>
OpenDBHandler::libraryPins(LibraryCell* cell) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, libraryPins);
    return std::vector<LibraryTerm*>();
}
std::vector<LibraryTerm*>
OpenDBHandler::libraryInputPins(LibraryCell* cell) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, libraryInputPins);
    return std::vector<LibraryTerm*>();
}
std::vector<LibraryTerm*>
OpenDBHandler::libraryOutputPins(LibraryCell* cell) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, libraryOutputPins);
    return std::vector<LibraryTerm*>();
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
OpenDBHandler::largestLibraryCell(LibraryCell* cell)
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, largestLibraryCell)
    return nullptr;
}
double
OpenDBHandler::dbuToMeters(uint dist) const
{
    return dist * 1E-9;
}
double
OpenDBHandler::dbuToMicrons(uint dist) const
{
    return (1.0 * dist) / db_->getTech()->getLefUnits();
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
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, isDriver)
    return false;
}
bool
OpenDBHandler::isTopLevel(InstanceTerm* term) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, isTopLevel)
    return false;
}

float
OpenDBHandler::pinCapacitance(InstanceTerm* term) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, pinCapacitance)
    return 0.0;
}

float
OpenDBHandler::pinCapacitance(LibraryTerm* term) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, pinCapacitance)
    return 0.0;
}
float
OpenDBHandler::pinAverageRise(LibraryTerm* term, LibraryTerm* to) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, pinAverageRise)
    return 0.0;
}
float
OpenDBHandler::pinAverageFall(LibraryTerm* term, LibraryTerm* to) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, pinAverageFall)
    return 0.0;
}
float
OpenDBHandler::pinAverageRiseTransition(LibraryTerm* term,
                                        LibraryTerm* to) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, pinAverageRiseTransition)
    return 0.0;
}
float
OpenDBHandler::pinAverageFallTransition(LibraryTerm* term,
                                        LibraryTerm* to) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, pinAverageFallTransition)
    return 0.0;
}
float
OpenDBHandler::loadCapacitance(InstanceTerm* term) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, loadCapacitance)
    return 0.0;
}
float
OpenDBHandler::targetLoad(LibraryCell* cell)
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, targetLoad)
    return 0.0;
}

float
OpenDBHandler::gateDelay(LibraryTerm* out_port, float load_cap)
{

    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, gateDelay);
    return 0.0;
}
float
OpenDBHandler::gateDelay(Instance* inst, InstanceTerm* to, float in_slew,
                         LibraryTerm* from, float* drvr_slew, int rise_fall)
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, gateDelay);
    return 0.0;
}
float
OpenDBHandler::gateDelay(InstanceTerm* out_port, float load_cap)
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, gateDelay);
    return 0.0;
}
float
OpenDBHandler::bufferChainDelayPenalty(LibraryCell* cell, ,
                                       float        area_penalty_alpha)
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, bufferChainDelayPenalty);
    return 0.0;
}
LibraryTerm*
OpenDBHandler::bufferInputPin(LibraryCell* buffer_cell) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, bufferInputPin);
    return nullptr;
}

LibraryTerm*
OpenDBHandler::bufferOutputPin(LibraryCell* buffer_cell) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, bufferOutputPin);
    return nullptr;
}

float
OpenDBHandler::bufferDelay(psn::LibraryCell* buffer_cell, float load_cap)
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, bufferDelay);
    return 0.0;
}

float
OpenDBHandler::portCapacitance(const LibraryTerm* port, bool isMax)
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, portCapacitance);
    return 0.0;
}
float
OpenDBHandler::inverterInputCapacitance(LibraryCell* buffer_cell)
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, inverterInputCapacitance);
    return 0.0;
}
float
OpenDBHandler::bufferInputCapacitance(LibraryCell* buffer_cell)
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, bufferInputCapacitance);
    return 0.0;
}
float
OpenDBHandler::bufferOutputCapacitance(LibraryCell* buffer_cell)
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, bufferOutputCapacitance);
    return 0.0;
}

bool
OpenDBHandler::isClock(Net* net) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, isClock);
    return false;
}

std::string
OpenDBHandler::generateNetName(int& start_index)
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, generateNetName);
    return "";
}

std::string
OpenDBHandler::generateInstanceName(const std::string& prefix, int& start_index)
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, generateInstanceName);
    return "";
}

float
OpenDBHandler::maxLoad(LibraryCell* cell)
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, maxLoad)
    return 0.0;
}
float
OpenDBHandler::maxLoad(LibraryTerm* term)
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, maxLoad)
    return 0.0;
}
bool
OpenDBHandler::isInput(InstanceTerm* term) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, isInput)
    return false;
}
bool
OpenDBHandler::isOutput(InstanceTerm* term) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, isOutput)
    return false;
}
bool
OpenDBHandler::isAnyInput(InstanceTerm* term) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, isAnyInput)
    return false;
}
bool
OpenDBHandler::isAnyOutput(InstanceTerm* term) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, isAnyOutput)
    return false;
}
bool
OpenDBHandler::isBiDirect(InstanceTerm* term) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, isBiDirect)
    return false;
}
bool
OpenDBHandler::isTriState(InstanceTerm* term) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, isTriState)
    return false;
}
bool
OpenDBHandler::isSingleOutputCombinational(Instance* inst) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, isSingleOutputCombinational)
    return false;
}
bool
OpenDBHandler::isSingleOutputCombinational(LibraryCell* cell) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, isSingleOutputCombinational)
    return false;
}
bool
OpenDBHandler::isCombinational(Instance* inst) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, isCombinational)
    return false;
}
bool
OpenDBHandler::isCombinational(LibraryCell* cell) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, isCombinational)
    return false;
}
bool
OpenDBHandler::isInput(BlockTerm* term) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, isInput)
    return false;
}
bool
OpenDBHandler::isOutput(BlockTerm* term) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, isOutput)
    return false;
}
bool
OpenDBHandler::isAnyInput(BlockTerm* term) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, isAnyInput)
    return false;
}
bool
OpenDBHandler::isAnyOutput(BlockTerm* term) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, isAnyOutput)
    return false;
}
bool
OpenDBHandler::isBiDirect(BlockTerm* term) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, isBiDirect)
    return false;
}
bool
OpenDBHandler::isTriState(BlockTerm* term) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, isTriState)
    return false;
}
bool
OpenDBHandler::isInput(LibraryTerm* term) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, isInput)
    return false;
}
bool
OpenDBHandler::isOutput(LibraryTerm* term) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, isOutput)
    return false;
}
bool
OpenDBHandler::isAnyInput(LibraryTerm* term) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, isAnyInput)
    return false;
}
bool
OpenDBHandler::isAnyOutput(LibraryTerm* term) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, isAnyOutput)
    return false;
}
bool
OpenDBHandler::isBiDirect(LibraryTerm* term) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, isBiDirect)
    return false;
}
bool
OpenDBHandler::isTriState(LibraryTerm* term) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, isTriState)
    return false;
}

bool
OpenDBHandler::violatesMaximumCapacitance(InstanceTerm* term,
                                          float         load_cap) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, violatesMaximumCapacitance)
    return false;
}
bool
OpenDBHandler::violatesMaximumCapacitance(InstanceTerm* term) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, violatesMaximumCapacitance)
    return false;
}
bool
OpenDBHandler::violatesMaximumTransition(InstanceTerm* term) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, violatesMaximumTransition)
    return false;
}
bool
OpenDBHandler::isLoad(InstanceTerm* term) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, isLoad)
    return false;
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
BlockTerm*
OpenDBHandler::port(const char* name) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, port)
    return nullptr;
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
Term*
OpenDBHandler::term(InstanceTerm* term) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, term)
    return nullptr;
}
Net*
OpenDBHandler::net(Term* term) const
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, net)
    return nullptr;
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
                          PinDirection* direction, bool include_top_level) const
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
        else if (include_top_level)
        {
            // TODO
        }
    }
    return inst_terms;
}

void
OpenDBHandler::del(Net* net) const
{
    Net::destroy(net);
}
void
OpenDBHandler::del(Instance* inst) const
{
    Instance::destroy(inst);
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

void
OpenDBHandler::swapPins(InstanceTerm* first, InstanceTerm* second)
{
    auto first_net  = net(first);
    auto second_net = net(second);
    disconnect(first);
    disconnect(second);
    connect(first_net, second);
    connect(second_net, first);
}
Instance*
OpenDBHandler::createInstance(const char* inst_name, LibraryCell* cell)
{
    return Instance::create(top(), cell, inst_name);
}
void
OpenDBHandler::createClock(const char*             clock_name,
                           std::vector<BlockTerm*> ports, float period)
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, createClock);
}
void
OpenDBHandler::createClock(const char*              clock_name,
                           std::vector<std::string> ports, float period)
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, createClock);
}

Net*
OpenDBHandler::createNet(const char* net_name)
{
    return Net::create(top(), net_name);
}

void
OpenDBHandler::connect(Net* net, Instance* inst, LibraryTerm* port) const
{
    InstanceTerm::connect(inst, net, port);
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
std::vector<Instance*>
OpenDBHandler::instances() const
{
    std::vector<Instance*> insts;
    Block*                 block = top();
    if (!block)
    {
        return std::vector<Instance*>();
    }
    auto inst_set = block->getInsts();
    for (auto itr = inst_set.begin(); itr != inst_set.end(); itr++)
    {
        insts.push_back(*itr);
    }
    return insts;
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
OpenDBHandler::name(InstanceTerm* object) const
{
    return std::string(object->getObjName());
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
bool
OpenDBHandler::dontUse(LibraryCell* cell) const
{
    return false;
}
bool
OpenDBHandler::dontTouch(Instance* cell) const
{
    return false;
}
void
OpenDBHandler::resetDelays()
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, resetDelays);
}
void
OpenDBHandler::resetDelays(InstanceTerm* term)
{
    PSN_HANDLER_UNSUPPORTED_METHOD(OpenDBHandler, resetDelays);
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
OpenDBHandler::resetCache()
{
}

void
OpenDBHandler::clear()
{
    db_->clear();
}
OpenDBHandler::~OpenDBHandler()
{
}
HandlerType
OpenDBHandler::handlerType() const
{
    return HandlerType::OPENDB;
}
} // namespace psn
#endif