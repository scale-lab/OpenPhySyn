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

#include <memory>
namespace psn
{
class BufferTree
{
    float                       capacitance_;
    float                       required_;
    float                       wire_capacitance_;
    float                       wire_delay_;
    float                       cost_;
    psn::Point                  location_;
    std::shared_ptr<BufferTree> left_, right_;
    psn::LibraryCell*           buffer_cell_;
    psn::InstanceTerm*          pin_;

public:
    BufferTree(float cap = 0.0, float req = 0.0, float cost = 0.0,
               psn::Point         location    = psn::Point(0, 0),
               psn::InstanceTerm* pin         = nullptr,
               psn::LibraryCell*  buffer_cell = nullptr)
        : capacitance_(cap),
          required_(req),
          cost_(cost),
          location_(location),
          buffer_cell_(buffer_cell),
          pin_(pin)
    {
    }
    BufferTree(std::shared_ptr<BufferTree>& left,
               std::shared_ptr<BufferTree>& right, psn::Point location)

    {
        left_             = left;
        right_            = right;
        capacitance_      = left_->capacitance() + right_->capacitance();
        required_         = std::min(left->required(), right->required());
        cost_             = left_->cost() + right_->cost();
        wire_delay_       = 0;
        wire_capacitance_ = 0;
        location_         = location;
    }
    float
    capacitance() const
    {
        return capacitance_;
    }
    float
    required() const
    {
        return required_;
    }
    float
    wireCapacitance() const
    {
        return wire_capacitance_;
    }
    float
    wireDelay() const
    {
        return wire_delay_;
    }
    float
    cost() const
    {
        return cost_;
    }
    psn::InstanceTerm*
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
    setRequired(float req)
    {
        required_ = req;
    }
    void
    setWireCapacitance(float cap)
    {
        wire_capacitance_ = cap;
    }
    void
    setWireDelay(float delay)
    {
        wire_delay_ = delay;
    }
    void
    setCost(float cost)
    {
        cost_ = cost;
    }
    void
    setPin(psn::InstanceTerm* pin)
    {
        pin_ = pin;
    }
    float
    bufferRequired(psn::Psn* psn_inst, psn::LibraryCell* buffer_cell) const
    {
        return required_ - wire_delay_ -
               psn_inst->handler()->bufferDelay(buffer_cell, capacitance_);
    }
    float
    bufferRequired(psn::Psn* psn_inst) const
    {
        return bufferRequired(psn_inst, buffer_cell_);
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
    setLeft(std::shared_ptr<BufferTree>& left)
    {
        left_ = left;
    }
    void
    setRight(std::shared_ptr<BufferTree>& right)
    {
        right_ = right;
    }
    psn::LibraryCell*
    bufferCell() const
    {
        return buffer_cell_;
    }
    void
    setBufferCell(psn::LibraryCell* buffer_cell)
    {
        buffer_cell_ = buffer_cell;
    }

    bool
    isBuffered() const
    {
        return buffer_cell_ != nullptr;
    }
    bool
    isUnbuffered() const
    {
        return !isBranched() && !isBuffered();
    }
    bool
    isBranched() const
    {
        return left_ != nullptr && right_ != nullptr;
    }
    bool
    operator<(const BufferTree& other) const
    {
        return (required_ - wire_delay_) <
               (other.required_ - other.wire_delay_);
    }
    bool
    operator>(const BufferTree& other) const
    {
        return (required_ - wire_delay_) >
               (other.required_ - other.wire_delay_);
    }
    bool
    operator<=(const BufferTree& other) const
    {
        return (required_ - wire_delay_) <=
               (other.required_ - other.wire_delay_);
    }
    bool
    operator>=(const BufferTree& other) const
    {
        return (required_ - wire_delay_) >=
               (other.required_ - other.wire_delay_);
    }
};

class BufferSolution
{
    std::vector<std::shared_ptr<BufferTree>> buffer_trees_;

public:
    BufferSolution(){};
    BufferSolution(std::shared_ptr<BufferSolution> left,
                   std::shared_ptr<BufferSolution> right, psn::Point location)

    {
        buffer_trees_.resize(left->bufferTrees().size() +
                             right->bufferTrees().size());
        int index = 0;
        for (auto& left_branch : left->bufferTrees())
        {
            for (auto& right_branch : right->bufferTrees())
            {
                buffer_trees_[index++] = std::shared_ptr<BufferTree>(
                    new BufferTree(left_branch, right_branch, location));
            }
        }
    }
    void
    addTree(std::shared_ptr<BufferTree> tree)
    {
        buffer_trees_.push_back(std::move(tree));
    }
    std::vector<std::shared_ptr<BufferTree>>&
    bufferTrees()
    {
        return buffer_trees_;
    }
    void
    addWireDelayAndCapacitance(float wire_delay, float wire_cap)
    {
        for (auto& tree : buffer_trees_)
        {
            tree->setWireDelay(wire_delay);
            tree->setWireCapacitance(wire_cap);
        }
    }

    void
    addLeafTrees(psn::Psn* psn_inst, psn::Point pt,
                 std::unordered_set<psn::LibraryCell*>& buffer_lib,
                 std::unordered_set<psn::LibraryCell*>&)
    {
        if (!buffer_trees_.size())
        {
            return;
        }
        for (auto& buff : buffer_lib)
        {
            auto optimal_tree  = *(buffer_trees_.begin());
            auto buff_required = optimal_tree->bufferRequired(psn_inst, buff);
            for (auto& tree : buffer_trees_)
            {
                auto req = tree->bufferRequired(psn_inst, buff);
                if (req > buff_required)
                {
                    optimal_tree  = tree;
                    buff_required = req;
                }
            }
            // Disable cost for now..
            // auto buffer_cost = psn_inst->area(buff);
            auto buffer_cost = 0.0;
            auto buffer_cap = psn_inst->handler()->bufferInputCapacitance(buff);
            auto buffer_opt = std::shared_ptr<BufferTree>(new BufferTree(
                buffer_cap, buff_required, buffer_cost, pt, nullptr, buff));
            buffer_opt->setLeft(optimal_tree);
            buffer_trees_.push_back(buffer_opt);
        }
    }
    std::shared_ptr<BufferTree>
    optimalTree(psn::Psn* psn_inst)
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

    void
    prune(const int prune_threshold = 0)
    {
        // TODO Add squeeze pruning
        PSN_UNUSED(prune_threshold);
        std::sort(buffer_trees_.begin(), buffer_trees_.end(),
                  std::greater<std::shared_ptr<BufferTree>>());

        size_t index = 0;
        for (size_t i = 0; i < buffer_trees_.size(); i++)
        {
            index = i + 1;
            for (size_t j = i + 1; j < buffer_trees_.size(); j++)
            {
                if ((buffer_trees_[j]->capacitance() +
                     buffer_trees_[j]->wireCapacitance()) <=
                        (buffer_trees_[i]->capacitance() +
                         buffer_trees_[i]->wireCapacitance()) &&
                    buffer_trees_[j]->cost() <= buffer_trees_[i]->cost())
                {
                    buffer_trees_[index++] = buffer_trees_[j];
                }
            }
        }
        buffer_trees_.resize(index);
    }
};
} // namespace psn