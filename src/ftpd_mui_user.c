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
#include	<string.h>
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
#include	<inline/dos.h>

#include "ftpd.h"

Object	*USER_List;
Object	*USER_Pass;
Object	*USER_Random;
Object	*USER_Name;
Object	*USER_Log;
Object	*USER_Add;
Object	*USER_Delete;
Object	*USER_Sort;
Object	*USER_Sessions;
Object	*USER_Sessions_Up;
Object	*USER_Sessions_Down;

void SetUsers( void )
{
	struct userdata *user;

	DoMethod( USER_List, MUIM_NList_Clear );
	set( USER_List, MUIA_NList_Quiet, TRUE );
	for ( user = user_list ; user ; user = user->next )
		DoMethod( USER_List, MUIM_NList_InsertSingle, &user, MUIV_NList_Insert_Bottom );
	set( USER_List, MUIA_NList_Quiet, FALSE );
}

void copy_user( struct userdata *to, struct userdata *from )
{
	strncpy( to->user, from->user, 16 );
	strncpy( to->pass, from->pass, 16 );
	to->access		= from->access;
	to->sessions	= from->sessions;
	to->loglevel	= from->loglevel;
	to->edit			= NULL;
}

void GetUsers( void )
{
	struct ftpdata *ftp;
	struct userdata *user, *fake, *fake_list;
	long	i;

	fake_list = user_list;
	user_list = NULL;
	for ( i = 0 ; ; i++ )
	{
		DoMethod( USER_List, MUIM_NList_GetEntry, i, &fake );
		if ( !fake )break;
		user = get_user( i );
		copy_user( user, fake );
	}
	for ( ftp = ftp_list ; ftp ; ftp = ftp->next )
	{
		if ( !ftp->user || ftp->user==(void*)-1 )continue;
		for ( user = user_list ; user ; user = user->next )if ( !stricmp( ftp->user->user, user->user ) )break;
		if ( user )ftp->user = user;
		else
		{
			output( ftp, REPLY_421, 421 );
			SET_BIT(ftp->flags,FLAG_QUIT);
		}
	}
	for ( fake = fake_list ; fake ; fake = user )
	{
		user = fake->next;
		FreePooled( myPool, fake, sizeof(struct userdata) );
	}
}

static unsigned long __saveds USER_List_Construct_Func( struct userdata **obj __asm__("a1") )
{
	struct userdata *user, *fake;

	if ( ( fake = AllocPooled( myPool, sizeof(struct userdata) ) ) )
	{
		if ( obj != (void*)-1 && ( user = *obj ) )
		{
			copy_user( fake, user );
			fake->edit = user;
			user->edit = fake;
		}
		else fake->access = 1;
	}
	return((ULONG)fake);
}
struct Hook USER_List_Construct_Hook = { {NULL,NULL},&USER_List_Construct_Func,NULL,NULL };

static unsigned long __saveds USER_List_Destruct_Func( struct userdata **obj __asm__("a1") )
{
	struct userdata *user;
	user = *obj;
	if ( user->edit )user->edit->edit = NULL;
	FreePooled( myPool, user, sizeof(struct userdata) );
	return(0);
}
struct Hook USER_List_Destruct_Hook = { {NULL,NULL},&USER_List_Destruct_Func,NULL,NULL };

static unsigned long __saveds USER_List_Compare_Func( struct userdata *user1 __asm__("a1"), struct userdata *user2 __asm__("a2") )
{
	if ( *user1->user && !*user2->user )return(-1);
	if ( *user2->user && !*user1->user )return(1);
	if ( *user1->pass && !*user2->pass )return(-1);
	if ( *user2->pass && !*user1->pass )return(1);
	return(stricmp(user1->user,user2->user));
}
struct Hook USER_List_Compare_Hook = { {NULL,NULL},&USER_List_Compare_Func,NULL,NULL };

