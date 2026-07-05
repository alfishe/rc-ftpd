/*****************************************************************************************************
 * rc-ftpd ©2000 Robin Cloutman <rycochet2@yahoo.com>                                                *
 * --------------------------------------------------                                                *
 * MUI ftp daemon, may be split into ftpd.library, ftpd, ftpserv and ftpgui later.                   *
 *****************************************************************************************************/

// We DON'T want bloody stdio.h!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#define	_STDIO_H_

#include <sys/types.h>

#include	<ctype.h>
#include	<exec/memory.h>
#include	<string.h>
#include <libraries/gadtools.h>
#include <libraries/asl.h>
#include <libraries/mui.h>
#include	<mui/MUIundoc.h>
#include	<mui/Lamp_mcc.h>
#include	<mui/NList_mcc.h>
#include	<mui/NListview_mcc.h>
#include	<rexx/storage.h>

#include	<clib/alib_protos.h>
#include	<clib/muimaster_protos.h>

#include	<inline/exec.h>
#include	<inline/dos.h>

#include "ftpd.h"

void add_word( char *a, const char *b, size_t l )
{
	if ( *a )strncat( a, " ", l );
	strncat( a, b, l );
}

void add_number( char *a, const long n, size_t l )
{
	char b[12];
	sprintf( b, "%ld", n );
	if ( *a )strncat( a, " ", l );
	strncat( a, b, l );
}

/*
cmd:		FTPD CMD/A,ARGS/F

cmds:		FTPD OPEN <0/1>
result:	<0/1>
cmds:		FTPD ANON <0/1>
result:	<0/1>
cmds:		FTPD MAXCPS <value>
result:	<oldvalue>




cmd:		USER CMD/A,HANDLE/N/A,STEM/K,ARGS/F

cmds:		USER QUERY ALL <stem>

cmds:		USER QUERY <handle> <stem>
cmds:		USER SET <handle> <stem>
cmds:		USER ADD <handle> <stem>
stem:		stem.name = <user name>
			stem.pass = <user pass>
			stem.groups = <user groups>
			stem.max = <max login>
			stem.log = [0-4]




cmd:		GROUP CMD/A,HANDLE/N/A,STEM/K,ARGS/F

cmds:		GROUP QUERY ALL <stem>

cmds:		GROUP QUERY <handle> <stem>
cmds:		GROUP SET <handle> <stem>
cmds:		GROUP ADD <handle> <stem>
stem:		stem.alias = <alias for ftp>
			stem.path = <real path, "" means virtual>
			stem.read = <read access>>
			stem.write = <write access>
			stem.delete = <delete access>
			stem.subdirs = <subdirs access>
*/

void stem_string( struct Message *msg, char *stem, char *var, char *value )
{
	char	buffer[512], word[512];
	sprintf( buffer, "%s%s", stem, var );
	sprintf( word, "%s", value );
	SetRexxVar( msg, buffer, word, strlen(word) );
}

void stem_long( struct Message *msg, char *stem, char *var, long value )
{
	char	buffer[512], word[128];

	sprintf( word, "%ld", value );
	sprintf( buffer, "%s%s", stem, var );
	SetRexxVar( msg, buffer, word, strlen(word) );
}

long Access_Query_List( struct Message *msg, char *line )
{
	struct accessdata *access = NULL;
	char buffer[BUFSIZE], word[512], stem[512];
	long i = 0;

	*buffer = *stem = '\0';
	line = one_word( word, line );
	if ( !stricmp(word,"STEM") )
	{
		line = one_word( stem, line );
		if ( !*stem )return(10);
		if ( stem[strlen(stem)-1]!='.' )strcat( stem, "." );
	}
	for ( access = access_list ; access ; access = access->next )
	{
		if ( *stem )
		{
			sprintf( buffer, "%ld", i );
			stem_long( msg, stem, buffer, (long)access );
		}
		else add_number(buffer,(long)access,BUFSIZE);
		i++;
	}
	if ( *stem )stem_long( msg, stem, "COUNT", i );
	else set( APP_Main, MUIA_Application_RexxString, buffer );
	return(0);
}

long Access_Query( struct Message *msg, char *line, long handle )
{
	struct accessdata *access = NULL;
	char buffer[BUFSIZE], word[512], stem[512];

	*buffer = *stem = '\0';
	for ( access = access_list ; access ; access = access->next )if ( access==(struct accessdata*)handle )break;
	if ( access )
	{
		line = one_word( word, line );
		if ( !stricmp(word,"STEM") )
		{
			line = one_word( stem, line );
			if ( !*stem )return(10);
			if ( stem[strlen(stem)-1]!='.' )strcat( stem, "." );
			stem_string( msg, stem, "ALIAS", access->alias );
			stem_string( msg, stem, "PATH", access->path );
			stem_long( msg, stem, "READ", access->read );
			stem_long( msg, stem, "WRITE", access->write );
			stem_long( msg, stem, "DELETE", access->delete );
			stem_long( msg, stem, "SUBDIRS", access->subdirs );
		}
		else
		{
//			set( APP_Main, MUIA_Application_RexxString, buffer );
			return(1);
		}
	}
	else return(5);
	return(0);
}

