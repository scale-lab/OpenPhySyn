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
    Point                       location_;
    std::shared_ptr<BufferTree> left_, right_;
    LibraryCell*                buffer_cell_;
    InstanceTerm*               pin_;
    LibraryCell*                upstream_buffer_cell_;

public:
    BufferTree(float cap = 0.0, float req = 0.0, float cost = 0.0,
               Point location = Point(0, 0), InstanceTerm* pin = nullptr,
               LibraryCell* buffer_cell = nullptr)
        : capacitance_(cap),
          required_(req),
          cost_(cost),
          location_(location),
          buffer_cell_(buffer_cell),
          pin_(pin),
          upstream_buffer_cell_(buffer_cell)
    {
    }
    BufferTree(Psn* psn_inst, std::shared_ptr<BufferTree> left,
               std::shared_ptr<BufferTree> right, Point location)

    {
        left_        = left;
        right_       = right;
        capacitance_ = left_->totalCapacitance() + right_->totalCapacitance();
        required_    = std::min(left->totalRequired(), right->totalRequired());
        cost_        = left_->cost() + right_->cost();
        wire_delay_  = 0;
        wire_capacitance_     = 0;
        location_             = location;
        buffer_cell_          = nullptr;
        upstream_buffer_cell_ = nullptr;

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
    required() const
    {
        return required_;
    }
    float
    totalRequired() const
    {
        return required_ - wire_delay_;
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
    setPin(InstanceTerm* pin)
    {
        pin_ = pin;
    }
    float
    bufferRequired(Psn* psn_inst, LibraryCell* buffer_cell) const
    {
        return totalRequired() - psn_inst->handler()->bufferDelay(
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

    Point
    location() const
    {
        return location_;
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
    BufferSolution(Psn* psn_inst, std::shared_ptr<BufferSolution>& left,
                   std::shared_ptr<BufferSolution>& right, Point location)

    {
        mergeBranches(psn_inst, left, right, location);
    }
    void
    mergeBranches(Psn* psn_inst, std::shared_ptr<BufferSolution>& left,
                  std::shared_ptr<BufferSolution>& right, Point location)
    {
        buffer_trees_.resize(left->bufferTrees().size() *
                                 right->bufferTrees().size(),
                             std::make_shared<BufferTree>());
        int index = 0;
        for (auto& left_branch : left->bufferTrees())
        {
            for (auto& right_branch : right->bufferTrees())
            {
                buffer_trees_[index++] = std::make_shared<BufferTree>(
                    psn_inst, left_branch, right_branch, location);
            }
        }
        prune(psn_inst);
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
    addWireDelayAndCapacitance(float wire_delay, float wire_cap)
    {
        for (auto& tree : buffer_trees_)
        {
            tree->setWireDelay(wire_delay);
            tree->setWireCapacitance(wire_cap);
        }
    }

    void
    addLeafTrees(Psn* psn_inst, Point pt,
                 std::unordered_set<LibraryCell*>& buffer_lib,
                 std::unordered_set<LibraryCell*>&)
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
            auto buffer_opt = std::make_shared<BufferTree>(
                buffer_cap, buff_required, buffer_cost, pt, nullptr, buff);
            buffer_opt->setLeft(optimal_tree);
            buffer_trees_.push_back(buffer_opt);
        }
    }
    void
    addUpstreamReferences(Psn*                        psn_inst,
                          std::shared_ptr<BufferTree> base_buffer_tree)
    {
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
    optimalDriverTree(Psn* psn_inst, InstanceTerm* driver_pin)
    {
        if (!buffer_trees_.size())
        {
            return nullptr;
        }
        float                       max_slack = -1E+30F;
        std::shared_ptr<BufferTree> max_tree  = nullptr;
        for (auto& tree : buffer_trees_)
        {
            float delay = psn_inst->handler()->gateDelay(
                driver_pin, tree->totalCapacitance());
            float slack = tree->totalRequired() - delay;

            if (slack > max_slack)
            {
                max_slack = slack;
                max_tree  = tree;
            }
        }
        return max_tree;
    }

    void
    prune(Psn* psn_inst, const float prune_threshold = 1E-6F)
    {
        // TODO Add squeeze pruning

        std::sort(buffer_trees_.begin(), buffer_trees_.end(),
                  [=](const std::shared_ptr<BufferTree>& a,
                      const std::shared_ptr<BufferTree>& b) -> bool {
                      float left_req  = a->upstreamBufferRequired(psn_inst);
                      float right_req = b->upstreamBufferRequired(psn_inst);
                      return left_req > right_req;
                  });

        size_t index = 0;
        // PSN_LOG_INFO("Before prune {}", buffer_trees_.size());
        // PSN_LOG_DEBUG("Before prune {}", buffer_trees_.size());
        // for (size_t i = 0; i < buffer_trees_.size(); i++)
        // {
        //     PSN_LOG_DEBUG("i {} Cap ({}) + RC: {}, Req {}, UREQ {} ( + RC:
        //     {})",
        //                   i, buffer_trees_[i]->capacitance(),
        //                   buffer_trees_[i]->totalCapacitance(),
        //                   buffer_trees_[i]->required(),
        //                   buffer_trees_[i]->upstreamBufferRequired(psn_inst),
        //                   buffer_trees_[i]->upstreamBufferRequired(psn_inst)
        //                   +
        //                       buffer_trees_[i]->wireDelay());
        // }
        for (size_t i = 0; i < buffer_trees_.size(); i++)
        {
            index = i + 1;
            for (size_t j = i + 1; j < buffer_trees_.size(); j++)
            {
                if (buffer_trees_[j]->totalCapacitance() <
                        buffer_trees_[i]->totalCapacitance() &&
                    !(std::abs(buffer_trees_[j]->totalCapacitance() -
                               buffer_trees_[i]->totalCapacitance()) <
                      prune_threshold *
                          std::max(
                              std::abs(buffer_trees_[j]->totalCapacitance()),
                              std::abs(buffer_trees_[i]->totalCapacitance()))))
                // &&
                // buffer_trees_[i]->cost() <= buffer_trees_[j]->cost())
                {
                    buffer_trees_[index++] = buffer_trees_[j];
                }
            }
            buffer_trees_.resize(index);
        }
        // PSN_LOG_INFO("After prune {}", buffer_trees_.size());
        // PSN_LOG_DEBUG("After prune {}", buffer_trees_.size());
        // for (size_t i = 0; i < buffer_trees_.size(); i++)
        // {
        //     PSN_LOG_DEBUG("i {} Cap ({}) + RC: {}, Req {}, UREQ {} ( +
        //     RC:{})",
        //                   i, buffer_trees_[i]->capacitance(),
        //                   buffer_trees_[i]->totalCapacitance(),
        //                   buffer_trees_[i]->required(),
        //                   buffer_trees_[i]->upstreamBufferRequired(psn_inst),
        //                   buffer_trees_[i]->upstreamBufferRequired(psn_inst)
        //                   +
        //                       buffer_trees_[i]->wireDelay());
        // }
    }
};
} // namespace psn