static unsigned long __saveds USER_List_Active_Func( char **array __asm__("a2") )
{
	struct userdata *user;

	DoMethod( USER_List, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &user );
	if ( user )
	{
		nnset( USER_Delete, MUIA_Disabled, (*user->user?TRUE:FALSE) );
		nnset( USER_Random, MUIA_Disabled, (*user->pass?TRUE:FALSE) );
		nnset( USER_Pass, MUIA_String_Contents, user->pass );
		nnset( USER_Name, MUIA_String_Contents, user->user );
		nnset( USER_Log, MUIA_Cycle_Active, user->loglevel );
		nnset( USER_Sessions, MUIA_Numeric_Value, user->sessions );
	}
	DoMethod(APP_Main,MUIM_MultiSet,MUIA_Disabled,user?FALSE:TRUE, USER_Sessions, USER_Sessions_Up, USER_Sessions_Down, USER_Pass, USER_Name, USER_Log, user?NULL:USER_Random, NULL );
	return(0);
}
struct Hook USER_List_Active_Hook = { {NULL,NULL},&USER_List_Active_Func,NULL,NULL };

char	lu[]	= "%lu";

static unsigned long __saveds USER_List_Func( char **array __asm__("a2"), struct userdata *user __asm__("a1") )
{
	static char buf[12];
	int i;
	if ( !user )
	{
		for ( i = 0 ; i < 16 ; i++ )*array++ = groups[i];
		*array++ = GetString(MSG_USER_LIST_USERNAME);
		*array++ = GetString(MSG_USER_LIST_PASSWORD);
		*array++ = GetString(MSG_USER_LIST_MAX);
		*array++ = GetString(MSG_USER_LIST_LOG);
	}
	else
	{
		for ( i = 0 ; i < 16 ; i++ )
		{
			array[DISPLAY_ARRAY_MAX] = "\ec";
			*array++ = (IS_SET(user->access,(1<<i))?"x":" ");
		}
		*array++ = (*user->user?user->user:GetString(MSG_USER_LIST_NONAME));
		*array++ = (GetString(*user->pass?MSG_USER_LIST_PASS:MSG_USER_LIST_NOPASS));
		if ( user->sessions )
		{
			sprintf( buf, lu, user->sessions );
			*array++ = buf;
		}
		else *array++ = "-";
		*array++ = log[user->loglevel];
	}
	return(0);
}
struct Hook USER_List_Hook = { {NULL,NULL},&USER_List_Func,NULL,NULL };

static unsigned long __saveds USER_Log_Func( char **array __asm__("a2"), long *value __asm__("a1") )
{
	struct userdata *user;

	DoMethod( USER_List, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &user );
	if ( user )
	{
		user->loglevel = (short)(*value);
		DoMethod( USER_List, MUIM_NList_Redraw, MUIV_NList_Redraw_Active );
	}
	return(0);
}
struct Hook USER_Log_Hook = { {NULL,NULL},&USER_Log_Func,NULL,NULL };

static unsigned long __saveds USER_Name_Func( char **array __asm__("a2"), char **name __asm__("a1") )
{
	struct userdata *user;

	DoMethod( USER_List, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &user );
	if ( user )
	{
		strncpy( user->user, (char*)*name, 16 );
		set( USER_Delete, MUIA_Disabled, (*user->user?TRUE:FALSE) );
		set( USER_Random, MUIA_Disabled, (*user->pass?TRUE:FALSE) );
		DoMethod( USER_List, MUIM_NList_Redraw, MUIV_NList_Redraw_Active );
	}
	return(0);
}
struct Hook USER_Name_Hook = { {NULL,NULL},&USER_Name_Func,NULL,NULL };

static unsigned long __saveds USER_Pass_Func( char **array __asm__("a2"), char **pass __asm__("a1") )
{
	struct userdata *user;

	DoMethod( USER_List, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &user );
	if ( user )
	{
		strncpy( user->pass, (char*)*pass, 16 );
		DoMethod( USER_List, MUIM_NList_Redraw, MUIV_NList_Redraw_Active );
	}
	return(0);
}
struct Hook USER_Pass_Hook = { {NULL,NULL},&USER_Pass_Func,NULL,NULL };

