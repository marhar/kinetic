//----------------------------------------------------------------------
// cppdemo.cc -- demonstrate C++ Tcl commands
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
}

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
    int methodname##ObjMethod(Tcl_Interp* interp, int objc, Tcl_Obj**objv); \
    public: \
    static int methodname##ObjCmd(ClientData cd, Tcl_Interp* interp, \
                         int objc, Tcl_Obj* const * objv) { \
        classname* myobj = (classname*)cd; \
        return myobj->methodname##ObjMethod(interp, objc, objv); \
    }

////////////////////////////////////////////////////////////////
// class DemoClass -- demonstrate C++ Tcl command dispatching
////////////////////////////////////////////////////////////////

class DemoClass {
public:
    Tcl_Method(DemoClass, echo);
    Tcl_ObjMethod(DemoClass, echo2);
    DemoClass(Tcl_Interp* interp) {
        ClientData cd = (ClientData)this;
        Tcl_CreateCommand(interp, "echo", echoCmd, cd, 0);
        Tcl_CreateObjCommand(interp, "echo2", echo2ObjCmd, cd, 0);
    }
};

int DemoClass::echoMethod(Tcl_Interp* interp, int argc, char**argv)
{
    int i;
    char tmpstr[20];

    Tcl_ResetResult(interp);
    sprintf(tmpstr, "%d", argc);
    Tcl_AppendResult(interp, "argc = ", tmpstr, NULL);
    for (i = 0; i < argc; ++i) {
        sprintf(tmpstr, "%d", i);
        Tcl_AppendResult(interp, "\n    argv[", tmpstr, "] = ", argv[i], NULL);
    }

    return TCL_OK;
}

int DemoClass::echo2ObjMethod(Tcl_Interp* interp, int objc, Tcl_Obj** objv)
{
    int i;
    char tmpstr[20];

    Tcl_ResetResult(interp);
    sprintf(tmpstr, "%d", objc);
    Tcl_AppendResult(interp, "argc = ", tmpstr, NULL);
    for (i = 0; i < objc; ++i) {
        sprintf(tmpstr, "%d", i);
        Tcl_AppendResult(interp, "\n    objv[", tmpstr, "] = ",
                Tcl_GetStringFromObj(objv[i], NULL), NULL);
    }

    return TCL_OK;
}

int myappinit(Tcl_Interp* interp)
{
    // what about freeing?
    DemoClass locker(interp);
    return TCL_OK;
}

int main(int argc, char** argv)
{
    Tcl_Main(argc, argv, myappinit);
}
