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

#include "BufferFanoutTransform.hpp"
#include <OpenPhySyn/PsnLogger/PsnLogger.hpp>
#include <OpenPhySyn/Utils/PsnGlobal.hpp>

#include <algorithm>
#include <cmath>

using namespace psn;

int
BufferFanoutTransform::buffer(Psn* psn_inst, int max_fanout,
                              std::string buffer_cell,
                              std::string buffer_in_port,
                              std::string buffer_out_port)
{

    PsnLogger&       logger  = PsnLogger::instance();
    DatabaseHandler& handler = *(psn_inst->handler());
    LibraryCell*     cell    = handler.libraryCell(buffer_cell.c_str());
    if (!cell)
    {
        logger.error("Buffer {} not found!", buffer_cell);
        return -1;
    }
    LibraryTerm* cell_in_pin = handler.libraryPin(cell, buffer_in_port.c_str());
    LibraryTerm* cell_out_pin =
        handler.libraryPin(cell, buffer_out_port.c_str());
    if (!cell_in_pin)
    {
        logger.error("Pin {} not found!", buffer_in_port);
        return -1;
    }
    if (!cell_out_pin)
    {
        logger.error("Pin {} not found!", buffer_out_port);
        return -1;
    }
    auto              nets = handler.nets();
    std::vector<Net*> high_fanout_nets;
    for (auto& net : nets)
    {
        if (!handler.isPrimary(net) &&
            handler.fanoutCount(net) > (unsigned int)max_fanout)
        {
            high_fanout_nets.push_back(net);
        }
    }
    logger.info("High fanout nets [{}]: ", high_fanout_nets.size());
    for (auto& net : high_fanout_nets)
    {
        logger.info("Net: {} {}", handler.name(net), handler.fanoutCount(net));
    }
    // To-Do: Remove clock net..
    int create_buffer_count = 0;

    std::vector<int> current_buffer;
    for (auto& net : high_fanout_nets)
    {
        InstanceTerm* source_pin = handler.faninPin(net);
        if (source_pin)
        {
            logger.info("Buffering: {}", handler.name(net));
            auto fanout_pins        = handler.fanoutPins(net);
            int  net_sink_pin_count = fanout_pins.size();
            logger.info("Sink count: {}", net_sink_pin_count);
            int              iter = net_sink_pin_count;
            std::vector<int> buffer_hier;
            while (iter > max_fanout)
            {
                iter = (iter * 1.0) / max_fanout;
                buffer_hier.push_back(ceil(iter));
            }
            handler.disconnectAll(net);
            handler.connect(net, source_pin);

            int current_sink_count = 0;
            int levels             = buffer_hier.size();
            if (!levels)
            {
                continue;
            }
            for (int i = 0; i < levels; i++)
            {
                current_buffer.push_back(0);
            }

            while (current_sink_count < net_sink_pin_count)
            {

                if (current_buffer[current_buffer.size() - 1] == 0)
                {
                    for (int i = 1; i < levels; i++)
                    {
                        std::vector<int> parent_buf(current_buffer.begin(),
                                                    current_buffer.end() - i);
                        if (handler.instance(bufferName(parent_buf).c_str()) ==
                            nullptr)
                        {
                            auto      buf_name = bufferName(parent_buf);
                            auto      net_name = bufferNetName(parent_buf);
                            Instance* new_buffer =
                                handler.createInstance(buf_name.c_str(), cell);
                            create_buffer_count++;
                            Net* new_net = handler.createNet(net_name.c_str());
                            handler.connect(new_net, new_buffer, cell_out_pin);
                            if (i == levels - 1)
                            {
                                handler.connect(net, new_buffer, cell_in_pin);
                            }
                            else
                            {
                                handler.connect(
                                    handler.net(
                                        bufferNetName(std::vector<int>(
                                                          parent_buf.begin(),
                                                          parent_buf.end() - 1))
                                            .c_str()),
                                    new_buffer, cell_in_pin);
                            }
                        }
                    }
                }
                auto buf_name = bufferName(current_buffer);
                auto net_name = bufferNetName(current_buffer);

                Instance* new_buffer =
                    handler.createInstance(buf_name.c_str(), cell);
                create_buffer_count++;
                Net* new_net = handler.createNet(net_name.c_str());
                handler.connect(new_net, new_buffer, cell_out_pin);
                int sink_connect_count = std::min(
                    max_fanout, net_sink_pin_count - current_sink_count);
                for (int i = 0; i < sink_connect_count; i++)
                {
                    handler.connect(new_net, fanout_pins[current_sink_count]);
                    current_sink_count++;
                }
                if (levels == 1)
                {
                    handler.connect(net, new_buffer, cell_in_pin);
                }
                else
                {
                    handler.connect(
                        handler.net(bufferNetName(std::vector<int>(
                                                      current_buffer.begin(),
                                                      current_buffer.end() - 1))
                                        .c_str()),
                        new_buffer, cell_in_pin);
                }

                current_buffer = nextBuffer(current_buffer, max_fanout);
            }
        }
    }
    logger.info("Added {} buffers", create_buffer_count);

    return create_buffer_count;
}
std::vector<int>
BufferFanoutTransform::nextBuffer(std::vector<int> current_buffer,
                                  int              max_fanout)
{
    int              levels    = current_buffer.size();
    int              incr_done = 0;
    std::vector<int> next_buffer;
    for (int i = levels - 1; i >= 0; i--)
    {
        if (incr_done)
        {
            next_buffer.insert(next_buffer.begin(), current_buffer[i]);
        }
        else if (current_buffer[i] == max_fanout - 1)
        {
            next_buffer.insert(next_buffer.begin(), 0);
        }
        else
        {
            next_buffer.insert(next_buffer.begin(), current_buffer[i] + 1);
            incr_done = 1;
        }
    }
    return next_buffer;
}
std::string
BufferFanoutTransform::bufferName(int index)
{
    return std::string("buf_" + std::to_string(index) + "_");
}

std::string
BufferFanoutTransform::bufferNetName(int index)
{
    return std::string("bufnet_" + std::to_string(index) + "_");
}
std::string
BufferFanoutTransform::bufferName(std::vector<int> indices)
{
    std::string name;
    for (unsigned int i = 0; i < indices.size(); i++)
    {
        name += std::to_string(indices[i]);
        if (i != indices.size() - 1)
        {
            name += "_";
        }
    }
    return std::string("buf_" + name);
}

std::string
BufferFanoutTransform::bufferNetName(std::vector<int> indices)
{
    std::string name;
    for (unsigned int i = 0; i < indices.size(); i++)
    {
        name += std::to_string(indices[i]);
        if (i != indices.size() - 1)
        {
            name += "_";
        }
    }
    return std::string("bufnet_" + name);
}

bool
BufferFanoutTransform::isNumber(const std::string& s)
{
    return !s.empty() && std::find_if(s.begin(), s.end(), [](char c) {
                             return !std::isdigit(c);
                         }) == s.end();
}

int
BufferFanoutTransform::run(Psn* psn_inst, std::vector<std::string> args)
{

    if (args.size() == 4 && isNumber(args[0]))
    {
        return buffer(psn_inst, stoi(args[0]), args[1], args[2], args[3]);
    }
    else
    {
        PsnLogger::instance().error(
            "Usage:\n transform hello_transform "
            "<net_name>\n transform hello_transform "
            "buffer "
            "<max_fanout> <buffer_cell> <buffer_in_port> "
            "<buffer_out_port>");
    }

    return -1;
}
