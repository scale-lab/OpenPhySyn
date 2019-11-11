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

#include <PhyKnight/Sta/DatabaseSta.hpp>
#include <PhyKnight/Sta/DatabaseStaNetwork.hpp>
#include "DatabaseSdcNetwork.hpp"
#include "Machine.hh"
#include "opendb/db.h"

namespace sta
{

DatabaseSta::DatabaseSta(dbDatabase* db) : Sta(), db_(db)
{
}

DatabaseStaNetwork*
DatabaseSta::getDatabaseStaNetwork()
{
    return dynamic_cast<class DatabaseStaNetwork*>(network_);
}

// Wrapper to init network db.
void
DatabaseSta::makeComponents()
{
    Sta::makeComponents();
    getDatabaseStaNetwork()->setDb(db_);
}

void
DatabaseSta::makeNetwork()
{
    network_ = new class DatabaseStaNetwork();
}

void
DatabaseSta::makeSdcNetwork()
{
    sdc_network_ = new DatabaseSdcNetwork(network_);
}

void
DatabaseSta::readDbAfter()
{
    getDatabaseStaNetwork()->readDbAfter();
}

// Wrapper to sync db/liberty libraries.
LibertyLibrary*
DatabaseSta::readLiberty(const char* filename, Corner* corner,
                         const MinMaxAll* min_max, bool infer_latches)

{
    LibertyLibrary* lib =
        Sta::readLiberty(filename, corner, min_max, infer_latches);
    getDatabaseStaNetwork()->readLibertyAfter(lib);
    return lib;
}

} // namespace sta
