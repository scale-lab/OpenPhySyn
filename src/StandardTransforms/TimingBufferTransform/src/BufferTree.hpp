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

#include "OpenPhySyn/Database/DatabaseHandler.hpp"
#include "OpenPhySyn/Database/Types.hpp"
#include "OpenPhySyn/Psn/Psn.hpp"
#include "OpenPhySyn/SteinerTree/SteinerTree.hpp"
#include "OpenPhySyn/Utils/PsnGlobal.hpp"
#include "PsnLogger/PsnLogger.hpp"

#include <memory>

namespace psn
{

enum BufferMode
{
    TimingDriven,
    Timerless
};
class BufferTree
{
    float capacitance_;
    float required_or_slew_; // required time for timing-driven and slew for
                             // timerless
    float                       wire_capacitance_;
    float                       wire_delay_or_slew_;
    float                       cost_;
    Point                       location_;
    std::shared_ptr<BufferTree> left_, right_;
    LibraryCell*                buffer_cell_;
    InstanceTerm*               pin_;
    LibraryTerm*                library_pin_;
    LibraryCell*                upstream_buffer_cell_;
    LibraryCell*                driver_cell_;
    int                         polarity_;
    int                         buffer_count_;
    BufferMode                  mode_;

public:
    BufferTree(float cap = 0.0, float req = 0.0, float cost = 0.0,
               Point location = Point(0, 0), LibraryTerm* library_pin = nullptr,
               InstanceTerm* pin = nullptr, LibraryCell* buffer_cell = nullptr,
               int        polarity    = 0,
               BufferMode buffer_mode = BufferMode::TimingDriven)
        : capacitance_(cap),
          required_or_slew_(req),
          wire_capacitance_(0.0),
          wire_delay_or_slew_(0.0),
          cost_(cost),
          location_(location),
          left_(nullptr),
          right_(nullptr),
          buffer_cell_(buffer_cell),
          pin_(pin),
          library_pin_(library_pin),
          upstream_buffer_cell_(buffer_cell),
          driver_cell_(nullptr),
          polarity_(polarity),
          buffer_count_(0),
          mode_(buffer_mode)

    {
    }
    BufferTree(Psn* psn_inst, std::shared_ptr<BufferTree> left,
               std::shared_ptr<BufferTree> right, Point location)
        : capacitance_(left->totalCapacitance() + right->totalCapacitance()),
          required_or_slew_(left->mode() == Timerless
                                ? (std::max(left->totalRequiredOrSlew(),
                                            right->totalRequiredOrSlew()))
                                : std::min(left->totalRequiredOrSlew(),
                                           right->totalRequiredOrSlew())),
          wire_capacitance_(0.0),
          wire_delay_or_slew_(0.0),
          cost_(left->cost() + right->cost()),
          location_(location),
          left_(left),
          right_(right),
          buffer_cell_(nullptr),
          pin_(),
          library_pin_(left->libraryPin()),
          upstream_buffer_cell_(nullptr),
          driver_cell_(nullptr),
          polarity_(0),
          buffer_count_(left->bufferCount() + right->bufferCount()),
          mode_(left->mode())

