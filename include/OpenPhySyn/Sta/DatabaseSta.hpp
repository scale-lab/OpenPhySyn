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
// This code is originally written by James Cherry and adapted for OpenPhySyn

#pragma once

#include "opendb/db.h"
#include "sta/Sta.hh"

namespace sta
{

class DatabaseStaNetwork;

using odb::dbBlock;
using odb::dbDatabase;
using odb::dbLib;
using odb::dbNet;
using odb::dbTech;

class DatabaseSta : public Sta
{
public:
    DatabaseSta();
    void init(Tcl_Interp* tcl_interp, dbDatabase* db);

    dbDatabase*
    db()
    {
        return db_;
    }
    virtual void makeComponents() override;
    DatabaseStaNetwork*
    getDbNetwork()
    {
        return db_network_;
    }

    virtual LibertyLibrary* readLiberty(const char* filename, Corner* corner,
                                        const MinMaxAll* min_max,
                                        bool infer_latches) override;

    Slack netSlack(const dbNet* net, const MinMax* min_max);
    using Sta::netSlack;

    // From ord::OpenRoad::Observer
    virtual void postReadLef(odb::dbTech* tech, odb::dbLib* library);
    virtual void postReadDef(odb::dbBlock* block);
    virtual void postReadDb(odb::dbDatabase* db);

    // Find clock nets connected by combinational gates from the clock roots.
    void findClkNets(std::set<dbNet*>& clk_nets);

protected:
    virtual void makeNetwork() override;
    virtual void makeSdcNetwork() override;

    dbDatabase*         db_;
    DatabaseStaNetwork* db_network_;
};

DatabaseSta* makeBlockSta(dbBlock* block);

} // namespace sta