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
#include	<inline/dos.h>
#include	<inline/socket.h>

#include "ftpd.h"

Object	*WIN_Prefs;
Object	*PREFS_List;
Object	*PREFS_Page;
Object	*PREFS_Pages[9];
Object	*PREFS_Current;
Object	*PREFS_Save;
Object	*PREFS_Use;
Object	*PREFS_Cancel;

char	unreg[]	= "\ecUnregistered version\n\nPlease show your support by registering.";
char	reg[]		= "\el\ebRegistered to:\en %s\n\ebSerial:\en [%08.lx]";
char	*pages[9];

Object *InfoGroup( void )
{
	Object *new;
	char	buffer[BUFSIZE];

	if ( !registered )strcpy( buffer, unreg );
	else sprintf( buffer, reg, registered, serial );
	new = VGroup,
		InnerSpacing(4,4),
		ReadListFrame,
		Child, TextObject,
			TextFrame,
			MUIA_Background, MUII_TextBack,
			MUIA_Text_Contents, "\ecRC-FTPd\nCopyright ©2000-2002 Robin Cloutman <rc-ftpd@rycochet.demon.co.uk>",
		End,
		Child, HVSpace,
		Child, HGroup,
			Child, HVSpace,
			Child, TextObject,
				MUIA_Text_Contents, buffer,
			End,
			Child, HVSpace,
		End,
		Child, HVSpace,
	End;
	return(new);
}

static unsigned long __saveds PREFS_Close_Func( char **array __asm__("a2"), long *status __asm__("a1") )
{
	int i;

	set( WIN_Prefs, MUIA_Window_Open, FALSE );
	if ( *status > 0 )
	{
		char	buffer[128];

		if ( PREFS_Pages[1] )GetUsers();
		if ( PREFS_Pages[2] )GetPath();
		if ( PREFS_Pages[3] )GetGeneral();
		if ( PREFS_Pages[4] )GetClosed();
		if ( PREFS_Pages[5] )GetGreeting();
		if ( PREFS_Pages[6] )GetWelcome();
		if ( PREFS_Pages[7] )GetGoodbye();
		switch ( *status )
		{
			case 2:
				strcpy( buffer, "ENVARC:" );
				AddPart( buffer, configname, 128 );
				save_config( buffer );
			case 1:
				strcpy( buffer, "ENV:" );
				AddPart( buffer, configname, 128 );
				save_config( buffer );
			default:
				break;
		}
	}
	nnset( PREFS_List, MUIA_List_Active, 0 );
	DoMethod( PREFS_Page, MUIM_Group_InitChange );
	if ( PREFS_Current )DoMethod( PREFS_Page, OM_REMMEMBER, PREFS_Current );
	DoMethod( PREFS_Page, OM_ADDMEMBER, PREFS_Current = PREFS_Pages[0] );
	DoMethod( PREFS_Page, MUIM_Group_ExitChange );
	for ( i = 1 ; i < 7 ; i++ )if ( PREFS_Pages[i] )
	{
		MUI_DisposeObject(PREFS_Pages[i]);
		PREFS_Pages[i] = NULL;
	}
	return(0);
}
struct Hook PREFS_Close_Hook = { {NULL,NULL},&PREFS_Close_Func,NULL,NULL };

static unsigned long __saveds PREFS_List_Func( char **array __asm__("a2"), long *page __asm__("a1") )
{
	if ( !PREFS_Pages[*page] )switch( *page )
	{
		case 0:
			PREFS_Pages[*page] = InfoGroup();
			break;
		case 1:
			PREFS_Pages[*page] = UserGroup();
			break;
		case 2:
			PREFS_Pages[*page] = PathGroup();
			break;
		case 3:
			PREFS_Pages[*page] = GeneralGroup();
			break;
		case 4:
			PREFS_Pages[*page] = ClosedGroup();
			break;
		case 5:
			PREFS_Pages[*page] = GreetingGroup();
			break;
		case 6:
			PREFS_Pages[*page] = WelcomeGroup();
			break;
		case 7:
			PREFS_Pages[*page] = GoodbyeGroup();
			break;
		default:
			PREFS_Pages[*page] = VCenter((TextObject,MUIA_Text_Contents,"\ecInvalid page...",End));
			break;
	}
	if ( !PREFS_Pages[*page] )PREFS_Pages[*page] = VCenter((TextObject,MUIA_Text_Contents,"\ecUnable to create page.",End));
	DoMethod( PREFS_Page, MUIM_Group_InitChange );
	if ( PREFS_Current )DoMethod( PREFS_Page, OM_REMMEMBER, PREFS_Current );
	DoMethod( PREFS_Page, OM_ADDMEMBER, PREFS_Current = PREFS_Pages[*page] );
	DoMethod( PREFS_Page, MUIM_Group_ExitChange );
	return(0);
}
struct Hook PREFS_List_Hook = { {NULL,NULL},&PREFS_List_Func,NULL,NULL };

Object *PrefsWindow( void )
{
	Object *new;

	pages[0] = GetString(MSG_PAGE_INFO);
	pages[1] = GetString(MSG_PAGE_USERS);
	pages[2] = GetString(MSG_PAGE_GROUPS);
	pages[3] = GetString(MSG_PAGE_GENERAL);
	pages[4] = GetString(MSG_PAGE_CLOSED);
	pages[5] = GetString(MSG_PAGE_GREETING);
	pages[6] = GetString(MSG_PAGE_WELCOME);
	pages[7] = GetString(MSG_PAGE_GOODBYE);
	new = WIN_Prefs = WindowObject,
		MUIA_Window_Title, GetString(MSG_PREF_WINDOW),
		MUIA_Window_ID, MAKE_ID('P','R','E','F'),
		WindowContents, VGroup,
			MUIA_Background, MUII_WindowBack,
			MUIA_Frame, MUIV_Frame_None,
			Child, HGroup,
				Child, PREFS_List = ListviewObject,
					MUIA_Listview_List, ListObject,
						InputListFrame,
						MUIA_List_SourceArray, pages,
						MUIA_List_AdjustWidth, TRUE,
					End,
				End,
				Child, PREFS_Page = VGroup,
					MUIA_Background, MUII_PageBack,
				End,
			End,
			Child, HGroup,
				Child, PREFS_Save = Button( GetString(MSG_SAVE) ),
				Child, PREFS_Use = Button( GetString(MSG_USE) ),
				Child, PREFS_Cancel = Button( GetString(MSG_CANCEL) ),
			End,
		End,
	End;
	DoMethod(APP_Main,MUIM_MultiSet,MUIA_CycleChain,TRUE, PREFS_List, PREFS_Save, PREFS_Use, PREFS_Cancel, NULL );

	DoMethod( WIN_Prefs,		MUIM_Notify, MUIA_Window_CloseRequest,	TRUE, MUIV_Notify_Application, 3, MUIM_CallHook, &PREFS_Close_Hook, 0 );
	DoMethod( PREFS_List,	MUIM_Notify, MUIA_List_Active,			MUIV_EveryTime, MUIV_Notify_Application, 3, MUIM_CallHook, &PREFS_List_Hook, MUIV_TriggerValue );
	DoMethod( PREFS_Save,	MUIM_Notify, MUIA_Pressed,					FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &PREFS_Close_Hook, 2 );
	DoMethod( PREFS_Use,		MUIM_Notify, MUIA_Pressed,					FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &PREFS_Close_Hook, 1 );
	DoMethod( PREFS_Cancel,	MUIM_Notify, MUIA_Pressed,					FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &PREFS_Close_Hook, 0 );
	return(new);
}
