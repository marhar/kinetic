/*----------------------------------------------------------------*/
/* chdir.c -- directory changing functions                        */
/* copied from tclUnixFile.c, version 8.0p2 in order to provide a */
/* standard interface for KAP.  The parameters changed in 8.2, so */
/* it's easier just to carry this along.                          */
/*----------------------------------------------------------------*/

#include <errno.h>
#include <unistd.h>

#include "tcl.h"

#ifndef MAXPATHLEN
#   ifdef PATH_MAX
#       define MAXPATHLEN PATH_MAX
#   else
#       define MAXPATHLEN 2048
#   endif
#endif

static char *currentDir =  NULL;
static int currentDirExitHandlerSet = 0;

/*
 *----------------------------------------------------------------------
 *
 * FreeCurrentDir --
 *
 *	Frees the string stored in the currentDir variable. This routine
 *	is registered as an exit handler and will be called during shutdown.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Frees the memory occuppied by the currentDir value.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
static void
FreeCurrentDir(clientData)
    ClientData clientData;	/* Not used. */
{
    if (currentDir != (char *) NULL) {
        ckfree(currentDir);
        currentDir = (char *) NULL;
        currentDirExitHandlerSet = 0;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * KapChdir --
 *
 *	Change the current working directory.
 *
 * Results:
 *	The result is a standard Tcl result.  If an error occurs and 
 *	interp isn't NULL, an error message is left in interp->result.
 *
 * Side effects:
 *	The working directory for this application is changed.  Also
 *	the cache maintained used by KapGetCwd is deallocated and
 *	set to NULL.
 *
 *----------------------------------------------------------------------
 */

int
KapChdir(interp, dirName)
    Tcl_Interp *interp;		/* If non NULL, used for error reporting. */
    char *dirName;     		/* Path to new working directory. */
{
    if (currentDir != NULL) {
	free(currentDir);
	currentDir = NULL;
    }
    if (chdir(dirName) != 0) {
	if (interp != NULL) {
	    Tcl_AppendResult(interp, "couldn't change working directory to \"",
		    dirName, "\": ", Tcl_PosixError(interp), (char *) NULL);
	}
	return TCL_ERROR;
    }
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * KapGetCwd --
 *
 *	Return the path name of the current working directory.
 *
 * Results:
 *	The result is the full path name of the current working
 *	directory, or NULL if an error occurred while figuring it out.
 *	The returned string is owned by the KapGetCwd routine and must
 *	not be freed by the caller.  If an error occurs and interp
 *	isn't NULL, an error message is left in interp->result.
 *
 * Side effects:
 *	The path name is cached to avoid having to recompute it
 *	on future calls;  if it is already cached, the cached
 *	value is returned.
 *
 *----------------------------------------------------------------------
 */

char *
KapGetCwd(interp)
    Tcl_Interp *interp;		/* If non NULL, used for error reporting. */
{
    char buffer[MAXPATHLEN+1];

    if (currentDir == NULL) {
        if (!currentDirExitHandlerSet) {
            currentDirExitHandlerSet = 1;
            Tcl_CreateExitHandler(FreeCurrentDir, (ClientData) NULL);
        }
#ifdef USEGETWD
	if ((int)getwd(buffer) == (int)NULL) {
	    if (interp != NULL) {
		Tcl_AppendResult(interp,
			"error getting working directory name: ",
			buffer, (char *)NULL);
	    }
	    return NULL;
	}
#else
	if (getcwd(buffer, MAXPATHLEN+1) == NULL) {
	    if (interp != NULL) {
		if (errno == ERANGE) {
		    Tcl_SetResult(interp,
			    "working directory name is too long",
		            TCL_STATIC);
		} else {
		    Tcl_AppendResult(interp,
			    "error getting working directory name: ",
			    Tcl_PosixError(interp), (char *) NULL);
		}
	    }
	    return NULL;
	}
#endif
	currentDir = (char *) malloc((unsigned) (strlen(buffer) + 1));
	strcpy(currentDir, buffer);
    }
    return currentDir;
}

#ifdef TESTMAIN

char *d[] = { "/", "/etc", "/tmp", "/usr/bin", "/sbin", "/root", "/nowhere"};
int nd = sizeof(d)/sizeof(d[0]);

main()
{
    Tcl_Interp* interp;
    int i;
    int rc;

    interp = Tcl_CreateInterp();

    for (i = 0; i < nd; ++i) {
        Tcl_ResetResult(interp);
        rc = KapChdir(interp, d[i]);
        if (rc == TCL_OK)
            printf("dir=%s\n", KapGetCwd(interp));
        else
            printf("error: %s\n", Tcl_GetStringResult(interp));
    }

    for (i = 0; i < nd; ++i) {
        rc = KapChdir(NULL, d[i]);
        if (rc == TCL_OK)
            printf("dir=%s\n", KapGetCwd(interp));
        else
            printf("error: %d\n", errno);
    }
}

#endif

