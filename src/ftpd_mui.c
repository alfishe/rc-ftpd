/*****************************************************************************************************
 * rc-ftpd ©2000 Robin Cloutman <rycochet2@yahoo.com>                                                *
 * --------------------------------------------------                                                *
 * MUI ftp daemon, may be split into ftpd.library, ftpd, ftpserv and ftpgui later.                   *
 *****************************************************************************************************/

// We DON'T want bloody stdio.h!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#define	_STDIO_H_

#include <sys/types.h>

#include	<ctype.h>
#include	<dos/dos.h>
#include	<dos/exall.h>
#include	<dos/datetime.h>
#include	<dos/dostags.h>
#include	<dos/dosextens.h>
#include	<exec/memory.h>
#include	<netdb.h>
#include	<netinet/in.h>
#include	<string.h>
#include	<sys/socket.h>
#include	<sys/time.h>
#include	<sys/filio.h>
#include <libraries/gadtools.h>
#include <libraries/asl.h>
#include <libraries/mui.h>
#include <libraries/locale.h>
#include	<mui/MUIundoc.h>
#include	<mui/Lamp_mcc.h>
#include	<mui/NList_mcc.h>
#include	<mui/NListview_mcc.h>
#include	<mui/BetterBalance_mcc.h>
#include	<workbench/icon.h>
#include	<workbench/startup.h>
#include	<workbench/workbench.h>

#include	<clib/alib_protos.h>
#include	<clib/muimaster_protos.h>

#include	<inetd.h>

#include	<inline/exec.h>
#include	<inline/socket.h>
#include	<inline/dos.h>
#include	<inline/icon.h>
#include	<inline/wb.h>
#include	<inline/intuition.h>

#include	<utility/taglist.h>
#include	<inline/taglist.h>
struct Library *TagListBase;

#include "ftpd.h"

#define GroupFrameT2(s)   MUIA_Frame, MUIV_Frame_Group, MUIA_FrameTitle, s, MUIA_Background, MUII_TextBack

Object	*APP_Main;
Object	*WIN_Main;
Object	*MAIN_Prefs;
Object	*MAIN_List;
Object	*MAIN_Display;
Object	*MAIN_CPS;
Object	*MAIN_Kick;
Object	*MAIN_Abort;
Object	*MAIN_Log;
Object	*MAIN_Message;
Object	*MAIN_Open;
Object	*MAIN_ServerGroup;
Object	*MAIN_UserGroup;
Object	*MAIN_StatsGroup;
Object	*MAIN_StatsPage;
Object	*MAIN_StatsReset;
Object	*MAIN_MainGroup;
Object	*MAIN_Balance;

Object	*MENU_Users;
Object	*MENU_Stats;

Object	*STATS_Data_Sent;
Object	*STATS_Data_Recv;
Object	*STATS_Data_Total;
Object	*STATS_Files_Sent;
Object	*STATS_Files_Recv;
Object	*STATS_Files_Total;
Object	*STATS_CPS_Sent;
Object	*STATS_CPS_Recv;
Object	*STATS_CPS_Total;
Object	*STATS_User_Account;
Object	*STATS_User_Anon;
Object	*STATS_User_Total;
Object	*STATS_Reset;