/*
 * ACCESS CMD/A,ARGS/F
 * command:	Used for getting information about access groups.
 */
enum { access_CMD = 0,access_ARGS };
static unsigned long __saveds AREXX_Access_Func( long *args[] __asm__("a1") )
{
	struct Message *msg;
	char	word[512], *line;
	long	ret = 0;

	get( APP_Main, MUIA_Application_RexxMsg, &msg );
	line = one_word( word, (char*)args[access_ARGS] );
	if ( !stricmp((char*)args[access_CMD],"QUERY") )
	{
		if ( !stricmp(word,"ALL") )					ret = Access_Query_List( msg, line );
		else if ( word[0]>='1' && word[0]<='9' )	ret = Access_Query( msg, line, strlong(word) );
		else ret = 10;
	}
	return(ret);
}
struct Hook AREXX_Access_Hook = { {NULL,NULL},&AREXX_Access_Func,NULL,NULL };

long User_Query_List( struct Message *msg, char *line )
{
	struct userdata *user = NULL;
	char buffer[BUFSIZE], word[512], stem[512];
	long i = 0;

	*buffer = *stem = '\0';
	line = one_word( word, line );
	if ( !stricmp(word,"STEM") )
	{
		line = one_word( stem, line );
		if ( !*stem )return(10);
		if ( stem[strlen(stem)-1]!='.' )strcat( stem, "." );
	}
	for ( user = user_list ; user ; user = user->next )
	{
		if ( *stem )
		{
			sprintf( buffer, "%ld", i );
			stem_long( msg, stem, buffer, (long)user );
		}
		else add_number(buffer,(long)user,BUFSIZE);
		i++;
	}
	if ( *stem )stem_long( msg, stem, "COUNT", i );
	else set( APP_Main, MUIA_Application_RexxString, buffer );
	return(0);
}

long User_Query( struct Message *msg, char *line, long handle )
{
	struct userdata *user = NULL;
	char buffer[BUFSIZE], word[512], stem[512];

	*buffer = *stem = '\0';
	for ( user = user_list ; user ; user = user->next )if ( user==(struct userdata*)handle )break;
	if ( user )
	{
		line = one_word( word, line );
		if ( !stricmp(word,"STEM") )
		{
			line = one_word( stem, line );
			if ( !*stem )return(10);
			if ( stem[strlen(stem)-1]!='.' )strcat( stem, "." );
			stem_string( msg, stem, "NAME", user->user );
			stem_string( msg, stem, "PASS", user->pass );
			stem_long( msg, stem, "ACCESS", user->access );
			stem_long( msg, stem, "MAX", user->sessions );
			stem_long( msg, stem, "LOG", user->loglevel );
		}
		else
		{
//			set( APP_Main, MUIA_Application_RexxString, buffer );
			return(1);
		}
	}
	else return(5);
	return(0);
}

/*
 * USER CMD/A,ARGS/F
 * command:	Used for getting information about accounts.
 */
enum { user_CMD = 0,user_ARGS };
static unsigned long __saveds AREXX_User_Func( long *args[] __asm__("a1") )
{
	struct Message *msg;
	char	word[512], *line;
	long	ret = 0;

	get( APP_Main, MUIA_Application_RexxMsg, &msg );
	line = one_word( word, (char*)args[user_ARGS] );
	if ( !stricmp((char*)args[user_CMD],"QUERY") )
	{
		if ( !stricmp(word,"ALL") )					ret = User_Query_List( msg, line );
		else if ( word[0]>='1' && word[0]<='9' )	ret = User_Query( msg, line, strlong(word) );
		else ret = 10;
	}
	return(ret);
}
struct Hook AREXX_User_Hook = { {NULL,NULL},&AREXX_User_Func,NULL,NULL };

long Ftp_Query_List( struct Message *msg, char *line, int type )
{
	struct ftpdata *ftp = NULL;
	char buffer[BUFSIZE], word[512], stem[512];
	long i = 0;

	*buffer = *stem = '\0';
	line = one_word( word, line );
	if ( !stricmp(word,"STEM") )
	{
		line = one_word( stem, line );
		if ( !*stem )return(10);
		if ( stem[strlen(stem)-1]!='.' )strcat( stem, "." );
	}
	for ( ftp = ftp_list ; ftp ; ftp = ftp->next )
	{
		if ( type==1 && !ftp->file )continue;
		if ( *stem )
		{
			sprintf( buffer, "%ld", i );
			stem_long( msg, stem, buffer, (long)ftp );
		}
		else add_number(buffer,(long)ftp,BUFSIZE);
		i++;
	}
	if ( *stem )stem_long( msg, stem, "COUNT", i );
	else set( APP_Main, MUIA_Application_RexxString, buffer );
	return(0);
}