    {
        required_or_slew_ =
            (isTimerless() ? (std::max(left->totalRequiredOrSlew(),
                                       right->totalRequiredOrSlew()))
                           : std::min(left->totalRequiredOrSlew(),
                                      right->totalRequiredOrSlew()));
        if (left->hasUpstreamBufferCell())
        {
            if (right->hasUpstreamBufferCell())
            {
                auto left_cell  = left->upstreamBufferCell();
                auto right_cell = right->upstreamBufferCell();
                if (bufferRequired(psn_inst, left_cell) <=
                    bufferRequired(psn_inst, right_cell))
                {
                    upstream_buffer_cell_ = left->upstreamBufferCell();
                }
                else
                {
                    upstream_buffer_cell_ = right->upstreamBufferCell();
                }
            }
            else
            {
                upstream_buffer_cell_ = left->upstreamBufferCell();
            }
        }
        else if (right->hasUpstreamBufferCell())
        {
            upstream_buffer_cell_ = right->upstreamBufferCell();
        }
    }
    float
    totalCapacitance() const
    {
        return capacitance() + wireCapacitance();
    }
    float
    capacitance() const
    {
        return capacitance_;
    }
    float
    requiredOrSlew() const
    {
        return required_or_slew_;
    }
    float
    totalRequiredOrSlew() const
    {
        return isTimerless() ? required_or_slew_ + wire_delay_or_slew_
                             : required_or_slew_ - wire_delay_or_slew_;
    }
    float
    wireCapacitance() const
    {
        return wire_capacitance_;
    }
    float
    wireDelayOrSlew() const
    {
        return wire_delay_or_slew_;
    }
    float
    cost() const
    {
        return cost_;
    }
    int
    polarity() const
    {
        return polarity_;
    }
    InstanceTerm*
    pin() const
    {
        return pin_;
    }
    void
    setCapacitance(float cap)
    {
        capacitance_ = cap;
    }
    void
    setRequiredOrSlew(float req)
    {
        required_or_slew_ = req;
    }
    void
    setWireCapacitance(float cap)
    {
        wire_capacitance_ = cap;
    }
    void
    setWireDelayOrSlew(float delay)
    {
        wire_delay_or_slew_ = delay;
    }
    void
    setCost(float cost)
    {
        cost_ = cost;
    }
    void
    setPolarity(int polarity)
    {
        polarity_ = polarity;
    }
    void
    setPin(InstanceTerm* pin)
    {
        pin_ = pin;
    }
    void
    setLibraryPin(LibraryTerm* library_pin)
    {
        library_pin_ = library_pin;
    }

    void
    setMode(BufferMode buffer_mode)
    {
        mode_ = buffer_mode;
    }

    BufferMode
    mode() const
    {
        return mode_;
    }

    float
    bufferSlew(Psn* psn_inst, LibraryCell* buffer_cell, float tr_slew = 0.0)
    {
        // return psn_inst->handler()->slew(
        //     psn_inst->handler()->bufferOutputPin(buffer_cell),
        //     totalCapacitance(), &tr_slew);
        return psn_inst->handler()->bufferFixedInputSlew(buffer_cell,
                                                         totalCapacitance());
    }

    float
    bufferRequired(Psn* psn_inst, LibraryCell* buffer_cell) const
    {
        return totalRequiredOrSlew() - psn_inst->handler()->bufferDelay(
                                           buffer_cell, totalCapacitance());
    }
    float
    bufferRequired(Psn* psn_inst) const
    {
        return bufferRequired(psn_inst, buffer_cell_);
    }
    float
    upstreamBufferRequired(Psn* psn_inst) const
    {
        return bufferRequired(psn_inst, upstream_buffer_cell_);
    }

    bool
    hasDownstreamSlewViolation(Psn* psn_inst, float slew_limit,
                               float tr_slew = 0.0)
    {
        if (isBufferNode())
        {
            if (tr_slew)
            {
                tr_slew = bufferSlew(psn_inst, buffer_cell_, tr_slew);
            }
            else
            {
                tr_slew = bufferSlew(psn_inst, buffer_cell_);
            }
            if (tr_slew > slew_limit)
            {
                return true;
            }
        }
        tr_slew += totalRequiredOrSlew();
        if (left_ &&
            left_->hasDownstreamSlewViolation(psn_inst, slew_limit, tr_slew))
        {
            return true;
        }
        if (right_ &&
            right_->hasDownstreamSlewViolation(psn_inst, slew_limit, tr_slew))
        {
            return true;
        }

        return false;
    }

    LibraryTerm*
    libraryPin() const
    {
        return library_pin_;
    }
    std::shared_ptr<BufferTree>&
    left()
    {
        return left_;
    }
    std::shared_ptr<BufferTree>&
    right()
    {
        return right_;
    }
    void
    setLeft(std::shared_ptr<BufferTree> left)
    {
        left_ = left;
    }
    void
    setRight(std::shared_ptr<BufferTree> right)
    {
        right_ = right;
    }
    bool
    hasUpstreamBufferCell() const
    {
        return upstream_buffer_cell_ != nullptr;
    }
    bool
    hasBufferCell() const
    {
        return buffer_cell_ != nullptr;
    }
    bool
    hasDriverCell() const
    {
        return driver_cell_ != nullptr;
    }

