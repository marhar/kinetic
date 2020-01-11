//----------------------------------------------------------------------
// class CmdLock -- disallow override for all existing commands
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


extern "C" {
#include "tcl.h"
extern int Tcl_RenameObjCmd(ClientData, Tcl_Interp *,
                            int objc, Tcl_Obj *CONST objv[]);
extern int Tcl_ProcObjCmd(ClientData, Tcl_Interp *, int objc,
                            Tcl_Obj *CONST objv[]);
}

////////////////////////////////////////////////////////////////
// These definitions reduce some code for declaring C++ cmds
////////////////////////////////////////////////////////////////

#define Tcl_Method(classname, methodname) \
    private: \
    int methodname##Method(Tcl_Interp* interp, int argc, char**argv); \
    public: \
    static int methodname##Cmd(ClientData cd, Tcl_Interp* interp, \
                         int argc, char** argv) { \
        classname* myobj = (classname*)cd; \
        return myobj->methodname##Method(interp, argc, argv); \
    }

#define Tcl_ObjMethod(classname, methodname) \
    private: \
    int methodname##ObjMethod(Tcl_Interp* interp, int objc, Tcl_Obj*const*objv); \
    public: \
    static int methodname##ObjCmd(ClientData cd, Tcl_Interp* interp, \
                         int objc, Tcl_Obj*const* objv) { \
        classname* myobj = (classname*)cd; \
        return myobj->methodname##ObjMethod(interp, objc, objv); \
    }

////////////////////////////////////////////////////////////////
// class CmdLock -- disallow override for all existing commands
////////////////////////////////////////////////////////////////

class CmdLock {
private:
    int islocked(Tcl_Interp* interp, char* cmdname);

public:
    Tcl_ObjMethod(CmdLock, rename);
    Tcl_ObjMethod(CmdLock, proc);
    CmdLock(Tcl_Interp* interp) {
        ClientData cd = (ClientData)this;
        Tcl_CreateObjCommand(interp, "rename", renameObjCmd, cd, 0);
        Tcl_CreateObjCommand(interp, "proc", procObjCmd, cd, 0);
    }
};

// eof: cmdlock.h