long Ftp_Query( struct Message *msg, char *line, long handle )
{
	struct ftpdata *ftp = NULL;
	char buffer[BUFSIZE], word[512], stem[512];

	*buffer = *stem = '\0';
	for ( ftp = ftp_list ; ftp ; ftp = ftp->next )if ( ftp==(struct ftpdata*)handle )break;
	if ( ftp )
	{
		line = one_word( word, line );
		if ( !stricmp(word,"STEM") )
		{
			line = one_word( stem, line );
			if ( !*stem )return(10);
			if ( stem[strlen(stem)-1]!='.' )strcat( stem, "." );
			stem_string( msg, stem, "IP", ftp->ip );
			stem_string( msg, stem, "FILE", ftp->file?ftp->filename:"" );
			stem_string( msg, stem, "ALIAS", ftp->path );
			stem_string( msg, stem, "PATH", ftp->cd );
			stem_long( msg, stem, "USER", (long)ftp->user );
			stem_long( msg, stem, "ACCESS", (long)ftp->access );
			stem_long( msg, stem, "CPS", ftp->file?ftp->cps:0 );
			stem_long( msg, stem, "SIZE", ftp->file?ftp->size:0 );
			stem_long( msg, stem, "CURRENT", ftp->file?ftp->current:0 );
			stem_long( msg, stem, "DONE", (ftp->file&&ftp->size&&ftp->cps)?(ftp->size-ftp->current)/ftp->cps:0 );
			stem_long( msg, stem, "LOG", (long)ftp->loglevel );
		}
		else
		{
//			set( APP_Main, MUIA_Application_RexxString, buffer );
			return(1);
		}
	}
	else return(5);
	return(0);
}

extern struct Hook MAIN_Kick_Hook;
extern struct Hook MAIN_Abort_Hook;

/*
 * FTP CMD/A,ARGS/F
 * command:	Used for getting information about online ppl.
 */
enum { ftp_CMD = 0,ftp_ARGS };
static unsigned long __saveds AREXX_Ftp_Func( long *args[] __asm__("a1") )
{
	struct Message *msg;
	char	word[512], *line;
	long	ret = 0;

	get( APP_Main, MUIA_Application_RexxMsg, &msg );
	line = one_word( word, (char*)args[ftp_ARGS] );
	if ( !stricmp((char*)args[ftp_CMD],"QUERY") )
	{
		if ( !stricmp(word,"ALL") )			ret = Ftp_Query_List( msg, line, 0 );
		else if ( !stricmp(word,"XFER") )	ret = Ftp_Query_List( msg, line, 1 );
		else if ( word[0]>='1' && word[0]<='9' )ret = Ftp_Query( msg, line, strlong(word) );
		else ret = 10;
	}
	else
	{
		struct ftpdata *ftp = NULL;
		long handle = strlong(word);

		for ( ftp = ftp_list ; ftp ; ftp = ftp->next )if ( ftp==(struct ftpdata*)handle )break;
		if ( ftp )
		{
			if ( !stricmp((char*)args[ftp_CMD],"ABORT") )			DoMethod( APP_Main, MUIM_CallHook, &MAIN_Abort_Hook, ftp );
			else if ( !stricmp((char*)args[ftp_CMD],"KICK") )		DoMethod( APP_Main, MUIM_CallHook, &MAIN_Kick_Hook, ftp );
			else if ( !stricmp((char*)args[ftp_CMD],"MESSAGE") )
			{
				if ( *line )
				{
					output( ftp, "*** Message from console ***", 0 );
					output( ftp, line, 0 );
				}
			}
			else if ( !stricmp((char*)args[ftp_CMD],"LOG") )
			{
				sprintf( word, "%ld", (long)ftp->loglevel );
				if ( *line>='0' && *line<='4' && *line!=*word )
				{
					ftp->loglevel = *line - '0';
					start_log( ftp );
				}
				set( APP_Main, MUIA_Application_RexxString, word );
			}
		}
	}
	return(ret);
}
struct Hook AREXX_Ftp_Hook = { {NULL,NULL},&AREXX_Ftp_Func,NULL,NULL };

/*
 * VERSION
 * command: Returns the current program version.
 * result:	<ver>.<rev>
 */
static unsigned long __saveds AREXX_Version_Func( long *args[] __asm__("a1") )
{
	set( APP_Main, MUIA_Application_RexxString, shortversion );
	return(0);
}
struct Hook AREXX_Version_Hook = { {NULL,NULL},&AREXX_Version_Func,NULL,NULL };