    LibraryCell*
    bufferCell() const
    {
        return buffer_cell_;
    }
    LibraryCell*
    upstreamBufferCell() const
    {
        return upstream_buffer_cell_;
    }
    LibraryCell*
    driverCell() const
    {
        return driver_cell_;
    }
    void
    setBufferCell(LibraryCell* buffer_cell)
    {
        buffer_cell_          = buffer_cell;
        upstream_buffer_cell_ = buffer_cell;
    }
    void
    setUpstreamBufferCell(LibraryCell* buffer_cell)
    {
        upstream_buffer_cell_ = buffer_cell;
    }
    void
    setDriverCell(LibraryCell* driver_cell)
    {
        driver_cell_ = driver_cell;
    }

    Point
    location() const
    {
        return location_;
    }

    bool
    isBufferNode() const
    {
        return buffer_cell_ != nullptr;
    }
    bool
    isLoadNode() const
    {
        return !isBranched() && !isBufferNode();
    }
    bool
    isBranched() const
    {
        return left_ != nullptr && right_ != nullptr;
    }
    bool
    operator<(const BufferTree& other) const
    {
        return (required_or_slew_ - wire_delay_or_slew_) <
               (other.required_or_slew_ - other.wire_delay_or_slew_);
    }
    bool
    operator>(const BufferTree& other) const
    {
        return (required_or_slew_ - wire_delay_or_slew_) >
               (other.required_or_slew_ - other.wire_delay_or_slew_);
    }
    bool
    operator<=(const BufferTree& other) const
    {
        return (required_or_slew_ - wire_delay_or_slew_) <=
               (other.required_or_slew_ - other.wire_delay_or_slew_);
    }
    bool
    operator>=(const BufferTree& other) const
    {
        return (required_or_slew_ - wire_delay_or_slew_) >=
               (other.required_or_slew_ - other.wire_delay_or_slew_);
    }
    void
    setBufferCount(int buffer_count)
    {
        buffer_count_ = buffer_count;
    }
    int
    bufferCount() const
    {
        return buffer_count_;
    }
    int
    count()
    {
        buffer_count_ = (buffer_cell_ != nullptr) +
                        (left_ ? left_->count() : 0) +
                        (right_ ? right_->count() : 0);
        return bufferCount();
    }
    int
    branchCount() const
    {
        return (left_ ? (1 + left_->branchCount()) : 0) +
               (right_ ? (1 + right_->branchCount()) : 0);
    }
    void
    logInfo() const
    {
        PSN_LOG_INFO(
            "Buffers {} Cap. {} (+W {}) Req./Slew {} (+W {}) Cost {} Left? "
            "{} Right? {} Branches {}",
            bufferCount(), capacitance(), totalCapacitance(), requiredOrSlew(),
            totalRequiredOrSlew(), cost(), left_ != nullptr, right_ != nullptr,
            branchCount());
    }
    void
    logDebug() const
    {
        PSN_LOG_DEBUG(
            "Buffers {} Cap. {} (+W {}) Req./Slew {} (+W {}) Cost {} Left? "
            "{} Right? {} Branches {}",
            bufferCount(), capacitance(), totalCapacitance(), requiredOrSlew(),
            totalRequiredOrSlew(), cost(), left_ != nullptr, right_ != nullptr,
            branchCount());
    }
    bool
    isTimerless() const
    {
        return mode_ == BufferMode::Timerless;
    }
};

class TimerlessBufferTree : public BufferTree
{
public:
    TimerlessBufferTree(float cap = 0.0, float req = 0.0, float cost = 0.0,
                        Point         location    = Point(0, 0),
                        LibraryTerm*  library_pin = nullptr,
                        InstanceTerm* pin         = nullptr,
                        LibraryCell* buffer_cell = nullptr, int polarity = 0)
        : BufferTree(cap, req, cost, location, library_pin, pin, buffer_cell,
                     polarity = 0, BufferMode::Timerless)
    {
    }
    TimerlessBufferTree(Psn* psn_inst, std::shared_ptr<BufferTree> left,
                        std::shared_ptr<BufferTree> right, Point location)
        : BufferTree(psn_inst, left, right, location)
    {
        setMode(BufferMode::Timerless);
    }
};

class BufferSolution
{
    std::vector<std::shared_ptr<BufferTree>> buffer_trees_;
    BufferMode                               mode_;

public:
    BufferSolution(BufferMode buffer_mode = BufferMode::TimingDriven)
        : mode_(buffer_mode){};
    BufferSolution(Psn* psn_inst, std::shared_ptr<BufferSolution>& left,
                   std::shared_ptr<BufferSolution>& right, Point location,
                   LibraryCell* upstream_res_cell,
                   float        minimum_upstream_res_or_max_slew,
                   BufferMode   buffer_mode = BufferMode::TimingDriven)
        : mode_(buffer_mode)

