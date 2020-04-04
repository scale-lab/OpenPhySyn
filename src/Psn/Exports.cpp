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

#include "Exports.hpp"
#include <memory>
#include "OpenPhySyn/Psn/Psn.hpp"
#include "OpenPhySyn/Utils/PsnGlobal.hpp"
#include "OpenSTA/liberty/Liberty.hh"
#include "PsnLogger/PsnLogger.hpp"

namespace psn
{

int
import_def(const char* def_path)
{
    return Psn::instance().readDef(def_path);
}
int
import_lef_tech(const char* lef_path)
{
    return Psn::instance().readLef(lef_path, false, true);
}
int
import_lef_sc(const char* lef_path)
{
    return Psn::instance().readLef(lef_path, true, false);
}
int
import_lef_tech_sc(const char* lef_path)
{
    return Psn::instance().readLef(lef_path, true, true);
}
int
import_lib(const char* lib_path)
{
    return import_liberty(lib_path);
}
int
import_liberty(const char* lib_path)
{
    return Psn::instance().readLib(lib_path);
}
int
export_def(const char* lib_path)
{
    return Psn::instance().writeDef(lib_path);
}
int
import_db(const char* db_path)
{
    return Psn::instance().readDatabase(db_path);
}
int
export_db(const char* db_path)
{
    return Psn::instance().writeDatabase(db_path);
}
int
print_liberty_cells()
{
    Liberty* liberty = Psn::instance().liberty();
    if (!liberty)
    {
        PSN_LOG_ERROR("Did not find any liberty files, use "
                      "import_liberty <file name> first.");
        return -1;
    }
    sta::LibertyCellIterator cell_iter(liberty);
    while (cell_iter.hasNext())
    {
        sta::LibertyCell* cell = cell_iter.next();
        PSN_LOG_INFO("Cell: {}", cell->name());
    }
    return 1;
}

bool
has_transform(const char* transform_name)
{
    return Psn::instance().hasTransform(transform_name);
}
float
design_area()
{
    return Psn::instance().handler()->area();
}
int
set_wire_rc(float res_per_micon, float cap_per_micron)
{
    Psn::instance().setWireRC(res_per_micon, cap_per_micron);

    return 1;
}

int
set_wire_rc(const char* layer_name)
{
    return Psn::instance().setWireRC(layer_name);
}
int
set_max_area(float area)
{
    Psn::instance().handler()->setMaximumArea(area);
    return 1;
}

float
max_area()
{
    return Psn::instance().handler()->maximumArea();
}
float
core_area()
{
    return Psn::instance().handler()->coreArea();
}

void
set_dont_use(std::vector<std::string> cell_names)
{
    Psn::instance().handler()->setDontUse(cell_names);
}

bool
has_design()
{
    return Psn::instance().hasDesign();
}
bool
has_liberty()
{
    return Psn::instance().hasLiberty();
}

std::vector<std::string>
capacitance_violations()
{
    std::vector<std::string> names;
    if (!Psn::instance().hasDesign())
    {
        PSN_LOG_ERROR("Could not find any loaded design.");
        return names;
    }
    for (auto& pin : Psn::instance().handler()->maximumTransitionViolations())
    {
        names.push_back(Psn::instance().handler()->name(pin));
    }
    return names;
}
std::vector<std::string>
transition_violations()
{
    std::vector<std::string> names;
    if (!Psn::instance().hasDesign())
    {
        PSN_LOG_ERROR("Could not find any loaded design.");
        return names;
    }
    for (auto& pin : Psn::instance().handler()->maximumCapacitanceViolations())
    {
        names.push_back(Psn::instance().handler()->name(pin));
    }
    return names;
}

std::vector<std::string>
cluster_buffer_names(float cluster_threshold, bool find_superior)
{
    std::vector<std::string> names;
    auto                     cells = Psn::instance()
                     .handler()
                     ->bufferClusters(cluster_threshold, find_superior, false)
                     .first;
    for (auto& cell : cells)
    {
        names.push_back(Psn::instance().handler()->name(cell));
    }
    return names;
}
std::vector<std::string>
cluster_inverter_names(float cluster_threshold, bool find_superior)
{
    std::vector<std::string> names;
    auto                     cells = Psn::instance()
                     .handler()
                     ->bufferClusters(cluster_threshold, find_superior, true)
                     .second;
    for (auto& cell : cells)
    {
        names.push_back(Psn::instance().handler()->name(cell));
    }
    return names;
}

int
link(const char* top_module)
{
    return link_design(top_module);
}
int
link_design(const char* top_module)
{
    return Psn::instance().linkDesign(top_module);
}
void
version()
{
    print_version();
}

int
transform_internal(std::string transform_name, std::vector<std::string> args)
{
    return Psn::instance().runTransform(transform_name, args);
}
void
help()
{
    print_usage();
}

void
print_usage()
{
    Psn::instance().printUsage();
}
void
print_transforms()
{
    Psn::instance().printTransforms(true);
}
void
print_license()
{
    Psn::instance().printLicense(true);
}

void
print_version()
{
    Psn::instance().printVersion();
}

Database&
get_database()
{
    return *(Psn::instance().database());
}
Liberty&
get_liberty()
{
    return *(Psn::instance().liberty());
}

int
set_log(const char* level)
{
    return set_log_level(level);
}

int
set_log_level(const char* level)
{
    return Psn::instance().setLogLevel(level);
}

int
set_log_pattern(const char* pattern)
{
    return Psn::instance().setLogPattern(pattern);
}
DatabaseHandler&
get_handler()
{
    return get_database_handler();
}
DatabaseHandler&
get_database_handler()
{
    return *(Psn::instance().handler());
}

SteinerTree*
make_steiner_tree(const char* net_name)
{
    auto net = Psn::instance().handler()->net(net_name);
    return make_steiner_tree(net);
}
SteinerTree*
make_steiner_tree(Net* net)
{
    auto         pt   = SteinerTree::create(net, &(Psn::instance()), 3);
    SteinerTree* tree = pt.get();
    pt.release();
    return tree;
}

} // namespace psn
