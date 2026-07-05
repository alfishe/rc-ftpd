/*
 * OpenLibrary( bsdsocket.library )
 * ReleaseSocket( 0, 0 )
 * CloseLibrary( bsdsocket.library )
 * OpenLibrary( rc.ftpd )
 * ObtainFTPdSocket( ? )
 * CloseLibrary( rc.ftpd )
 */

#include	<exec/exec.h>
#include	<exec/execbase.h>
#include	<dos/dos.h>
#include	<dos/dosextens.h>
//#include	<sys/socket.h>
#include	<inline/exec.h>
#include	<inline/dos.h>
#include	<inline/socket.h>
#include	<inetd.h>

// gcc stuff
char __nocommandline = 1; /* Disable commandline parsing  */
char __initlibraries = 0; /* Disable auto-library-opening */

int main(void)
{
	struct ExecBase *SysBase	= NULL;
	struct Library *SocketBase	= NULL;
	struct DaemonMessage *dm	= NULL;

	SysBase = (*((struct ExecBase **)4));
	if ( !( dm = (struct DaemonMessage*)((struct Process*)(SysBase->ThisTask))->pr_ExitData ) )
	{
// Start as Server
		struct Library *DosBase		= NULL;

		if ( ( DosBase = OpenLibrary( "dos.library", 37 ) ) )
		{
			VPrintf( "ftpd must be launched by InetD!\n", NULL );
			CloseLibrary( DosBase );
		}
		return( RETURN_WARN );
// End as Server
	}
	else if ( ( SocketBase = OpenLibrary( "bsdsocket.library", 4 ) ) )
	{
		long	s;

		if ( ( s = ObtainSocket( dm->dm_Id, dm->dm_Family, dm->dm_Type, 0 ) ) >= 0 )
		{
// Start as Daemon
			CloseSocket( s );
// End as Daemon
		}
		else dm->dm_Retval = DERR_OBTAIN;
		CloseLibrary( SocketBase );
	}
	else dm->dm_Retval = DERR_LIB;
	return( RETURN_OK );
}

void __chkabort(void){}