    {
        mergeBranches(psn_inst, left, right, location, upstream_res_cell,
                      minimum_upstream_res_or_max_slew);
    }
    void
    mergeBranches(Psn* psn_inst, std::shared_ptr<BufferSolution>& left,
                  std::shared_ptr<BufferSolution>& right, Point location,
                  LibraryCell* upstream_res_cell,
                  float        minimum_upstream_res_or_max_slew)
    {
        buffer_trees_.resize(left->bufferTrees().size() *
                                 right->bufferTrees().size(),
                             std::make_shared<BufferTree>());
        int index = 0;
        for (auto& left_branch : left->bufferTrees())
        {
            for (auto& right_branch : right->bufferTrees())
            {
                if (left_branch->polarity() == right_branch->polarity())
                {
                    buffer_trees_[index++] =
                        isTimerless()
                            ? std::make_shared<TimerlessBufferTree>(
                                  psn_inst, left_branch, right_branch, location)
                            : std::make_shared<BufferTree>(
                                  psn_inst, left_branch, right_branch,
                                  location);
                }
            }
        }
        buffer_trees_.resize(index);
        prune(psn_inst, upstream_res_cell, minimum_upstream_res_or_max_slew);
    }
    void
    addTree(std::shared_ptr<BufferTree>& tree)
    {
        buffer_trees_.push_back(tree);
    }
    std::vector<std::shared_ptr<BufferTree>>&
    bufferTrees()
    {
        return buffer_trees_;
    }
    void
    addWireDelayAndCapacitance(float wire_res, float wire_cap)
    {
        float wire_delay = wire_res * wire_cap;
        for (auto& tree : buffer_trees_)
        {
            tree->setWireDelayOrSlew(wire_delay);
            tree->setWireCapacitance(wire_cap);
        }
    }
    void
    addWireSlewAndCapacitance(float wire_res, float wire_cap)
    {
        for (auto& tree : buffer_trees_)
        {
            float slew_degrade = // std::log(9)
                wire_res * ((wire_cap / 2.0) + tree->totalCapacitance());
            float wire_slew = std::log(9) * slew_degrade;
            tree->setWireDelayOrSlew(wire_slew);
            tree->setWireCapacitance(wire_cap);
        }
    }