char	*groups[]		= { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F" };
char	*ftpd_log[5];
char	*closedopen[3];
char	*page[4];

Object *SmallText( char *text __asm__("a0") )
{
	return( TextObject,
		MUIA_Text_Contents, text,
		MUIA_Weight, 0,
	End );
}

Object *LeftText( char *text __asm__("a0") )
{
	return( TextObject,
		MUIA_Text_PreParse, "\33l",
		MUIA_Text_Contents, text,
	End );
}

Object *RightText( char *text __asm__("a0") )
{
	return( TextObject,
		MUIA_Text_PreParse, "\33r",
		MUIA_Text_Contents, text,
	End );
}

Object *Button( char *text __asm__("a0") )
{
	return( TextObject,
		MUIA_Text_Contents, text,
		ButtonFrame,
		MUIA_Background, MUII_ButtonBack,
		MUIA_Text_PreParse, "\ec",
		MUIA_Font, MUIV_Font_Button,
		MUIA_InputMode, MUIV_InputMode_RelVerify,
	End );
}

Object *ButtonT( char *text __asm__("a0") )
{
	return( TextObject,
		MUIA_Text_Contents, text,
		ButtonFrame,
		MUIA_Background, MUII_ButtonBack,
		MUIA_Text_PreParse, "\ec",
		MUIA_Font, MUIV_Font_Button,
		MUIA_InputMode, MUIV_InputMode_Toggle,
		MUIA_Selected, TRUE,
	End );
}

Object *SmallButton( char *text __asm__("a0") )
{
	return( TextObject,
		MUIA_Text_Contents, text,
		ButtonFrame,
		MUIA_Background, MUII_ButtonBack,
		MUIA_Font, MUIV_Font_Button,
		MUIA_InputMode, MUIV_InputMode_RelVerify,
		MUIA_Text_SetMax, TRUE,
	End );
}

Object *MiniButton( void )
{
	return( RectangleObject,
		ButtonFrame,
		MUIA_Background, MUII_ButtonBack,
		MUIA_InputMode, MUIV_InputMode_RelVerify,
		MUIA_FixWidth, 6,
	End );
}

Object *SmallToggle( char *text __asm__("a0") )
{
	return( TextObject,
		MUIA_Text_Contents, text,
		ButtonFrame,
		MUIA_Background, MUII_ButtonBack,
		MUIA_Font, MUIV_Font_Button,
		MUIA_InputMode, MUIV_InputMode_Toggle,
		MUIA_Text_SetMax, TRUE,
	End );
}

Object *ReplaceCodes( void )
{
	return( TextObject,
		TextFrame,
		MUIA_Background, MUII_TextBack,
		MUIA_Text_Contents,GetString(MSG_CONTROL_CODES),
	End );
}

Object *Balance( ULONG id )
{
	Object *obj;
	if ( (obj = MUI_NewObject(MUIC_BetterBalance,MUIA_ObjectID,id,TAG_DONE)) )return( obj );
	return( MUI_NewObject(MUIC_Balance,TAG_DONE) );
}

void update_list( struct ftpdata *ftp )
{
	struct ftpdata *prev;
	int i = 0;

	for ( ; ; i++ )
	{
		DoMethod( MAIN_List, MUIM_NList_GetEntry, i, &prev );
		if ( !prev )break;
		if ( prev == ftp )
		{
			DoMethod( MAIN_List, MUIM_NList_Redraw, i );
			break;
		}
	}
}

void update_ftp( struct ftpdata *ftp )
{
	char *old_text, buf[512];

	sprintf( ftp->buffer, "%s [%s]\n%s\n%s\n", (ftp->user&&ftp->user!=(APTR)-1)?ftp->user->user:GetString(MSG_LOGIN), ftp->ip, ftp->path, ftp->cd );
	if ( ftp->file || ftp->file2 )
	{
		sprintf( buf, "%s (%s", ftp->filename, add_comma(ftp->current,FALSE) );
		strcat( ftp->buffer, buf );
		if ( ftp->size )
		{
			sprintf( buf, "/%s", add_comma(ftp->size,FALSE) );
			strcat( ftp->buffer, buf );
		}
		sprintf( buf, " bytes, %s cps)", add_comma(ftp->cps,FALSE) );
		strcat( ftp->buffer, buf );
	}
	if ( !get(ftp->Info,MUIA_Text_Contents,&old_text) || strcmp(ftp->buffer,old_text) )
	{
		struct ftpdata *prev;

		if ( ftp->Info )set( ftp->Info, MUIA_Text_Contents, ftp->buffer );
		DoMethod( MAIN_List, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &prev );
		if ( prev == ftp )DoMethod( APP_Main,MUIM_MultiSet,MUIA_Disabled,ftp->file?FALSE:TRUE, MAIN_Abort, ftp->Abort, NULL );
	}
	return;
}

void update_users( void )
{
	static char buffer[64], iconname[64];
	static ULONG old_connections = 1234, old_opencount = 1234;

	if ( IconBase && icon && iconport )
	{
		if ( appicon )RemoveAppIcon( appicon );
		if ( connections )sprintf( iconname, "RC-FTPd (%ld/%ld)", opencount, connections );
		else sprintf( iconname, "RC-FTPd (%ld)", opencount );
		appicon = AddAppIcon( 0, 0, iconname, iconport, NULL, icon, WBAPPICONA_SupportsOpen, TRUE, WBAPPICONA_SupportsSnapshot, TRUE, WBAPPICONA_SupportsUnSnapshot, TRUE, TAG_DONE );
	}
	if ( old_opencount==opencount && old_connections==connections )return;
	if ( connections )set( MAIN_Display, MUIA_Gauge_Max, connections );
	else set( MAIN_Display, MUIA_Gauge_Max, 1 );
	set( MAIN_Display, MUIA_Gauge_Current, opencount );
	if ( connections )sprintf(buffer,GetString(MSG_ONLINE_LIMIT), opencount, connections );
	else sprintf(buffer,GetString(MSG_ONLINE_NOLIMIT), opencount );
	set( MAIN_Display, MUIA_Gauge_InfoText, buffer );
	old_opencount = opencount;
	old_connections = connections;
}

void update_cps( void )
{
	static char buffer[64];
	static ULONG old_cps = 1234, old_cps2 = 1234, old_maxcps = 1234, old_files = 1234;
	struct ftpdata *ftp;
	ULONG cps = 0, cps2 = 0, files = 0;

	for ( ftp = ftp_list ; ftp ; ftp = ftp->next )
	{
		if ( IS_SET(ftp->flags,FLAG_SPEEDY) )cps2 += ftp->cps;
		else cps += ftp->cps;
		if ( ftp->file || ftp->file2 )files++;
	}
	if ( old_cps==cps && old_cps2==cps2 && old_maxcps==maxcps )return;
	if ( !files )
	{
		sprintf( buffer, "No transfers" );
		set( MAIN_CPS, MUIA_Gauge_Max, 1 );
	}
	else if ( maxcps )
	{
		if ( cps2 )sprintf( buffer, "%ld (%ld)/%ld cps in %ld files", cps, cps2, maxcps, files );
		else sprintf( buffer, "%ld/%ld cps in %ld files", cps, maxcps, files );
		set( MAIN_CPS, MUIA_Gauge_Max, maxcps );
	}
	else
	{
		if ( cps2 )sprintf( buffer, "%ld (%ld) cps in %ld files", cps, cps2, files );
		else sprintf( buffer, "%ld cps in %ld files", cps, files );
		set( MAIN_CPS, MUIA_Gauge_Max, 1 );
	}
	set( MAIN_CPS, MUIA_Gauge_Current, cps?cps:cps2 );
	set( MAIN_CPS, MUIA_Gauge_InfoText, buffer );
	old_cps = cps;
	old_maxcps = maxcps;
	old_cps2 = cps2;
	old_files = files;
}

void add_comma_short( char *to, char *fmt, long x, long y, char *type, BOOL short_num )
{
	char buf[128];
	char group[2];

	group[1] = '\0';
	if ( short_num )group[0] = locale ? locale->loc_DecimalPoint[0] : '.';
	else group[0]  = locale ? locale->loc_GroupSeparator[0] : ',';
	sprintf( buf, fmt, x, group );
	strcat( to, buf );
	if ( type && short_num )
	{
		sprintf( buf, "%ld%s", y/100, type );
		strcat( to, buf );
	}
}

char add_comma1[] = "%ld%s";
char add_comma2[] = "%03.ld%s";

char *add_comma( long long number, BOOL short_num )
{
	static char buffer[128];
	char buf[128], *out;
	char *dataname[7] = { NULL, NULL, "T", "G", "M", "K", NULL };
	long data[7];
	int i;

	buffer[0] = '\0';
	out = add_comma1;
	if ( short_num )short_num = short_numbers;

	for ( i = 0 ; i < 7 ; i++ )
	{
		data[6-i] = number % 1000;
		number /= 1000;
	}
	for ( i = 0 ; i < 6 ; i++ )
	{
		if ( out==add_comma2 || data[i] )
		{
			add_comma_short( buffer, out, data[i], data[i+1], dataname[i], short_num );
			if ( dataname[i] && short_num )return( buffer );
			out = add_comma2;
		}
	}
	sprintf( buf, out, data[i], "" );
	strcat( buffer, buf );
	return( buffer );
}

void setstat( Object *obj, char *str, unsigned long long value )
{
	char buffer[128];

	sprintf( buffer, str, add_comma(value,FALSE) );
	set( obj, MUIA_Text_Contents, buffer );
}

void setstat2( Object *obj, char *str, unsigned long long value )
{
	char buffer[128];

	sprintf( buffer, str, add_comma(value,TRUE) );
	set( obj, MUIA_Text_Contents, buffer );
}

void update_stats( void )
{
	unsigned long cps;

	setstat2( STATS_Data_Sent,		GetString(MSG_STATS_DATA_BYTES), stats[stats_page].fs_sent_data );
	setstat2( STATS_Data_Recv,		GetString(MSG_STATS_DATA_BYTES), stats[stats_page].fs_recv_data );
	setstat2( STATS_Data_Total,	GetString(MSG_STATS_DATA_BYTES), stats[stats_page].fs_sent_data + stats[stats_page].fs_recv_data );

	setstat( STATS_Files_Sent,		GetString(MSG_STATS_FILES_FILES), stats[stats_page].fs_sent_files );
	setstat( STATS_Files_Recv,		GetString(MSG_STATS_FILES_FILES), stats[stats_page].fs_recv_files );
	setstat( STATS_Files_Total,	GetString(MSG_STATS_FILES_FILES), stats[stats_page].fs_sent_files + stats[stats_page].fs_recv_files );

	if ( stats[stats_page].fs_sent_cps && stats[stats_page].fs_recv_cps )cps = ( stats[stats_page].fs_sent_cps + stats[stats_page].fs_recv_cps ) / 2;
	else if ( stats[stats_page].fs_sent_files )cps = stats[stats_page].fs_sent_cps;
	else cps = stats[stats_page].fs_recv_cps;
	setstat( STATS_CPS_Sent,		GetString(MSG_STATS_CPS_CPS), stats[stats_page].fs_sent_cps );
	setstat( STATS_CPS_Recv,		GetString(MSG_STATS_CPS_CPS), stats[stats_page].fs_recv_cps );
	setstat( STATS_CPS_Total,		GetString(MSG_STATS_CPS_CPS), cps );

	setstat( STATS_User_Account,	"%s", stats[stats_page].fs_users_account );
	setstat( STATS_User_Anon,		"%s", stats[stats_page].fs_users_anon );
	setstat( STATS_User_Total,		"%s", stats[stats_page].fs_users_account + stats[stats_page].fs_users_anon );
}

static unsigned long __saveds STATS_Reset_Func( void )
{
	memclr( &stats[stats_page], sizeof(struct ftpd_stat) );
	save_stats( "PROGDIR:ftpd.stats" );
	update_stats();
	return(0);
}
struct Hook STATS_Reset_Hook = { {NULL,NULL},&STATS_Reset_Func,NULL,NULL };

static unsigned long __saveds MAIN_List_Func( char **array __asm__("a2"), struct ftpdata *ftp __asm__("a1") )
{
	if ( !ftp )
	{
		*array++ = GetString(MSG_LIST_STATUS);
		*array++ = GetString(MSG_LIST_IP);
		*array++ = GetString(MSG_LIST_USER);
		*array++ = GetString(MSG_LIST_CMD);
		*array++ = GetString(MSG_LIST_ARGS);
	}
	else
	{
		long i;
		if ( ftp->file || ftp->file2 )i = 4;
		else
		{
			unsigned long seconds;

			seconds = timer( ftp );
			if ( seconds<30 )i = 1;
			else if ( seconds<60 )i = 2;
			else i = 3;
		}
		array[-1] = (void*)-2;
		sprintf( ftp->buffer, "\eo[%ld]%s%s%s%s",
			i,
			ftp->file||ftp->file2?"\eI[6:24]":"",
			((ftp->file||ftp->file2)&&IS_SET(ftp->flags,FLAG_SEND))?"\eI[6:30]":"",
			((ftp->file||ftp->file2)&&IS_SET(ftp->flags,FLAG_RECV))?"\eI[6:31]":"",
			ftp->data>=0?"\eI[6:28]":"" );
		*array++ = ftp->buffer;
		*array++ = ftp->ip;
		array[DISPLAY_ARRAY_MAX] = ftp->loglevel?"\eb":"";
		*array++ = IS_SET(ftp->flags,FLAG_ONLINE)?ftp->user->user:GetString(MSG_LOGIN);
		*array++ = ftp->command;
		*array++ = (!stricmp(ftp->command,"pass")?"********":ftp->args);
	}
	return(0);
}
struct Hook MAIN_List_Hook = { {NULL,NULL},&MAIN_List_Func,NULL,NULL };

static unsigned long __saveds MAIN_Kick_Func( char **array __asm__("a2"), struct ftpdata **obj __asm__("a1") )
{
	struct ftpdata *ftp;

	if ( !( ftp = *obj ) )DoMethod( MAIN_List, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &ftp );
	if ( ftp )
	{
		output( ftp, REPLY_421, 421 );
		SET_BIT(ftp->flags,FLAG_QUIT);
	}
	return(0);
}
struct Hook MAIN_Kick_Hook = { {NULL,NULL},&MAIN_Kick_Func,NULL,NULL };

static unsigned long __saveds MAIN_Stats_Func( char **array __asm__("a2"), ULONG *value __asm__("a1") )
{
	static long statswidth;
	long open;

	get( WIN_Main, MUIA_Window_Open, &open );
	if ( open )
	{
		long statsopen;

		get( MAIN_StatsGroup, MUIA_ShowMe, &statsopen );
		if ( statsopen )
		{
			long leftright, rightright;

			get( MAIN_ServerGroup, MUIA_RightEdge, &leftright );
			get( MAIN_StatsGroup, MUIA_RightEdge, &rightright );
			statswidth = rightright - leftright;
		}
	}
	set( MAIN_StatsGroup, MUIA_ShowMe, *value );
	set( MAIN_Balance, MUIA_ShowMe, *value );
	if ( open )
	{
		struct Window *win;

		get( WIN_Main, MUIA_Window_Window, &win );
		if ( win && statswidth )SizeWindow( win, (open?-statswidth:statswidth), 0 );
	}
	return(0);
}
struct Hook MAIN_Stats_Hook = { {NULL,NULL},&MAIN_Stats_Func,NULL,NULL };

static unsigned long __saveds MAIN_StatsPage_Func( char **array __asm__("a2"), ULONG *value __asm__("a1") )
{
	stats_page = *value;
	update_stats();
	return(0);
}
struct Hook MAIN_StatsPage_Hook = { {NULL,NULL},&MAIN_StatsPage_Func,NULL,NULL };



static unsigned long __saveds MAIN_Abort_Func( char **array __asm__("a2"), struct ftpdata **obj __asm__("a1") )
{
	struct ftpdata *ftp;

	if ( !( ftp = *obj ) )DoMethod( MAIN_List, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &ftp );
	if ( ftp )
	{
		output( ftp, REPLY_426, 426 );
		free_data(ftp);
		update_ftp(ftp);
	}
	return(0);
}
struct Hook MAIN_Abort_Hook = { {NULL,NULL},&MAIN_Abort_Func,NULL,NULL };

static unsigned long __saveds MAIN_Log_Func( char **array __asm__("a2") )
{
	struct ftpdata *ftp;

	DoMethod( MAIN_List, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &ftp );
	if ( ftp )
	{
		long level;

		get( MAIN_Log, MUIA_Cycle_Active, &level );
		if ( ( ftp->loglevel = (short)level ) )set( ftp->List, MUIA_ShowMe, TRUE );
		if ( ftp->Log )nnset( ftp->Log, MUIA_Cycle_Active, level );
		update_list(ftp);
		update_ftp(ftp);
	}
	return(0);
}
struct Hook MAIN_Log_Hook = { {NULL,NULL},&MAIN_Log_Func,NULL,NULL };

static unsigned long __saveds MAIN_Snoop_Func( char **array __asm__("a2") )
{
	struct ftpdata *ftp;

	DoMethod( MAIN_List, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &ftp );
	if ( ftp )
	{
		if ( !ftp->Win )start_log( ftp );
		set( ftp->Win, MUIA_Window_Open, TRUE );
		DoMethod( ftp->Win, MUIM_Window_ToFront );
	}
	return(0);
}
struct Hook MAIN_Snoop_Hook = { {NULL,NULL},&MAIN_Snoop_Func,NULL,NULL };

static unsigned long __saveds MAIN_Open_Func( char **array __asm__("a2") )
{
	long isopen;
	get( MAIN_Open, MUIA_Cycle_Active, &isopen );
	open = isopen?TRUE:FALSE;
	return(0);
}
struct Hook MAIN_Open_Hook = { {NULL,NULL},&MAIN_Open_Func,NULL,NULL };

static unsigned long __saveds MAIN_List_Active_Func( char **array __asm__("a2"), long *entry __asm__("a1") )
{
	if ( *entry!=MUIV_NList_Active_Off )
	{
		struct ftpdata *ftp;
		DoMethod( MAIN_List, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &ftp );
		set( MAIN_Abort, MUIA_Disabled, (ftp->file||ftp->file2)?FALSE:TRUE );
		DoMethod( APP_Main, MUIM_MultiSet, MUIA_Disabled, FALSE, MAIN_Kick, MAIN_Message, MAIN_Log, NULL );
		if ( ftp->Log )
		{
			long level;
			get( ftp->Log, MUIA_Cycle_Active, &level );
			nnset( MAIN_Log, MUIA_Cycle_Active, level );
		}
	}
	else DoMethod( APP_Main, MUIM_MultiSet, MUIA_Disabled, TRUE, MAIN_Kick, MAIN_Message, MAIN_Log, MAIN_Abort, NULL );
	return(0);
}
struct Hook MAIN_List_Active_Hook = { {NULL,NULL},&MAIN_List_Active_Func,NULL,NULL };

static unsigned long __saveds MAIN_Message_Func( Object *string __asm__("a2"), struct ftpdata **target  __asm__("a1") )
{
	struct ftpdata *ftp;
	char *txt;

	get( string, MUIA_String_Contents, &txt );
	if ( !( ftp = *target ) )DoMethod( MAIN_List, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &ftp );
	if ( *txt && ftp )
	{
		output( ftp, "*** Message from console ***", 0 );
		output( ftp, txt, 0 );
	}
	nnset( string, MUIA_String_Contents, (ULONG)"" );
	return(0);
}
struct Hook MAIN_Message_Hook = { {NULL,NULL},&MAIN_Message_Func,NULL,NULL };

BOOL init_ftpd( void )
{
	struct sockaddr_in sa;
	long nonblocking = 1, x;
	char	buffer[128];
	Object *MENU_Enabled, *MENU_Settings, *MENU_MuiSettings, *MENU_Quit;
	struct DaemonMessage *dm;
	struct Process *pr;

	if ( !( DOSBase = OpenLibrary( "dos.library", 37 ) ) )return FALSE;
	if ( !( IntuitionBase = OpenLibrary( "intuition.library", 36 ) ) )return FALSE;
	if ( !( MUIMasterBase = OpenLibrary( "muimaster.library", 18 ) ) )
	{
		ErrorString(MSG_STARTUP_MUI);
		return FALSE;
	}
	if ( !( SocketBase = OpenLibrary( "bsdsocket.library", 4 ) ) )
	{
		ErrorString(MSG_STARTUP_SOCKET);
		return FALSE;
	}

/*
	if ( !( TagListBase = OpenLibrary( "taglist.library", 0 ) ) )
	{
Printf( "Unable to open taglist.library\n" );
		return FALSE;
	}
	else
	{
		struct TagItem *tags;
		BPTR file;

		tags = tl_AllocateTagItems( 100 );
		if ( !tags )Printf("Unable to allocate taglist\n" );
		tl_SetTagData( TAG_USER|1, 1234, tags );
		tl_SetTagData( TAG_USER|TAG_STRING|2, (ULONG)"testing", tags );
		Printf("Value: %ld, %s\n", tl_GetTagData(TAG_USER|1,2345,tags), tl_GetTagData(TAG_USER|TAG_STRING|2,(ULONG)"",tags) );
		if ( ( file = Open( "Ram:test", MODE_NEWFILE ) ) )
		{
			tl_SaveTags( file, tags );
			Close( file );
		}
		tl_ClearTagData( TAG_USER|1, tags );
		tl_ClearTagData( TAG_USER|TAG_STRING|2, tags );
		Printf("Value: %ld, %s\n", tl_GetTagData(TAG_USER|1,2345,tags), tl_GetTagData(TAG_USER|TAG_STRING|2,(ULONG)"",tags) );
		if ( ( file = Open( "Ram:test", MODE_OLDFILE ) ) )
		{
			tl_LoadTags( file, tags );
			Close( file );
		}
		Printf("Value: %ld, %s\n", tl_GetTagData(TAG_USER|1,2345,tags), tl_GetTagData(TAG_USER|TAG_STRING|2,(ULONG)"",tags) );
		tl_FreeTagItems( tags );
		CloseLibrary( TagListBase );
	}
*/
	if ( !( myPool = CreatePool( MEMF_ANY|MEMF_CLEAR, 65536, 16384 ) ) )
	{
		ErrorString(MSG_STARTUP_MEMORY);
		return FALSE;
	}
	if ( ( IconBase = OpenLibrary( "icon.library", 44 ) ) )
	{
		WorkbenchBase = OpenLibrary( "workbench.library", 36 );
		iconport = CreateMsgPort();
	}
	ftpd_log[0]			= GetString(MSG_LOG_NONE);
	ftpd_log[1]			= GetString(MSG_LOG_KNOWN);
	ftpd_log[2]			= GetString(MSG_LOG_COMMANDS);
	ftpd_log[3]			= GetString(MSG_LOG_ALL);
	closedopen[0]	= GetString(MSG_FTP_CLOSED);
	closedopen[1]	= GetString(MSG_FTP_OPEN);
	page[0]			= "Page: 1";
	page[1]			= "Page: 2";
	page[2]			= "Page: 3";
	APP_Main = ApplicationObject,
		MUIA_Application_Title,			"RC-FTPd",
		MUIA_Application_Version,		version,
		MUIA_Application_Copyright,	"©2000 Robin Cloutman",
		MUIA_Application_Author,		"Robin Cloutman",
		MUIA_Application_Description,	GetString(MSG_MUI_ABOUT),
		MUIA_Application_Base,			"RC-FTPD",
		MUIA_Application_HelpFile,		GetString(MSG_FTP_GUIDE),
		SubWindow, WIN_Main = WindowObject,
			MUIA_Window_Title, windowname,
			MUIA_Window_ID, MAKE_ID('M','A','I','N'),
			MUIA_Window_Menustrip, MenustripObject,
				Child, MenuObjectT(GetString(MSG_MENU_PROJECT)),
					Child, MENU_Enabled = MenuitemObject,
						MUIA_Menuitem_Title, GetString(MSG_MENU_ENABLED),
						MUIA_Menuitem_Checkit, TRUE,
//						MUIA_Menuitem_Shortcut, "E",
					End,
					Child, MenuitemObject,MUIA_Menuitem_Title,-1,End,
					Child, MENU_Users = MenuitemObject,
						MUIA_Menuitem_Title, "Users",
						MUIA_Menuitem_Checkit, TRUE,
//						MUIA_Menuitem_Shortcut, "U",
						MUIA_ObjectID, MAKE_ID('M','N','U','S'),
					End,
					Child, MENU_Stats = MenuitemObject,
						MUIA_Menuitem_Title, "Stats",
						MUIA_Menuitem_Checkit, TRUE,
//						MUIA_Menuitem_Shortcut, "S",
						MUIA_ObjectID, MAKE_ID('M','N','S','T'),
					End,
//					Child, MenuitemObject,
//						MUIA_Menuitem_Title, GetString(MSG_MENU_ABOUT),
//					End,
					Child, MenuitemObject,MUIA_Menuitem_Title,-1,End,
					Child, MENU_Settings = MenuitemObject,
						MUIA_Menuitem_Title, GetString(MSG_MENU_SETTINGS),
					End,
					Child, MENU_MuiSettings = MenuitemObject,
						MUIA_Menuitem_Title, GetString(MSG_MENU_MUISETTINGS),
					End,
					Child, MenuitemObject,MUIA_Menuitem_Title,-1,End,
					Child, MENU_Quit = MenuitemObject,
						MUIA_Menuitem_Title, GetString(MSG_MENU_QUIT),
//						MUIA_Menuitem_Shortcut, "Q",
					End,
				End,
			End,
			WindowContents, MAIN_MainGroup = HGroup,
				MUIA_Background, MUII_WindowBack,
				MUIA_Frame, MUIV_Frame_None,
				Child, MAIN_ServerGroup = VGroup,
					Child, VGroup,
						GroupFrameT( "Server" ),
						Child, MAIN_Display = GaugeObject,
							GaugeFrame,
							MUIA_Gauge_Horiz, TRUE,
							MUIA_Gauge_InfoText, "",
						End,
						Child, MAIN_CPS = GaugeObject,
							GaugeFrame,
							MUIA_Gauge_Horiz, TRUE,
							MUIA_Gauge_InfoText, "",
						End,
						Child, HGroup,
							MUIA_Weight, 0,
							Child, MAIN_Prefs = Button( GetString(MSG_MUI_PREFS) ),
							Child, MAIN_Open = CycleObject,
								MUIA_ShortHelp, GetString(MSG_SHORT_OPEN),
								MUIA_Font, MUIV_Font_Button,
								MUIA_Cycle_Entries, closedopen,
								MUIA_Cycle_Active, open?1:0,
							End,
						End,
					End,
					Child, MAIN_UserGroup = VGroup,
						GroupFrameT( GetString(MSG_LIST_USER) ),
						Child, MAIN_List = NListviewObject,
							MUIA_NListview_NList, NListObject,
								InputListFrame,
								MUIA_NList_Format, "BAR PW=45,BAR,BAR,BAR,BAR",
								MUIA_NList_DisplayHook, &MAIN_List_Hook,
								MUIA_NList_Title, TRUE,
								MUIA_NList_AutoCopyToClip, TRUE,
								MUIA_NList_Exports, MUIV_NList_Exports_Cols,
								MUIA_NList_Imports, MUIV_NList_Imports_Cols,
								MUIA_NList_MinColSortable, 0,
								MUIA_NList_AdjustHeight, TRUE,
								MUIA_ObjectID, MAKE_ID('M','L','S','T'),
							End,
						End,
						Child, HGroup,
							Child, HGroup,
								MUIA_ShortHelp, GetString(MSG_SHORT_KICKABORT),
								Child, MAIN_Kick = Button( GetString(MSG_KICK) ),
								Child, MAIN_Abort = Button( GetString(MSG_ABORT) ),
							End,
							Child, MAIN_Log = CycleObject,
								MUIA_Font, MUIV_Font_Button,
								MUIA_Cycle_Entries, ftpd_log,
							End,
						End,
						Child, MAIN_Message = StringObject,
							MUIA_HelpNode, "MESS",
							MUIA_ShortHelp, GetString(MSG_SHORT_MESSAGE),
							StringFrame,
							MUIA_String_MaxLen, 512,
						End,
					End,
				End,
				Child, MAIN_Balance = Balance( MAKE_ID('M','N','B','A') ),
				Child, MAIN_StatsGroup = VGroup,
					GroupFrameT( "Stats" ),
					Child, HVSpace,
					Child, HGroup,
						Child, MAIN_StatsPage = CycleObject,
							MUIA_Font, MUIV_Font_Button,
							MUIA_Cycle_Entries, page,
							MUIA_Cycle_Active, 0,
						End,
						Child, MAIN_StatsReset = Button( "Reset Page" ),
					End,
					Child, HVSpace,
					Child, ColGroup(2),
						GroupFrameT2( GetString(MSG_STATS_DATA) ),
						Child, LeftText( GetString(MSG_STATS_SENT) ),
						Child, STATS_Data_Sent = RightText( "" ),
						Child, LeftText( GetString(MSG_STATS_RECIEVED) ),
						Child, STATS_Data_Recv = RightText( "" ),
						Child, LeftText( GetString(MSG_STATS_TOTAL) ),
						Child, STATS_Data_Total = RightText( "" ),
					End,
					Child, ColGroup(2),
						GroupFrameT2( GetString(MSG_STATS_FILES) ),
						Child, LeftText( GetString(MSG_STATS_SENT) ),
						Child, STATS_Files_Sent = RightText( "" ),
						Child, LeftText( GetString(MSG_STATS_RECIEVED) ),
						Child, STATS_Files_Recv = RightText( "" ),
						Child, LeftText( GetString(MSG_STATS_TOTAL) ),
						Child, STATS_Files_Total = RightText( "" ),
					End,
					Child, ColGroup(2),
						GroupFrameT2( GetString(MSG_STATS_CPS) ),
						Child, LeftText( GetString(MSG_STATS_SENT) ),
						Child, STATS_CPS_Sent = RightText( "" ),
						Child, LeftText( GetString(MSG_STATS_RECIEVED) ),
						Child, STATS_CPS_Recv = RightText( "" ),
						Child, LeftText( GetString(MSG_STATS_TOTAL) ),
						Child, STATS_CPS_Total = RightText( "" ),
					End,
					Child, ColGroup(2),
						GroupFrameT2( GetString(MSG_STATS_USERS) ),
						Child, LeftText( GetString(MSG_STATS_USERS_ACCOUNT) ),
						Child, STATS_User_Account = RightText( "" ),
						Child, LeftText( GetString(MSG_STATS_USERS_ANON) ),
						Child, STATS_User_Anon = RightText( "" ),
						Child, LeftText( GetString(MSG_STATS_TOTAL) ),
						Child, STATS_User_Total = RightText( "" ),
					End,
					Child, HVSpace,
				End,
			End,
		End,
		SubWindow, WIN_Prefs = PrefsWindow(),
	End;
	if( !APP_Main )
	{
		ErrorString(MSG_STARTUP_APP);
		return FALSE;
	}
	DoMethod( APP_Main, MUIM_Application_Load, MUIV_Application_Load_ENV );

	DoMethod( APP_Main,MUIM_MultiSet,MUIA_Disabled,TRUE, MAIN_Kick, MAIN_Abort, MAIN_Log, MAIN_Message, NULL );
	DoMethod( APP_Main,MUIM_MultiSet,MUIA_CycleChain,TRUE, MAIN_Prefs, MAIN_List, MAIN_Kick, MAIN_Abort, MAIN_Open, NULL );

	DoMethod( WIN_Main, MUIM_Notify,			MUIA_Window_CloseRequest,	TRUE, APP_Main, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit );
	DoMethod( MAIN_List, MUIM_Notify,		MUIA_NList_DoubleClick,		MUIV_EveryTime, APP_Main, 2, MUIM_CallHook, &MAIN_Snoop_Hook );
	DoMethod( MAIN_List, MUIM_Notify,		MUIA_NList_Active,			MUIV_EveryTime, APP_Main, 3, MUIM_CallHook, &MAIN_List_Active_Hook, MUIV_TriggerValue );
	DoMethod( MAIN_Kick, MUIM_Notify,		MUIA_Pressed,					FALSE, APP_Main, 3, MUIM_CallHook, &MAIN_Kick_Hook, 0 );
	DoMethod( MAIN_Abort, MUIM_Notify,		MUIA_Pressed,					FALSE, APP_Main, 3, MUIM_CallHook, &MAIN_Abort_Hook, 0 );
	DoMethod( MAIN_Log,	MUIM_Notify,		MUIA_Cycle_Active,			MUIV_EveryTime, APP_Main, 2, MUIM_CallHook, &MAIN_Log_Hook );
	DoMethod( MAIN_Open, MUIM_Notify,		MUIA_Cycle_Active,			MUIV_EveryTime, MENU_Enabled, 3, MUIM_NoNotifySet, MUIA_Menuitem_Checked, MUIV_TriggerValue );
	DoMethod( MAIN_Open, MUIM_Notify,		MUIA_Cycle_Active,			MUIV_EveryTime, APP_Main,2,MUIM_CallHook,&MAIN_Open_Hook );
	DoMethod( MAIN_Message, MUIM_Notify,	MUIA_String_Acknowledge,	MUIV_EveryTime, MUIV_Notify_Self, 3, MUIM_CallHook, &MAIN_Message_Hook, 0 );
	DoMethod( MAIN_StatsPage,MUIM_Notify,	MUIA_Cycle_Active,			MUIV_EveryTime, APP_Main,3,MUIM_CallHook,&MAIN_StatsPage_Hook, MUIV_TriggerValue );
	DoMethod( MAIN_StatsReset,MUIM_Notify,	MUIA_Pressed,					FALSE, APP_Main, 2, MUIM_CallHook, &STATS_Reset_Hook );

	DoMethod( MAIN_Prefs, MUIM_Notify,		MUIA_Pressed,					FALSE, WIN_Prefs, 3, MUIM_NoNotifySet, MUIA_Window_Open, TRUE );

	DoMethod( MENU_Stats, MUIM_Notify,		MUIA_Menuitem_Checked,		MUIV_EveryTime, APP_Main, 3, MUIM_CallHook, &MAIN_Stats_Hook, MUIV_TriggerValue );
	DoMethod( MENU_Users, MUIM_Notify,		MUIA_Menuitem_Checked,		MUIV_EveryTime, MAIN_UserGroup, 3, MUIM_Set, MUIA_ShowMe, MUIV_TriggerValue );

	DoMethod( MENU_Enabled, MUIM_Notify,	MUIA_Menuitem_Checked,		MUIV_EveryTime, MAIN_Open, 3, MUIM_Set, MUIA_Cycle_Active, MUIV_TriggerValue );
	DoMethod( MENU_Settings, MUIM_Notify,	MUIA_Menuitem_Trigger,		MUIV_EveryTime, WIN_Prefs, 3, MUIM_NoNotifySet, MUIA_Window_Open, TRUE );
	DoMethod( MENU_MuiSettings, MUIM_Notify,MUIA_Menuitem_Trigger,		MUIV_EveryTime, APP_Main, 2, MUIM_Application_OpenConfigWindow, 0 );
	DoMethod( MENU_Quit, MUIM_Notify,		MUIA_Menuitem_Trigger,		MUIV_EveryTime, APP_Main, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit );

	DoMethod( MAIN_List, MUIM_NList_UseImage, (LampObject,MUIA_Lamp_Color,MUIV_Lamp_Color_Ok,End), 1, 0 );
	DoMethod( MAIN_List, MUIM_NList_UseImage, (LampObject,MUIA_Lamp_Color,MUIV_Lamp_Color_Warning,End), 2, 0 );
	DoMethod( MAIN_List, MUIM_NList_UseImage, (LampObject,MUIA_Lamp_Color,MUIV_Lamp_Color_Error,End), 3, 0 );
	DoMethod( MAIN_List, MUIM_NList_UseImage, (LampObject,MUIA_Lamp_Color,MUIV_Lamp_Color_Processing,End), 4, 0 );

	DoMethod( STATS_Reset, MUIM_Notify,		MUIA_Menuitem_Trigger,		MUIV_EveryTime, MUIV_Notify_Self, 2, MUIM_CallHook, &STATS_Reset_Hook );

	get( MENU_Stats, MUIA_Menuitem_Checked, &x );
	if ( !x )
	{
		set( MAIN_StatsGroup, MUIA_ShowMe, FALSE );
		set( MAIN_Balance, MUIA_ShowMe, FALSE );
	}
	get( MENU_Users, MUIA_Menuitem_Checked, &x );
	if ( !x )set( MAIN_UserGroup, MUIA_ShowMe, FALSE );

	strcpy( buffer, "ENV:" );
	AddPart( buffer, configname, 128 );
	load_config( buffer );
	load_stats( "PROGDIR:ftpd.stats" );
	update_stats( );
	update_cps();

	if ( IconBase && ( icon = GetIconTags(NULL, ICONGETA_GetDefaultName, (LONG)"rc-ftpd", TAG_DONE ) ) )
	{
		icon->do_Type		= 0;
		icon->do_CurrentX	= iconx;
		icon->do_CurrentY	= icony;
	}
	update_users();

	pr = (struct Process*)FindTask(NULL);
	dm = (struct DaemonMessage*)pr->pr_ExitData;
	if ( dm && (control = ObtainSocket(dm->dm_ID,dm->dm_Family,dm->dm_Type,0))>=0 )
	{
		new_ftp( control );
	}
	else
	{
//syslog( 5, "Trying to start a new socket too!!!\n" );
//CloseSocket( control );
//return(10);
		if ( ( control = socket(AF_INET, SOCK_STREAM, 0) ) < 0 )
		{
			ErrorString(MSG_SOCKET_OPEN);
			return FALSE;
		}
		IoctlSocket( control, FIONBIO, (char*)&nonblocking );
		if ( setsockopt( control, SOL_SOCKET, SO_REUSEADDR, (char *) &x, sizeof(x) ) < 0 )
		{
			ErrorString(MSG_SOCKET_OPTIONS);
			return FALSE;
		}
		memclr( &sa, sizeof(struct sockaddr_in) );
		sa.sin_family   = AF_INET;
		sa.sin_port	    = htons(port);
		if ( bind( control, (struct sockaddr*)&sa, sizeof(sa) ) == -1 )
		{
			ErrorString(MSG_SOCKET_BIND);
			return FALSE;
		}
		if ( listen( control, 5 ) < 0 )
		{
			ErrorString(MSG_SOCKET_LISTEN);
			return FALSE;
		}
	}
	return TRUE;
}
