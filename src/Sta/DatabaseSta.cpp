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
#ifndef OPENROAD_BUILD

#include <OpenPhySyn/Sta/DatabaseSta.hpp>
#include <OpenPhySyn/Sta/DatabaseStaNetwork.hpp>
#include "Machine.hh"
#include "Sta/DatabaseSdcNetwork.hpp"
#include "opendb/db.h"

namespace sta
{

DatabaseSta::DatabaseSta(dbDatabase* db) : Sta(), db_(db)
{
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
DatabaseSta::readLefAfter(dbLib* lib)
{
    db_network_->readLefAfter(lib);
}

void
DatabaseSta::readDefAfter()
{
    db_network_->readDefAfter();
}

void
DatabaseSta::readDbAfter()
{
    db_network_->readDbAfter();
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
} // namespace sta

#endif