    void
    addLeafTrees(Psn* psn_inst, InstanceTerm*, Point pt,
                 std::vector<LibraryCell*>& buffer_lib,
                 std::vector<LibraryCell*>& inverter_lib,
                 float                      slew_limit = 0.0)
    {
        if (!buffer_trees_.size())
        {
            return;
        }
        if (!isTimerless())
        {

            for (auto& buff : buffer_lib)
            {
                auto optimal_tree = *(buffer_trees_.begin());
                auto buff_required =
                    optimal_tree->bufferRequired(psn_inst, buff);
                for (auto& tree : buffer_trees_)
                {
                    auto req = tree->bufferRequired(psn_inst, buff);
                    if (req > buff_required)
                    {
                        optimal_tree  = tree;
                        buff_required = req;
                    }
                }
                auto buffer_cost = psn_inst->handler()->area(buff);
                auto buffer_cap =
                    psn_inst->handler()->bufferInputCapacitance(buff);
                auto buffer_opt = std::make_shared<BufferTree>(
                    buffer_cap, buff_required,
                    optimal_tree->cost() + buffer_cost, pt, nullptr, nullptr,
                    buff);
                buffer_opt->setBufferCount(optimal_tree->bufferCount() + 1);
                buffer_opt->setLeft(optimal_tree);
                buffer_trees_.push_back(buffer_opt);
            }
            for (auto& inv : inverter_lib)
            {
                auto optimal_tree = *(buffer_trees_.begin());
                auto buff_required =
                    optimal_tree->bufferRequired(psn_inst, inv);
                for (auto& tree : buffer_trees_)
                {
                    auto req = tree->bufferRequired(psn_inst, inv);
                    if (req > buff_required)
                    {
                        optimal_tree  = tree;
                        buff_required = req;
                    }
                }
                auto buffer_cost = psn_inst->handler()->area(inv);
                auto buffer_cap =
                    psn_inst->handler()->inverterInputCapacitance(inv);
                auto buffer_opt = std::make_shared<BufferTree>(
                    buffer_cap, buff_required,
                    optimal_tree->cost() + buffer_cost, pt, nullptr, nullptr,
                    inv);

                buffer_opt->setPolarity(!optimal_tree->polarity());
                buffer_opt->setBufferCount(optimal_tree->bufferCount() + 1);

                buffer_opt->setLeft(optimal_tree);
                buffer_trees_.push_back(buffer_opt);
            }
        }
        else
        {
            std::sort(buffer_trees_.begin(), buffer_trees_.end(),
                      [&](const std::shared_ptr<BufferTree>& a,
                          const std::shared_ptr<BufferTree>& b) -> bool {
                          return a->cost() < b->cost();
                      });
            // auto sol_tree = buffer_trees_[0];
            std::vector<std::shared_ptr<BufferTree>> new_trees;
            for (auto& buff : buffer_lib)
            {
                for (auto& sol_tree : buffer_trees_)
                {
                    auto buffer_cost = psn_inst->handler()->area(buff);
                    auto buffer_cap =
                        psn_inst->handler()->bufferInputCapacitance(buff);
                    float buffer_slew = sol_tree->bufferSlew(psn_inst, buff);
                    if (buffer_slew < slew_limit)
                    {
                        auto buffer_opt = std::make_shared<TimerlessBufferTree>(
                            buffer_cap, 0, sol_tree->cost() + buffer_cost, pt,
                            nullptr, nullptr, buff);
                        buffer_opt->setBufferCount(sol_tree->bufferCount() + 1);
                        buffer_opt->setLeft(sol_tree);
                        new_trees.push_back(buffer_opt);
                        break;
                    }
                }
            }
            for (auto& inv : inverter_lib)
            {
                for (auto& sol_tree : buffer_trees_)
                {
                    auto buffer_cost = psn_inst->handler()->area(inv);
                    auto buffer_cap =
                        psn_inst->handler()->bufferInputCapacitance(inv);
                    float buffer_slew = sol_tree->bufferSlew(psn_inst, inv);
                    if (buffer_slew < slew_limit)
                    {
                        auto buffer_opt = std::make_shared<TimerlessBufferTree>(
                            buffer_cap, 0, sol_tree->cost() + buffer_cost, pt,
                            nullptr, nullptr, inv);
                        buffer_opt->setBufferCount(sol_tree->bufferCount() + 1);
                        buffer_opt->setLeft(sol_tree);
                        buffer_opt->setPolarity(!sol_tree->polarity());
                        new_trees.push_back(buffer_opt);
                        break;
                    }
                }
            }
            buffer_trees_.insert(buffer_trees_.end(), new_trees.begin(),
                                 new_trees.end());
            buffer_trees_.erase(
                std::remove_if(
                    buffer_trees_.begin(), buffer_trees_.end(),
                    [&](const std::shared_ptr<BufferTree>& t) -> bool {
                        if ((t->isBufferNode() &&
                             std::sqrt(
                                 std::pow(t->totalRequiredOrSlew(), 2) +
                                 std::pow(psn_inst->handler()->bufferDelay(
                                              t->bufferCell(),
                                              t->totalCapacitance()),
                                          2)) > slew_limit) ||
                            (!t->isBufferNode() &&
                             t->totalRequiredOrSlew() > slew_limit))
                        {
                            return true;
                        }
                        if (t->hasDownstreamSlewViolation(psn_inst, slew_limit))
                        {
                            return true;
                        }

                        return false;
                    }),
                buffer_trees_.end());
        }
    }
    void
    addUpstreamReferences(Psn*                        psn_inst,
                          std::shared_ptr<BufferTree> base_buffer_tree)
    {
        return; // Not used anymore..
        for (auto& tree : bufferTrees())
        {
            if (tree != base_buffer_tree)
            {
                if (!base_buffer_tree->hasUpstreamBufferCell())
                {
                    base_buffer_tree->setUpstreamBufferCell(tree->bufferCell());
                }
                else
                {
                    if (tree->bufferRequired(psn_inst) <
                        base_buffer_tree->upstreamBufferRequired(psn_inst))
                    {
                        base_buffer_tree->setUpstreamBufferCell(
                            tree->bufferCell());
                    }
                }
            }
        }
    }