static unsigned long __saveds USER_Access_Func( char **array __asm__("a2") )
{
	struct userdata *user;

	DoMethod( USER_List, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &user );
	if ( user )
	{
		long	column;
		get( USER_List, MUIA_NList_ClickColumn, &column );
		if ( column>=0 && column<=15 )
		{
			TOGGLE_BIT(user->access,(1<<column));
			DoMethod( USER_List, MUIM_NList_Redraw, MUIV_NList_Redraw_Active );
		}
	}
	return(0);
}
struct Hook USER_Access_Hook = { {NULL,NULL},&USER_Access_Func,NULL,NULL };

unsigned long Seed;

unsigned long Rand( unsigned long max )
{
	unsigned long a = Seed;
	unsigned short i = max - 1;
	do
	{
		unsigned long b = a;

		a <<= 1;
		if ( (long)b <= 0 )a ^= 0x1d872b41;
	}while( ( i >>= 1 ) );
	Seed = a;
	if ( (unsigned short)max )return(unsigned short)( (unsigned short)a * (unsigned short)max>>16 );
	return(a);
}

static unsigned long __saveds USER_Random_Func( char **array __asm__("a2") )
{
	char	newpass[9];
	int	i;

	struct DateStamp ds;

	DateStamp( &ds );
	Seed ^= (ds.ds_Days ^ ds.ds_Minute ^ ds.ds_Tick );

	for ( i = 0 ; i < 8 ; i++ )
	{
		switch( Rand(3) )
		{
			case 0:
				newpass[i] = ( 'a' + (char)Rand(26) );
				continue;
			case 1:
				newpass[i] = ( 'A' + (char)Rand(26) );
				continue;
			case 2:
				newpass[i] = ( '0' + (char)Rand(10) );
				continue;
		}
	}
	newpass[i] = '\0';
	set( USER_Pass, MUIA_String_Contents, newpass );
	return(0);
}
struct Hook USER_Random_Hook = { {NULL,NULL},&USER_Random_Func,NULL,NULL };

static unsigned long __saveds USER_Sessions_Func( char **array __asm__("a2") )
{
	struct userdata *user;
	long	value;

	DoMethod( USER_List, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &user );
	if ( user )
	{
		get( USER_Sessions, MUIA_Numeric_Value, &value );
		user->sessions = (unsigned short)value;
		DoMethod( USER_List, MUIM_NList_Redraw, MUIV_NList_Redraw_Active );
	}
	return(0);
}
struct Hook USER_Sessions_Hook = { {NULL,NULL},&USER_Sessions_Func,NULL,NULL };

