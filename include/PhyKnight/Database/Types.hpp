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

#ifndef __PHY_TYPES__
#define __PHY_TYPES__

// Temproary fix for OpenSTA
#define THROW_DCL throw()

#include <OpenSTA/liberty/Liberty.hh>
#include <opendb/db.h>
#include <opendb/dbTypes.h>
#include <opendb/defin.h>
#include <opendb/defout.h>
#include <opendb/lefin.h>

namespace phy
{
class OpenDBHelper;
#ifdef USE_OPENDB_DB_HELPER
typedef odb::dbDatabase     Database;
typedef odb::dbChip         Chip;
typedef odb::dbBlock        Block;
typedef odb::dbInst         Instance;
typedef odb::dbITerm        InstanceTerm; // Instance pin
typedef odb::dbMTerm        LibraryTerm;  // Library pin
typedef odb::dbBTerm        BlockTerm;
typedef odb::dbMaster       LibraryCell;
typedef odb::dbLib          Library;
typedef odb::dbTech         LibraryTechnology;
typedef odb::dbNet          Net;
typedef odb::defin          DefParser;
typedef odb::defout         DefOut;
typedef odb::lefin          LefParser;
typedef sta::LibertyLibrary Liberty;

typedef odb::dbSet<Library>      LibrarySet;
typedef odb::dbSet<Net>          NetSet;
typedef odb::dbSet<BlockTerm>    BlockTermSet;
typedef odb::dbSet<InstanceTerm> InstanceTermSet;
typedef odb::dbIoType::Value     PinDirection;
typedef OpenDBHelper             DatabaseHelper;
#else
// Default is OpenSTA helper
typedef odb::dbDatabase     Database;
typedef odb::dbChip         Chip;
typedef odb::dbBlock        Block;
typedef odb::dbInst         Instance;
typedef odb::dbITerm        InstanceTerm; // Instance pin
typedef odb::dbMTerm        LibraryTerm;  // Library pin
typedef odb::dbBTerm        BlockTerm;
typedef odb::dbMaster       LibraryCell;
typedef odb::dbLib          Library;
typedef odb::dbTech         LibraryTechnology;
typedef odb::dbNet          Net;
typedef odb::defin          DefParser;
typedef odb::defout         DefOut;
typedef odb::lefin          LefParser;
typedef sta::LibertyLibrary Liberty;

typedef odb::dbSet<Library>      LibrarySet;
typedef odb::dbSet<Net>          NetSet;
typedef odb::dbSet<BlockTerm>    BlockTermSet;
typedef odb::dbSet<InstanceTerm> InstanceTermSet;
typedef odb::dbIoType::Value     PinDirection;
#endif

} // namespace phy
#endif