    std::shared_ptr<BufferTree>
    optimalRequiredTree(Psn* psn_inst)
    {
        if (!buffer_trees_.size())
        {
            return nullptr;
        }
        auto optimal_tree  = *(buffer_trees_.begin());
        auto buff_required = optimal_tree->bufferRequired(psn_inst);
        for (auto& tree : buffer_trees_)
        {
            auto req = tree->bufferRequired(psn_inst);
            if (req > buff_required)
            {
                optimal_tree  = tree;
                buff_required = req;
            }
        }
        return optimal_tree;
    }
    std::shared_ptr<BufferTree>
    optimalDriverTree(Psn* psn_inst, InstanceTerm* driver_pin,
                      std::vector<LibraryCell*> driver_types,
                      float                     area_penalty)
    {
        if (!buffer_trees_.size())
        {
            return nullptr;
        }
        float max_slack;
        auto  max_tree = optimalDriverTree(psn_inst, driver_pin, &max_slack);
        auto  inst     = psn_inst->handler()->instance(driver_pin);
        auto  original_lib = psn_inst->handler()->libraryCell(inst);
        max_tree->setDriverCell(original_lib);
        for (auto& drv_type : driver_types)
        {
            for (size_t i = 0; i < buffer_trees_.size(); i++)
            {
                auto& tree = buffer_trees_[i];
                if (tree->polarity())
                {
                    continue;
                }
                auto drvr_pin =
                    psn_inst->handler()->libraryOutputPins(drv_type)[0];
                float max_cap =
                    psn_inst->handler()->largestInputCapacitance(drv_type);

                float penalty =
                    psn_inst->handler()->bufferChainDelayPenalty(max_cap) +
                    area_penalty * psn_inst->handler()->area(drv_type);

                float delay = psn_inst->handler()->gateDelay(
                    drvr_pin, tree->totalCapacitance());
                float slack = tree->totalRequiredOrSlew() - delay - penalty;

                if (slack > max_slack)
                {
                    max_slack = slack;
                    max_tree  = tree;
                    max_tree->setDriverCell(drv_type);
                }
            }
        }
        return max_tree;
    }