Object *UserGroup( void )
{
	Object *new;

	new = VGroup,
		MUIA_HelpNode, "USER",
		InnerSpacing(4,4),
		ReadListFrame,
		Child, USER_List = NListviewObject,
			MUIA_NListview_NList, NListObject,
				VirtualFrame,
				MUIA_NList_Format, ",,,BAR SBAR,,,,BAR SBAR,,,,BAR SBAR,,,,BAR,BAR,P=\ec BAR,P=\ec BAR,",
				MUIA_NList_DragSortable, TRUE,
				MUIA_NList_ConstructHook, &USER_List_Construct_Hook,
				MUIA_NList_CompareHook, &USER_List_Compare_Hook,
				MUIA_NList_DestructHook, &USER_List_Destruct_Hook,
				MUIA_NList_DisplayHook, &USER_List_Hook,
				MUIA_NList_Title, TRUE,
				MUIA_NList_Exports, MUIV_NList_Exports_Cols,
				MUIA_NList_MinColSortable, 64,
			End,
		End,
		Child, HGroup,
			Child, USER_Add = Button( GetString(MSG_ADD) ),
			Child, USER_Delete = Button( GetString(MSG_REMOVE) ),
			Child, USER_Sort = Button( GetString(MSG_SORT) ),
		End,
		Child, HGroup,
			Child, SmallText( GetString(MSG_USER_USER) ),
			Child, USER_Name = StringObject,
				StringFrame,
				MUIA_String_MaxLen, 16,
			End,
			Child, SmallText( GetString(MSG_USER_PASS) ),
			Child, HGroup,
				GroupSpacing(0),
				Child, USER_Pass = StringObject,
					StringFrame,
					MUIA_String_MaxLen, 16,
				End,
				Child, USER_Random = SmallButton( "?" ),
			End,
			Child, SmallText( GetString(MSG_USER_MAX) ),
			Child, HGroup,
				GroupSpacing(0),
				Child, USER_Sessions = NumericbuttonObject,
					MUIA_Numeric_Min, 0,
					MUIA_Numeric_Max, 99,
					MUIA_Numeric_Default, 1,
				End,
				Child, VGroup,
					GroupSpacing(0),
					Child, USER_Sessions_Up = MiniButton(),
					Child, USER_Sessions_Down = MiniButton(),
				End,
			End,
			Child, USER_Log = CycleObject,
				MUIA_Weight, 0,
				MUIA_Font, MUIV_Font_Button,
				MUIA_Cycle_Entries, log,
				MUIA_Disabled, TRUE,
			End,
		End,
	End;
	DoMethod(APP_Main,MUIM_MultiSet,MUIA_CycleChain,TRUE, USER_List, USER_Add, USER_Delete, USER_Name, USER_Pass, USER_Random, USER_Sessions, USER_Log, NULL );

	DoMethod( new,						MUIM_MultiSet, MUIA_Disabled,TRUE, USER_Pass, USER_Random, USER_Name, USER_Log, USER_Delete, USER_Sessions, USER_Sessions_Up, USER_Sessions_Down, NULL );
	DoMethod( USER_List,				MUIM_Notify, MUIA_NList_DoubleClick,	MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_CallHook, &USER_Access_Hook );
	DoMethod( USER_List,				MUIM_Notify, MUIA_NList_Active,			MUIV_EveryTime, MUIV_Notify_Application, 3, MUIM_CallHook, &USER_List_Active_Hook, MUIV_TriggerValue );
	DoMethod( USER_Add,				MUIM_Notify, MUIA_Pressed,					FALSE, USER_List, 3, MUIM_NList_InsertSingle, -1, MUIV_NList_Insert_Bottom );
	DoMethod( USER_Add,				MUIM_Notify, MUIA_Pressed,					FALSE, USER_List, 3, MUIM_Set, MUIA_NList_Active, MUIV_NList_Active_Bottom );
	DoMethod( USER_Delete,			MUIM_Notify, MUIA_Pressed,					FALSE, USER_List, 2, MUIM_NList_Remove, MUIV_NList_Remove_Active );
	DoMethod( USER_Sort,				MUIM_Notify, MUIA_Pressed,					FALSE, USER_List, 1, MUIM_NList_Sort );
	DoMethod( USER_Name,				MUIM_Notify, MUIA_String_Contents,		MUIV_EveryTime, MUIV_Notify_Application, 3, MUIM_CallHook, &USER_Name_Hook, MUIV_TriggerValue );
	DoMethod( USER_Name,				MUIM_Notify, MUIA_String_Acknowledge,	MUIV_EveryTime, MUIV_Notify_Window, 3, MUIM_Set, MUIA_Window_ActiveObject, USER_Pass );
	DoMethod( USER_Pass,				MUIM_Notify, MUIA_String_Contents,		MUIV_EveryTime, MUIV_Notify_Application, 3, MUIM_CallHook, &USER_Pass_Hook, MUIV_TriggerValue );
	DoMethod( USER_Random,			MUIM_Notify, MUIA_Pressed,					FALSE, MUIV_Notify_Application, 2, MUIM_CallHook, &USER_Random_Hook );
	DoMethod( USER_Sessions,		MUIM_Notify, MUIA_Numeric_Value,			MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_CallHook, &USER_Sessions_Hook );
	DoMethod( USER_Sessions_Up,	MUIM_Notify, MUIA_Pressed,					FALSE, USER_Sessions, 2, MUIM_Numeric_Increase, 1 );
	DoMethod( USER_Sessions_Down,	MUIM_Notify, MUIA_Pressed,					FALSE, USER_Sessions, 2, MUIM_Numeric_Decrease, 1 );
	DoMethod( USER_Log,				MUIM_Notify, MUIA_Cycle_Active,			MUIV_EveryTime, MUIV_Notify_Application, 3, MUIM_CallHook, &USER_Log_Hook, MUIV_TriggerValue );
	SetUsers();
	return(new);
}
