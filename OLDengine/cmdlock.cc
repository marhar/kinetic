//----------------------------------------------------------------------
// cmdlock.cc -- lock commands to prevent them from changing
//----------------------------------------------------------------------
// This file is part of KAP, the Kinetic Application Processor.
// Copyright (c) 1999, AsiaInfo Technologies, Inc.
//
// See the file kap-license.txt for information on usage and
// redistribution of this file, and for a DISCLAIMER OF ALL
// WARRANTIES.  Share and Enjoy!
//
// The KAP home page is: http://www.markharrison.net/kap
//----------------------------------------------------------------------

#include "cmdlock.h"
#include <string.h>

////////////////////////////////////////////////////////////////
// islocked -- tell if a command is locked
//
// I'm sorry this logic is so confusing.  Basically, it
// locks commands but not procedures.  "info commands"
// returns procedures, so we must check to see if
//     name is a command AND and name is not a procedure
////////////////////////////////////////////////////////////////

int CmdLock::islocked(Tcl_Interp* interp, char* name)
{
    int rc = 0;
    Tcl_DString isproc;
    Tcl_DStringInit(&isproc);
    Tcl_DStringAppend(&isproc, "info procs ", -1);
    Tcl_DStringAppend(&isproc, name, -1);

    Tcl_DString iscmd;
    Tcl_DStringInit(&iscmd);
    Tcl_DStringAppend(&iscmd, "info commands ", -1);
    Tcl_DStringAppend(&iscmd, name, -1);

    Tcl_Eval(interp, Tcl_DStringValue(&iscmd));
    if (strcmp(interp->result, name) == 0) {
        Tcl_Eval(interp, Tcl_DStringValue(&isproc));
        if (strcmp(interp->result, "") == 0) {
            rc = 1;
        }
    }
    Tcl_DStringFree(&isproc);
    Tcl_DStringFree(&iscmd);
    // leave interpreter clean
    Tcl_ResetResult(interp);
    return rc;
}

////////////////////////////////////////////////////////////////
// renameObjMethod -- disallow renaming of existing commands
////////////////////////////////////////////////////////////////

int CmdLock::renameObjMethod(Tcl_Interp* interp, int objc, Tcl_Obj*const* objv)
{
    if (objc != 3) {
        Tcl_WrongNumArgs(interp, 1, objv, "oldName newName");
        return TCL_ERROR;
    }
    char* cmd = Tcl_GetStringFromObj(objv[1], NULL);
    if (islocked(interp, cmd)) {
        Tcl_ResetResult(interp);
        Tcl_AppendResult(interp, cmd, " is locked and cannot be renamed", NULL);
        return TCL_ERROR;
    }
    return Tcl_RenameObjCmd(0, interp, objc, objv);
}

////////////////////////////////////////////////////////////////
// procObjMethod -- disallow renaming of existing commands
////////////////////////////////////////////////////////////////

int CmdLock::procObjMethod(Tcl_Interp* interp, int objc, Tcl_Obj*const* objv)
{
    if (objc != 4) {
        Tcl_WrongNumArgs(interp, 1, objv, "name args body");
        return TCL_ERROR;
    }
    char* cmd = Tcl_GetStringFromObj(objv[1], NULL);
    if (islocked(interp, cmd)) {
        Tcl_ResetResult(interp);
        Tcl_AppendResult(interp, cmd, " is locked and cannot be renamed", NULL);
        return TCL_ERROR;
    }
    return Tcl_ProcObjCmd(0, interp, objc, objv);
}

// eof: cmdlock.cc