    std::shared_ptr<BufferTree>
    optimalTimerlessDriverTree(Psn* psn_inst, InstanceTerm* driver_pin)
    {
        if (!buffer_trees_.size() || !isTimerless())
        {
            return nullptr;
        }
        std::sort(buffer_trees_.begin(), buffer_trees_.end(),
                  [&](const std::shared_ptr<BufferTree>& a,
                      const std::shared_ptr<BufferTree>& b) -> bool {
                      return a->cost() < b->cost();
                  });
        float slew_limit = psn_inst->handler()->pinSlewLimit(driver_pin);
        for (auto& tr : buffer_trees_)
        {
            float in_slew = psn_inst->handler()->slew(
                psn_inst->handler()->libraryPin(driver_pin),
                tr->totalCapacitance());
            if (tr->bufferCount() &&
                isLess(tr->totalRequiredOrSlew(), slew_limit, 1E-6F) &&
                !tr->hasDownstreamSlewViolation(psn_inst, slew_limit, in_slew))
            {
                return tr;
            }
        }
        return buffer_trees_[0];
    }
    std::shared_ptr<BufferTree>
    optimalDriverTree(Psn* psn_inst, InstanceTerm* driver_pin,
                      float* tree_slack = nullptr)
    {
        if (!buffer_trees_.size())
        {
            return nullptr;
        }

        auto first_tree = buffer_trees_[0];
        std::sort(buffer_trees_.begin(), buffer_trees_.end(),
                  [&](const std::shared_ptr<BufferTree>& a,
                      const std::shared_ptr<BufferTree>& b) -> bool {
                      float a_delay = psn_inst->handler()->gateDelay(
                          driver_pin, a->totalCapacitance());
                      float a_slack = a->totalRequiredOrSlew() - a_delay;
                      float b_delay = psn_inst->handler()->gateDelay(
                          driver_pin, b->totalCapacitance());
                      float b_slack = b->totalRequiredOrSlew() - b_delay;
                      return a_slack > b_slack ||
                             (isEqual(a_slack, b_slack, 1E-6F) &&
                              a->cost() < b->cost());
                  });

        float                       max_slack = -1E+30F;
        std::shared_ptr<BufferTree> max_tree  = nullptr;
        for (auto& tree : buffer_trees_)
        {
            if (tree->polarity())
            {
                continue;
            }
            float delay = psn_inst->handler()->gateDelay(
                driver_pin, tree->totalCapacitance());
            float slack = tree->totalRequiredOrSlew() - delay;

            if (isGreater(slack, max_slack))
            {
                max_slack = slack;
                max_tree  = tree;
                if (tree_slack)
                {
                    *tree_slack = max_slack;
                }
                break; // Already sorted, no need to continue searching.
            }
        }
        return max_tree;
    }

    static bool
    isGreater(float first, float second, float threshold = 1E-6F)
    {
        return first > second &&
               !(std::abs(first - second) <
                 threshold * std::max(std::abs(first), std::abs(second)));
    }
    static bool
    isLess(float first, float second, float threshold)
    {
        return first < second &&
               !(std::abs(first - second) <
                 threshold * std::max(std::abs(first), std::abs(second)));
    }
    static bool
    isEqual(float first, float second, float threshold)
    {
        return std::abs(first - second) <
               threshold * std::max(std::abs(first), std::abs(second));
    }
    static bool
    isLessOrEqual(float first, float second, float threshold)
    {
        return first < second ||
               (std::abs(first - second) <
                threshold * std::max(std::abs(first), std::abs(second)));
    }
    static bool
    isGreaterOrEqual(float first, float second, float threshold)
    {
        return first > second ||
               (std::abs(first - second) <
                threshold * std::max(std::abs(first), std::abs(second)));
    }

