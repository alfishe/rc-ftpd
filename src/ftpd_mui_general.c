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
#include	<mui/NList_mcc.h>
#include	<mui/NListview_mcc.h>

#include	<clib/alib_protos.h>
#include	<clib/muimaster_protos.h>

#include	<inline/exec.h>
#include	<inline/socket.h>
#include	<inline/dos.h>

#include "ftpd.h"

Object	*XTRA_Port;
Object	*XTRA_Port_Up;
Object	*XTRA_Port_Down;
Object	*XTRA_List;
Object	*XTRA_Anon;
Object	*XTRA_Cps;
//Object	*XTRA_Cps_Up;
//Object	*XTRA_Cps_Down;
Object	*XTRA_Users;
Object	*XTRA_Users_Up;
Object	*XTRA_Users_Down;
Object	*XTRA_Timeout;
Object	*XTRA_Timeout_Up;
Object	*XTRA_Timeout_Down;
Object	*XTRA_Noop;
Object	*XTRA_Nopasv;
Object	*XTRA_Message;
Object	*XTRA_Comment;
Object	*XTRA_LogFile;
Object	*XTRA_Seperate;
Object	*XTRA_Async;
Object	*XTRA_Short;

void SetGeneral( void )
{
	set( XTRA_Port, MUIA_Numeric_Value, port );
	set( XTRA_List, MUIA_String_Contents, list );
	set( XTRA_Anon, MUIA_Selected, anon );
	set( XTRA_Users, MUIA_Numeric_Value, connections );
	set( XTRA_Cps, MUIA_String_Integer, maxcps );
	set( XTRA_Timeout, MUIA_Numeric_Value, timeout );
	set( XTRA_Noop, MUIA_Selected, noop );
	set( XTRA_Nopasv, MUIA_Selected, nopasv );
	set( XTRA_Message, MUIA_Selected, message );
	set( XTRA_Comment, MUIA_Selected, comment );
	set( XTRA_LogFile, MUIA_Selected, logfile );
	set( XTRA_Seperate, MUIA_Disabled, !logfile );
	set( XTRA_Seperate, MUIA_Selected, seperate );
	set( XTRA_Async, MUIA_Selected, async );
	set( XTRA_Short, MUIA_Selected, short_numbers );
}

void GetGeneral( void )
{
	char *string;
	long value;

	get( XTRA_Port, MUIA_Numeric_Value, &value );
		port = value;
	get( XTRA_List, MUIA_String_Contents, &string );
		strcpy( list, string );
	get( XTRA_Anon, MUIA_Selected, &value );
		anon = value;
	get( XTRA_Users, MUIA_Numeric_Value, &value );
		connections = value;
	get( XTRA_Cps, MUIA_String_Integer, &value );
		maxcps = value;
	get( XTRA_Timeout, MUIA_Numeric_Value, &value );
		timeout = value;
	get( XTRA_Noop, MUIA_Selected, &value );
		noop = value;
	get( XTRA_Nopasv, MUIA_Selected, &value );
		nopasv = value;
	get( XTRA_Message, MUIA_Selected, &value );
		message = value;
	get( XTRA_Comment, MUIA_Selected, &value );
		comment = value;
	get( XTRA_LogFile, MUIA_Selected, &value );
		logfile = value;
	get( XTRA_Seperate, MUIA_Selected, &value );
		seperate = value;
	get( XTRA_Async, MUIA_Selected, &value );
		async = value;
	get( XTRA_Short, MUIA_Selected, &value );
		short_numbers = value;
	update_stats();
	update_cps();
}

