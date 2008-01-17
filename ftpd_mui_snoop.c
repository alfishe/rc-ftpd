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
#include	<exec/memory.h>
#include	<netdb.h>
#include	<netinet/in.h>
#include	<string.h>
#include	<sys/socket.h>
#include	<sys/time.h>
#include <libraries/gadtools.h>
#include <libraries/asl.h>
#include <libraries/mui.h>
#include	<mui/MUIundoc.h>
#include	<mui/NList_mcc.h>
#include	<mui/NListview_mcc.h>
#include	<workbench/startup.h>

#include	<clib/alib_protos.h>
#include	<clib/muimaster_protos.h>

#include	<inline/exec.h>
#include	<inline/socket.h>
#include	<inline/dos.h>

#include "ftpd.h"

extern struct Hook MAIN_Kick_Hook;
extern struct Hook MAIN_Abort_Hook;
extern struct Hook MAIN_Message_Hook;

static unsigned long __saveds SNOOP_Log_Func( char **array __asm__("a2"), struct ftpdata **obj __asm__("a1") )
{
	struct ftpdata *ftp, *ftp2;

	if ( ( ftp = *obj ) )
	{
		long level;

		get( ftp->Log, MUIA_Cycle_Active, &level );
		if ( ( ftp->loglevel = (short)level ) )set( ftp->List, MUIA_ShowMe, TRUE );
		DoMethod( MAIN_List, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &ftp2 );
		if ( ftp==ftp2 )nnset( MAIN_Log, MUIA_Cycle_Active, level );
		update_list(ftp);
		update_ftp(ftp);
	}
	return(0);
}
struct Hook SNOOP_Log_Hook = { {NULL,NULL},&SNOOP_Log_Func,NULL,NULL };

void start_log( struct ftpdata *ftp )
{
	char	*name;
	Object	*msg;

	if ( ftp->Win )
	{
		set( ftp->Log, MUIA_Cycle_Active, ftp->loglevel );
//		DoMethod( APP_Main, MUIM_CallHook, &SNOOP_Log_Hook, ftp );
		return;
	}
	name = IS_SET(ftp->flags,FLAG_ONLINE)?ftp->user->user:GetString(MSG_LOGIN);
	ftp->Win = WindowObject,
		MUIA_Window_Title, GetString(MSG_SNOOP_TITLE),
		MUIA_Window_ID, MAKE_ID('*',name[0],name[1],name[2]),
		MUIA_Window_Menustrip, MenustripObject,
			Child, MenuObjectT(GetString(MSG_MENU_PROJECT)),
				Child, ftp->Clear = MenuitemObject,
					MUIA_Menuitem_Title, GetString(MSG_SNOOP_CLEAR),
				End,
			End,
		End,
		WindowContents, VGroup,
			MUIA_Background, MUII_WindowBack,
			MUIA_Frame, MUIV_Frame_None,
			Child, ftp->Info = TextObject,
				TextFrame,
				MUIA_Text_Contents, "\n\n\n",
				MUIA_Text_PreParse, "\33c",
				MUIA_Background, MUII_TextBack,
			End,
			Child, ftp->List = NListviewObject,
				MUIA_ShowMe, FALSE,
				MUIA_NListview_NList, NListObject,
					VirtualFrame,
					MUIA_NList_ConstructHook, MUIV_NList_ConstructHook_String,
					MUIA_NList_DestructHook, MUIV_NList_DestructHook_String,
					MUIA_NList_AutoCopyToClip, TRUE,
					MUIA_NList_Exports, 0,
					MUIA_NList_TypeSelect, MUIV_NList_TypeSelect_Char,
				End,
			End,
			Child, HGroup,
				Child, HGroup,
					MUIA_ShortHelp, GetString(MSG_SHORT_KICKABORT),
					Child, ftp->Kick = Button( GetString(MSG_KICK) ),
					Child, ftp->Abort = Button( GetString(MSG_ABORT) ),
				End,
				Child, ftp->Log = CycleObject,
					MUIA_Font, MUIV_Font_Button,
					MUIA_Cycle_Entries, log,
				End,
			End,
			Child, msg = StringObject,
				MUIA_HelpNode, "MESS",
				MUIA_ShortHelp, GetString(MSG_SHORT_MESSAGE),
				StringFrame,
				MUIA_String_MaxLen, 512,
			End,
		End,
	End;
	if ( !ftp->Win )ftp->Info = ftp->List = ftp->Clear = ftp->Kick = ftp->Abort = ftp->Log = NULL;
	else
	{
		DoMethod(APP_Main,MUIM_MultiSet,MUIA_CycleChain,TRUE, ftp->List, ftp->Kick, ftp->Abort, ftp->Kick, ftp->Log, NULL );

		DoMethod( APP_Main, MUIM_MultiSet, MUIA_Disabled, TRUE, ftp->Abort, ftp->Clear, NULL );
		DoMethod( ftp->Win,		MUIM_Notify, MUIA_Window_CloseRequest,	TRUE, MUIV_Notify_Self, 3, MUIM_NoNotifySet, MUIA_Window_Open, FALSE);

//		DoMethod( ftp->Clear,	MUIM_Notify, MUIA_Pressed,					FALSE, ftp->List, 1, MUIM_NList_Clear );
		DoMethod( ftp->Clear,	MUIM_Notify, MUIA_Menuitem_Trigger,		MUIV_EveryTime, ftp->List, 1, MUIM_NList_Clear );

		DoMethod( ftp->Kick,		MUIM_Notify, MUIA_Pressed,					FALSE, APP_Main, 3, MUIM_CallHook, &MAIN_Kick_Hook, ftp );
		DoMethod( ftp->Abort,	MUIM_Notify, MUIA_Pressed,					FALSE, APP_Main, 3, MUIM_CallHook, &MAIN_Abort_Hook, ftp );
		DoMethod( ftp->Log,		MUIM_Notify, MUIA_Cycle_Active,			MUIV_EveryTime, APP_Main, 3, MUIM_CallHook, &SNOOP_Log_Hook, ftp );
		DoMethod( ftp->List,		MUIM_Notify, MUIA_ShowMe,					TRUE, ftp->Clear, 3, MUIM_Set, MUIA_Disabled, FALSE );
		DoMethod( msg,				MUIM_Notify, MUIA_String_Acknowledge,	MUIV_EveryTime, MUIV_Notify_Self, 3, MUIM_CallHook, &MAIN_Message_Hook, ftp );
		set( ftp->Log, MUIA_Cycle_Active, ftp->loglevel );
		DoMethod( APP_Main, OM_ADDMEMBER, ftp->Win );
		update_list(ftp);
		update_ftp(ftp);
	}
}
