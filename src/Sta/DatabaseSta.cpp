/////////////////////////////////////////////////////////////////////////////
// Original author: James Cherry, Parallax Software, Inc.
// Naming adjustments by: Ahmed A. Agiza (Ph.D. advisors: Sherief Reda)
// OpenStaDB, OpenSTA on OpenDB
// Copyright (c) 2019, Parallax Software, Inc.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "OpenPhySyn/Sta/DatabaseSta.hpp"
#include "OpenPhySyn/Sta/DatabaseStaNetwork.hpp"
#include "Sta/DatabaseSdcNetwork.hpp"
#include "opendb/db.h"
#include "sta/Bfs.hh"
#include "sta/Graph.hh"
#include "sta/Search.hh"
#include "sta/StaMain.hh"
namespace sta
{

extern "C"
{
    extern int Sta_Init(Tcl_Interp* interp);
}

extern const char* tcl_inits[];

DatabaseSta*
makeBlockSta(dbBlock* block)
{
    Sta*         sta  = Sta::sta();
    DatabaseSta* sta2 = new DatabaseSta;
    sta2->makeComponents();
    sta2->getDbNetwork()->setBlock(block);
    sta2->setTclInterp(sta->tclInterp());
    sta2->copyUnits(sta->units());
    return sta2;
}
DatabaseSta::DatabaseSta() : Sta(), db_(nullptr)
{
}

void
DatabaseSta::init(Tcl_Interp* tcl_interp, dbDatabase* db)
{
    initSta();
    Sta::setSta(this);
    db_ = db;
    makeComponents();
    setTclInterp(tcl_interp);
    // Define swig TCL commands.
    Sta_Init(tcl_interp);
    // Eval encoded sta TCL sources.
    evalTclInit(tcl_interp, tcl_inits);
    // Import exported commands from sta namespace to global namespace.
    Tcl_Eval(tcl_interp, "sta::define_sta_cmds");
    Tcl_Eval(tcl_interp, "namespace import sta::*");
}

// Wrapper to init network db.
void
DatabaseSta::makeComponents()
{
    Sta::makeComponents();
    db_network_->setDb(db_);
}

void
DatabaseSta::makeNetwork()
{
    db_network_ = new class DatabaseStaNetwork();
    network_    = db_network_;
}

void
DatabaseSta::makeSdcNetwork()
{
    sdc_network_ = new DatabaseSdcNetwork(network_);
}

void
DatabaseSta::postReadLef(dbTech* tech, dbLib* library)
{
    if (library)
    {
        db_network_->readLefAfter(library);
    }
}

void
DatabaseSta::postReadDef(dbBlock* block)
{
    db_network_->readDefAfter(block);
}

void
DatabaseSta::postReadDb(dbDatabase* db)
{
    db_network_->readDbAfter(db);
}

// Wrapper to sync db/liberty libraries.
LibertyLibrary*
DatabaseSta::readLiberty(const char* filename, Corner* corner,
                         const MinMaxAll* min_max, bool infer_latches)

{
    LibertyLibrary* lib =
        Sta::readLiberty(filename, corner, min_max, infer_latches);
    db_network_->readLibertyAfter(lib);
    return lib;
}

Slack
DatabaseSta::netSlack(const dbNet* db_net, const MinMax* min_max)
{
    const Net* net = db_network_->dbToSta(db_net);
    return netSlack(net, min_max);
}

void
DatabaseSta::findClkNets(std::set<dbNet*>& clk_nets)
{
    ensureGraph();
    ensureLevelized();
    ClkArrivalSearchPred srch_pred(this);
    BfsFwdIterator       bfs(BfsIndex::other, &srch_pred, this);
    PinSet               clk_pins;
    search_->findClkVertexPins(clk_pins);
    for (Pin* pin : clk_pins)
    {
        Vertex *vertex, *bidirect_drvr_vertex;
        graph_->pinVertices(pin, vertex, bidirect_drvr_vertex);
        bfs.enqueue(vertex);
        if (bidirect_drvr_vertex)
            bfs.enqueue(bidirect_drvr_vertex);
    }
    while (bfs.hasNext())
    {
        Vertex*    vertex = bfs.next();
        const Pin* pin    = vertex->pin();
        if (!network_->isTopLevelPort(pin))
        {
            Net* net = network_->net(pin);
            clk_nets.insert(db_network_->staToDb(net));
        }
        bfs.enqueueAdjacentVertices(vertex);
    }
}

} // namespace sta