Object *GeneralGroup( void )
{
	Object *new;

	new = VGroup,
		InnerSpacing(4,4),
		ReadListFrame,
		Child, HVSpace,
		Child, VGroup,
			Child, ColGroup(2),
				Child, SmallText( GetString(MSG_PREF_PORT) ),
				Child, HGroup,
					GroupSpacing(0),
					MUIA_ShortHelp, GetString(MSG_PREF_PORT_HELP),
					Child, XTRA_Port = NumericbuttonObject,
						MUIA_Numeric_Min, 2,
						MUIA_Numeric_Default, 21,
						MUIA_Numeric_Max, 65535,
					End,
					Child, VGroup,
						GroupSpacing(0),
						Child, XTRA_Port_Up = MiniButton(),
						Child, XTRA_Port_Down = MiniButton(),
					End,
					Child, HVSpace,
				End,
				Child, SmallText( GetString(MSG_PREF_LIST) ),
				Child, XTRA_List = StringObject,
					StringFrame,
					MUIA_ShortHelp, GetString(MSG_PREF_LIST_HELP),
					MUIA_String_MaxLen, 64,
				End,
				Child, SmallText( GetString(MSG_PREF_ANON) ),
				Child, HGroup,
					MUIA_ShortHelp, GetString(MSG_PREF_ANON_HELP),
					Child, XTRA_Anon = CheckMark( FALSE ),
					Child, HVSpace,
				End,
				Child, SmallText( GetString(MSG_PREF_MAXUSERS) ),
				Child, HGroup,
					GroupSpacing(0),
					MUIA_ShortHelp, GetString(MSG_PREF_MAXUSERS_HELP),
					Child, XTRA_Users = NumericbuttonObject,
						MUIA_Numeric_Default, 0,
					End,
					Child, VGroup,
						GroupSpacing(0),
						Child, XTRA_Users_Up = MiniButton(),
						Child, XTRA_Users_Down = MiniButton(),
					End,
					Child, HVSpace,
				End,
				Child, SmallText( GetString(MSG_PREF_MAXCPS) ),
				Child, XTRA_Cps = StringObject,
					StringFrame,
					MUIA_ShortHelp, GetString(MSG_PREF_MAXCPS_HELP),
					MUIA_String_Integer, 0,
					MUIA_String_Accept, "0123456789",
					MUIA_String_MaxLen, 8,
				End,
				Child, SmallText( GetString(MSG_PREF_TIMEOUT) ),
				Child, HGroup,
					GroupSpacing(0),
					MUIA_ShortHelp, GetString(MSG_PREF_TIMEOUT_HELP),
					Child, XTRA_Timeout = NumericbuttonObject,
						MUIA_Numeric_Default, 0,
						MUIA_Numeric_Min, 0,
						MUIA_Numeric_Max, 60,
					End,
					Child, VGroup,
						GroupSpacing(0),
						Child, XTRA_Timeout_Up = MiniButton(),
						Child, XTRA_Timeout_Down = MiniButton(),
					End,
					Child, HVSpace,
				End,
				Child, SmallText( GetString(MSG_PREF_NOOP) ),
				Child, HGroup,
					MUIA_ShortHelp, GetString(MSG_PREF_NOOP_HELP),
					Child, XTRA_Noop = CheckMark( FALSE ),
					Child, HVSpace,
				End,
				Child, SmallText( GetString(MSG_PREF_NOPASV) ),
				Child, HGroup,
					MUIA_ShortHelp, GetString(MSG_PREF_NOPASV_HELP),
					Child, XTRA_Nopasv = CheckMark( FALSE ),
					Child, HVSpace,
				End,
				Child, SmallText( GetString(MSG_PREF_AUTOMESSAGE) ),
				Child, HGroup,
					MUIA_ShortHelp, GetString(MSG_PREF_AUTOMESSAGE_HELP),
					Child, XTRA_Message = CheckMark( FALSE ),
					Child, HVSpace,
				End,
				Child, SmallText( GetString(MSG_PREF_AUTOCOMMENT) ),
				Child, HGroup,
					MUIA_ShortHelp, GetString(MSG_PREF_AUTOCOMMENT_HELP),
					Child, XTRA_Comment = CheckMark( FALSE ),
					Child, HVSpace,
				End,
				Child, SmallText( GetString(MSG_PREF_LOGFILE) ),
				Child, HGroup,
					MUIA_ShortHelp, GetString(MSG_PREF_LOGFILE_HELP),
					Child, XTRA_LogFile = CheckMark( FALSE ),
					Child, HVSpace,
					Child, SmallText( GetString(MSG_PREF_LOGFILE_SEPERATE) ),
					Child, XTRA_Seperate = CheckMark( FALSE ),
					Child, HVSpace,
				End,
				Child, SmallText( GetString(MSG_PREF_ASYNC) ),
				Child, HGroup,
					MUIA_ShortHelp, GetString(MSG_PREF_ASYNC_HELP),
					Child, XTRA_Async = CheckMark( FALSE ),
					Child, HVSpace,
				End,
				Child, SmallText( "Short Numbers" ),
				Child, HGroup,
					MUIA_ShortHelp, "T/G/M/K, saves up to 12 digits ;-)",
					Child, XTRA_Short = CheckMark( FALSE ),
					Child, HVSpace,
				End,
			End,
		End,
		Child, HVSpace,
	End;
	DoMethod( XTRA_LogFile,			MUIM_Notify, MUIA_Selected, MUIV_EveryTime, XTRA_Seperate, 3, MUIM_NoNotifySet, MUIA_Disabled, MUIV_NotTriggerValue );
	SetGeneral();
	DoMethod(APP_Main,MUIM_MultiSet,MUIA_CycleChain,TRUE, XTRA_Port, XTRA_List, XTRA_Users, XTRA_Cps, NULL );

	DoMethod( XTRA_Port_Up,			MUIM_Notify, MUIA_Pressed,	FALSE, XTRA_Port, 2, MUIM_Numeric_Increase, 1 );
	DoMethod( XTRA_Port_Down,		MUIM_Notify, MUIA_Pressed,	FALSE, XTRA_Port, 2, MUIM_Numeric_Decrease, 1 );
//	DoMethod( XTRA_Cps_Up,			MUIM_Notify, MUIA_Pressed,	FALSE, XTRA_Cps, 2, MUIM_Numeric_Increase, 256 );
//	DoMethod( XTRA_Cps_Down,		MUIM_Notify, MUIA_Pressed,	FALSE, XTRA_Cps, 2, MUIM_Numeric_Decrease, 256 );
	DoMethod( XTRA_Users_Up,		MUIM_Notify, MUIA_Pressed,	FALSE, XTRA_Users, 2, MUIM_Numeric_Increase, 1 );
	DoMethod( XTRA_Users_Down,		MUIM_Notify, MUIA_Pressed,	FALSE, XTRA_Users, 2, MUIM_Numeric_Decrease, 1 );
	DoMethod( XTRA_Timeout_Up,		MUIM_Notify, MUIA_Pressed,	FALSE, XTRA_Timeout, 2, MUIM_Numeric_Increase, 1 );
	DoMethod( XTRA_Timeout_Down,	MUIM_Notify, MUIA_Pressed,	FALSE, XTRA_Timeout, 2, MUIM_Numeric_Decrease, 1 );
	return(new);
}
