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

#include <PsnLogger/PsnLogger.hpp>
#include <tcl.h>
#include "Psn/Psn.hpp"
#include "PsnException/ProgramOptionsException.hpp"

extern "C"
{
    extern int Psn_Init(Tcl_Interp* interp);
}

int psnTclAppInit(Tcl_Interp* interp);

int
main(int argc, char* argv[])
{
    try
    {
        psn::Psn::instance().setProgramOptions(argc, argv);
        if (psn::Psn::instance().programOptions().help())
        {
            psn::Psn::instance().printUsage(true, false, false);
            psn::Psn::instance().setupInterpreter(Tcl_CreateInterp());
            return 0;
        }
        if (psn::Psn::instance().programOptions().version())
        {
            psn::Psn::instance().printVersion(true);
            psn::Psn::instance().setupInterpreter(Tcl_CreateInterp());
            return 0;
        }
    }
    catch (psn::ProgramOptionsException& e)
    {
        psn::PsnLogger::instance().error(e.what());
        return -1;
    }

    Tcl_Main(1, argv, psnTclAppInit);
    return 0;
}

int
psnTclAppInit(Tcl_Interp* interp)
{
    if (Tcl_Init(interp) == TCL_ERROR)
    {
        return TCL_ERROR;
    }

    if (Psn_Init(interp) == TCL_ERROR)
    {
        return TCL_ERROR;
    }
    psn::Psn::instance().loadTransforms();
    psn::Psn::instance().setupInterpreter(interp);
    psn::Psn::instance().processStartupProgramOptions();
    psn::Psn::instance().setupInterpreterReadline(); // This should be that last
                                                     // initialization step
                                                     // because of the RL loop
    return TCL_OK;
}