    void
    prune(Psn* psn_inst, LibraryCell* upstream_res_cell,
          float       minimum_upstream_res_or_max_slew,
          const float cap_prune_threshold  = 1E-6F,
          const float cost_prune_threshold = 1E-6F)
    {
        if (!isTimerless()) // Timing-driven
        {
            // TODO Add squeeze pruning
            if (!upstream_res_cell)
            {
                PSN_LOG_WARN("Pruning without upstream resistance");
                return;
            }

            std::sort(buffer_trees_.begin(), buffer_trees_.end(),
                      [&](const std::shared_ptr<BufferTree>& a,
                          const std::shared_ptr<BufferTree>& b) -> bool {
                          float left_req =
                              a->bufferRequired(psn_inst, upstream_res_cell);
                          float right_req =
                              b->bufferRequired(psn_inst, upstream_res_cell);
                          return left_req > right_req;
                      });

            size_t index = 0;
            for (size_t i = 0; i < buffer_trees_.size(); i++)
            {
                index = i + 1;
                for (size_t j = i + 1; j < buffer_trees_.size(); j++)
                {
                    if (isLess(buffer_trees_[j]->totalCapacitance(),
                               buffer_trees_[i]->totalCapacitance(),
                               cap_prune_threshold) ||
                        isLess(buffer_trees_[j]->cost(),
                               buffer_trees_[i]->cost(), cost_prune_threshold))
                    {
                        buffer_trees_[index++] = buffer_trees_[j];
                    }
                }
                buffer_trees_.resize(index);
            }
            if (minimum_upstream_res_or_max_slew)
            {
                std::sort(buffer_trees_.begin(), buffer_trees_.end(),
                          [](const std::shared_ptr<BufferTree>& a,
                             const std::shared_ptr<BufferTree>& b) -> bool {
                              return a->totalRequiredOrSlew() <
                                     b->totalRequiredOrSlew();
                          });

                index = 0;
                for (size_t i = 0; i < buffer_trees_.size(); i++)
                {
                    index = i + 1;
                    for (size_t j = i + 1; j < buffer_trees_.size(); j++)
                    {
                        if (isGreaterOrEqual(
                                buffer_trees_[i]->totalCapacitance(),
                                buffer_trees_[j]->totalCapacitance(),
                                cap_prune_threshold) ||
                            !((buffer_trees_[j]->totalRequiredOrSlew() -
                               buffer_trees_[i]->totalRequiredOrSlew()) /
                                  (buffer_trees_[j]->totalCapacitance() -
                                   buffer_trees_[i]->totalCapacitance()) <
                              minimum_upstream_res_or_max_slew))
                        {
                            buffer_trees_[index++] = buffer_trees_[j];
                        }
                    }
                    buffer_trees_.resize(index);
                }
            }
        }
        else
        {
            buffer_trees_.erase(
                std::remove_if(
                    buffer_trees_.begin(), buffer_trees_.end(),
                    [&](const std::shared_ptr<BufferTree>& t) -> bool {
                        return isGreaterOrEqual(
                            t->totalRequiredOrSlew(),
                            minimum_upstream_res_or_max_slew,
                            cap_prune_threshold);
                    }),
                buffer_trees_.end());
            std::sort(buffer_trees_.begin(), buffer_trees_.end(),
                      [](const std::shared_ptr<BufferTree>& a,
                         const std::shared_ptr<BufferTree>& b) -> bool {
                          return a->cost() < b->cost();
                      });
            size_t index = 0;
            for (size_t i = 0; i < buffer_trees_.size(); i++)
            {
                index = i + 1;
                for (size_t j = i + 1; j < buffer_trees_.size(); j++)
                {
                    if ((isLess(buffer_trees_[j]->totalRequiredOrSlew(),
                                buffer_trees_[i]->totalRequiredOrSlew(),
                                cap_prune_threshold) ||
                         isLess(buffer_trees_[j]->totalCapacitance(),
                                buffer_trees_[i]->totalCapacitance(),
                                cap_prune_threshold)))
                    {
                        buffer_trees_[index++] = buffer_trees_[j];
                    }
                }
                buffer_trees_.resize(index);
            }
        }
    }
    void
    setMode(BufferMode buffer_mode)
    {
        mode_ = buffer_mode;
    }

    BufferMode
    mode() const
    {
        return mode_;
    }

    bool
    isTimerless() const
    {
        return mode_ == BufferMode::Timerless;
    }
}; // namespace psn

class TimerlessBufferSolution : public BufferSolution
{

public:
    TimerlessBufferSolution() : BufferSolution(BufferMode::Timerless)
    {
    }
    TimerlessBufferSolution(Psn*                             psn_inst,
                            std::shared_ptr<BufferSolution>& left,
                            std::shared_ptr<BufferSolution>& right,
                            Point location, LibraryCell* upstream_res_cell,
                            float      minimum_upstream_res_or_max_slew,
                            BufferMode buffer_mode = BufferMode::TimingDriven)
        : BufferSolution(psn_inst, left, right, location, upstream_res_cell,
                         minimum_upstream_res_or_max_slew,
                         BufferMode::Timerless)
    {
    }
};
} // namespace psn