/*
 * STATS
 * command:	Used for getting the stats
 */
enum { stats_ARGS=0 };
static unsigned long __saveds AREXX_Stats_Func( long *args[] __asm__("a1") )
{
	struct Message *msg;
	char	word[512], *line;
	long	page = stats_page, ret = 0;

	get( APP_Main, MUIA_Application_RexxMsg, &msg );
	line = (char*)args[stats_ARGS];
	if ( *line>='1' && *line<='3' )page = *line++ - '1';
	line = one_word( word, line );
	if ( !stricmp(word,"STEM") )
	{
		char stem[512];
		unsigned long cps;

		line = one_word( stem, line );
		if ( !*stem )return(10);
		if ( stem[strlen(stem)-1]!='.' )strcat( stem, "." );
		stem_string( msg, stem, "DATASENT",		add_comma(stats[page].fs_sent_data,TRUE) );
		stem_string( msg, stem, "DATARECV",		add_comma(stats[page].fs_recv_data,TRUE) );
		stem_string( msg, stem, "DATATOTAL",	add_comma(stats[page].fs_sent_data + stats[page].fs_recv_data,TRUE) );

		stem_string( msg, stem, "FILESENT",		add_comma(stats[page].fs_sent_files,FALSE) );
		stem_string( msg, stem, "FILERECV",		add_comma(stats[page].fs_recv_files,FALSE) );
		stem_string( msg, stem, "FILETOTAL",	add_comma(stats[page].fs_sent_files + stats[page].fs_recv_files,FALSE) );

		if ( stats[page].fs_sent_cps && stats[page].fs_recv_cps )cps = ( stats[page].fs_sent_cps + stats[page].fs_recv_cps ) / 2;
		else if ( stats[page].fs_sent_files )cps = stats[page].fs_sent_cps;
		else cps = stats[page].fs_recv_cps;
		stem_string( msg, stem, "CPSSENT",		add_comma(stats[page].fs_sent_cps,FALSE) );
		stem_string( msg, stem, "CPSRECV",		add_comma(stats[page].fs_recv_cps,FALSE) );
		stem_string( msg, stem, "CPSTOTAL",		add_comma(cps,FALSE) );

		stem_string( msg, stem, "USERACCOUNT",	add_comma(stats[page].fs_users_account,FALSE) );
		stem_string( msg, stem, "USERANON",		add_comma(stats[page].fs_users_anon,FALSE) );
		stem_string( msg, stem, "USERTOTAL",	add_comma(stats[page].fs_users_account + stats[page].fs_users_anon,FALSE) );
		return(0);
	}
	if ( !stricmp(word,"RESET") )
	{
		memclr( &stats[page], sizeof(struct ftpd_stat) );
		save_stats( "PROGDIR:ftpd.stats" );
		update_stats();
		return(0);
	}
	else
	{
//		set( APP_Main, MUIA_Application_RexxString, buffer );
		return(1);
	}
	return(ret);
}
struct Hook AREXX_Stats_Hook = { {NULL,NULL},&AREXX_Stats_Func,NULL,NULL };

/*
 * STATUS
 * command:	Used for set/getting the online status
 */
enum { status_ARGS=0 };
static unsigned long __saveds AREXX_Status_Func( long *args[] __asm__("a1") )
{
	unsigned long status;
	struct Message *msg;

	get( APP_Main, MUIA_Application_RexxMsg, &msg );
	if ( args[status_ARGS] )
	{
		if ( !stricmp((char*)args[status_ARGS],"OPEN") )			set( MAIN_Open, MUIA_Cycle_Active, 1 );
		else if ( !stricmp((char*)args[status_ARGS],"CLOSED") )	set( MAIN_Open, MUIA_Cycle_Active, 0 );
	}
	get( MAIN_Open, MUIA_Cycle_Active, &status );
	set( APP_Main, MUIA_Application_RexxString, status?"OPEN":"CLOSED" );
	return(0);
}
struct Hook AREXX_Status_Hook = { {NULL,NULL},&AREXX_Status_Func,NULL,NULL };



struct MUI_Command mui_commands[] =
{
	{ "VERSION",	NULL,					0,	&AREXX_Version_Hook },
	{ "FTP",			"CMD/A,ARGS/F",	2,	&AREXX_Ftp_Hook },
	{ "USER",		"CMD/A,ARGS/F",	2,	&AREXX_User_Hook },
	{ "ACCESS",		"CMD/A,ARGS/F",	2,	&AREXX_Access_Hook },
	{ "STATS",		"ARGS/F",			1,	&AREXX_Stats_Hook },
	{ "STATUS",		"ARGS/F",			1,	&AREXX_Status_Hook },
	{ NULL, NULL, 0, NULL }
};


