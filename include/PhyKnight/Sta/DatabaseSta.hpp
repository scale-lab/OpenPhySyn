// StaDb, OpenSTA on OpenDB
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

#ifndef __PHY_DATABASE_STA_H__
#define __PHY_DATABASE_STA_H__

// Temproary fix for OpenSTA
#define THROW_DCL throw()

#include "Sta.hh"
#include "opendb/db.h"

namespace sta
{

class DatabaseStaNetwork;

using odb::dbDatabase;

class DatabaseSta : public Sta
{
public:
    DatabaseSta(dbDatabase* db);
    dbDatabase*
    db()
    {
        return db_;
    }
    virtual void            makeComponents();
    DatabaseStaNetwork*     getDatabaseStaNetwork();
    void                    readDbAfter();
    virtual LibertyLibrary* readLiberty(const char* filename, Corner* corner,
                                        const MinMaxAll* min_max,
                                        bool             infer_latches);

protected:
    virtual void makeNetwork();
    virtual void makeSdcNetwork();

    dbDatabase* db_;
};

} // namespace sta
#endif
