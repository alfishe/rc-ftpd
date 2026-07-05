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
#include	<inline/socket.h>
#include	<inline/dos.h>

#include "ftpd.h"

Object	*WIN_Path;
Object	*PATH_List;
Object	*PATH_Path;
Object	*PATH_Alias;
Object	*PATH_Add;
Object	*PATH_Delete;
Object	*PATH_Sort;
Object	*PATH_FRead[16];
Object	*PATH_FWrite[16];
Object	*PATH_FDelete[16];
Object	*PATH_SubDirs[16];

void SetPath( void )
{
	struct accessdata *access;

	DoMethod( PATH_List, MUIM_NList_Clear );
	set( PATH_List, MUIA_NList_Quiet, TRUE );
	for ( access = access_list ; access ; access = access->next )
		DoMethod( PATH_List, MUIM_NList_InsertSingle, &access, MUIV_NList_Insert_Bottom );
	set( PATH_List, MUIA_NList_Quiet, FALSE );
}

void copy_access( struct accessdata *to, struct accessdata *from )
{
	strncpy( to->alias, from->alias, 512 );
	strncpy( to->path, from->path, 512 );
	to->read		= from->read;
	to->write	= from->write;
	to->delete	= from->delete;
	to->subdirs	= from->subdirs;
	to->edit		= NULL;
}

void GetPath( void )
{
	struct ftpdata *ftp;
	struct accessdata *access, *fake, *fake_list;
	long	i;

	fake_list = access_list;
	access_list = NULL;
	for ( i = 0 ; ; i++ )
	{
		DoMethod( PATH_List, MUIM_NList_GetEntry, i, &fake );
		if ( !fake )break;
		access = get_access( i );
		copy_access( access, fake );
	}
	for ( ftp = ftp_list ; ftp ; ftp = ftp->next )
	{
		if ( !ftp->access )continue;
		for ( access = access_list ; access ; access = access->next )
			if ( !stricmp(ftp->access->alias,access->alias)
			  && IS_SET((ftp->user?ftp->user->access:1),(access->read|access->write|access->delete)) )
			break;
		if ( access )ftp->access = access;
		else
		{
			output( ftp, REPLY_421, 421 );
			SET_BIT(ftp->flags,FLAG_QUIT);
		}
	}
	for ( fake = fake_list ; fake ; fake = access )
	{
		access = fake->next;
		FreePooled( myPool, fake, sizeof(struct accessdata) );
	}
}

static unsigned long __saveds PATH_List_Construct_Func( struct accessdata **obj __asm__("a1") )
{
	struct accessdata *access, *fake;

	if ( ( fake = AllocPooled( myPool, sizeof(struct accessdata) ) ) && obj != (void*)-1 && ( access = *obj ) )
	{
		copy_access( fake, access );
		fake->edit = access;
		access->edit = fake;
	}
	return((ULONG)fake);
}
struct Hook PATH_List_Construct_Hook = { {NULL,NULL},&PATH_List_Construct_Func,NULL,NULL };

static unsigned long __saveds PATH_List_Destruct_Func( struct accessdata **obj __asm__("a1") )
{
	struct accessdata *access;
	access = *obj;
	if ( access->edit )access->edit->edit = NULL;
	FreePooled( myPool, access, sizeof(struct accessdata) );
	return(0);
}
struct Hook PATH_List_Destruct_Hook = { {NULL,NULL},&PATH_List_Destruct_Func,NULL,NULL };

static unsigned long __saveds PATH_List_Compare_Func( struct accessdata *access1 __asm__("a1"), struct accessdata *access2 __asm__("a2") )
{
	long i;
	if ( !( i = stricmp(access1->alias,access2->alias) ) )i = stricmp(access1->path,access2->path);
	return(i);
}
struct Hook PATH_List_Compare_Hook = { {NULL,NULL},&PATH_List_Compare_Func,NULL,NULL };

static unsigned long __saveds PATH_List_Func( char **array __asm__("a2"), struct accessdata *access __asm__("a1") )
{
	int i;
	if ( !access )
	{
		for ( i = 0 ; i < 16 ; i++ )
		{
			array[DISPLAY_ARRAY_MAX] = "\ec";
			*array++ = groups[i];
		}
		*array++ = GetString(MSG_PATH_LIST_ALIAS);
		*array++ = GetString(MSG_PATH_LIST_PATH);
	}
	else
	{
		for ( i = 0 ; i < 16 ; i++ )
		{
			if ( !IS_SET(access->subdirs,(1<<i)) )array[DISPLAY_ARRAY_MAX] = "\ec";
			else array[DISPLAY_ARRAY_MAX] = "\ec\eb";
			if ( IS_SET(access->delete,(1<<i)) )
				*array++ = (IS_SET(access->read,(1<<i))?IS_SET(access->write,(1<<i))?"X":"R":IS_SET(access->write,(1<<i))?"W":"D");
			else
				*array++ = (IS_SET(access->read,(1<<i))?IS_SET(access->write,(1<<i))?"x":"r":IS_SET(access->write,(1<<i))?"w":IS_SET(access->subdirs,(1<<i))?"+":" ");
		}
		*array++ = (*access->alias?access->alias:GetString(MSG_PATH_NOALIAS));
		*array++ = (*access->path?access->path:GetString(MSG_PATH_NOPATH));
	}
	return(0);
}
struct Hook PATH_List_Hook = { {NULL,NULL},&PATH_List_Func,NULL,NULL };

static unsigned long __saveds PATH_Alias_Func( char **array __asm__("a2"), char **alias __asm__("a1") )
{
	struct accessdata *access;

	DoMethod( PATH_List, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &access );
	if ( access )
	{
		int i;
		if ( !((char*)*alias)[0] || ((char*)*alias)[0]=='/' )strncpy( access->alias, (char*)*alias, 512 );
		else
		{
			access->alias[0] = '/';
			strncpy( &access->alias[1], (char*)*alias, 511 );
		}
		i = strlen( access->alias );
		while( i-->1 && ( access->alias[i]=='/' || access->alias[i]==' ' ) )access->alias[i] = '\0';
		set( PATH_Delete, MUIA_Disabled, (*access->alias?TRUE:FALSE) );
		DoMethod( PATH_List, MUIM_NList_Redraw, MUIV_NList_Redraw_Active );
	}
	return(0);
}
struct Hook PATH_Alias_Hook = { {NULL,NULL},&PATH_Alias_Func,NULL,NULL };

static unsigned long __saveds PATH_Path_Func( char **array __asm__("a2"), char **path __asm__("a1") )
{
	struct accessdata *access;

	DoMethod( PATH_List, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &access );
	if ( access )
	{
		strncpy( access->path, (char*)*path, 512 );
		strip_trailing( access->path );
		DoMethod( PATH_List, MUIM_NList_Redraw, MUIV_NList_Redraw_Active );
	}
	return(0);
}
struct Hook PATH_Path_Hook = { {NULL,NULL},&PATH_Path_Func,NULL,NULL };

static unsigned long __saveds PATH_List_Active_Func( char **array __asm__("a2") )
{
	struct accessdata *access;
	long i;

	DoMethod( PATH_List, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &access );
	if ( access )
	{
		nnset( PATH_Alias, MUIA_String_Contents, access->alias );
		nnset( PATH_Path, MUIA_String_Contents, access->path );
		set( PATH_Delete, MUIA_Disabled, (*access->alias?TRUE:FALSE) );
	}
	DoMethod(APP_Main,MUIM_MultiSet,MUIA_Disabled,access?FALSE:TRUE, PATH_Alias, PATH_Path, NULL );
	for ( i = 0 ; i < 16 ; i++ )
	{
		DoMethod( APP_Main, MUIM_MultiSet, MUIA_Disabled, access?FALSE:TRUE, PATH_FRead[i], PATH_FWrite[i], PATH_FDelete[i], PATH_SubDirs[i], NULL );
		if ( access )
		{
			nnset( PATH_FRead[i], MUIA_Selected, IS_SET(access->read,(1<<i)) );
			nnset( PATH_FWrite[i], MUIA_Selected, IS_SET(access->write,(1<<i)) );
			nnset( PATH_FDelete[i], MUIA_Selected, IS_SET(access->delete,(1<<i)) );
			nnset( PATH_SubDirs[i], MUIA_Selected, IS_SET(access->subdirs,(1<<i)) );
		}
	}
	return(0);
}
struct Hook PATH_List_Active_Hook = { {NULL,NULL},&PATH_List_Active_Func,NULL,NULL };

static unsigned long __saveds PATH_Read_Func( char **array __asm__("a2"), long *flag __asm__("a1") )
{
	struct accessdata *access;

	DoMethod( PATH_List, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &access );
	if ( access )TOGGLE_BIT(access->read,*flag);
	DoMethod( PATH_List, MUIM_NList_Redraw, MUIV_NList_Redraw_Active );
	return(0);
}
struct Hook PATH_Read_Hook = { {NULL,NULL},&PATH_Read_Func,NULL,NULL };

static unsigned long __saveds PATH_Write_Func( char **array __asm__("a2"), long *flag __asm__("a1") )
{
	struct accessdata *access;

	DoMethod( PATH_List, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &access );
	if ( access )TOGGLE_BIT(access->write,*flag);
	DoMethod( PATH_List, MUIM_NList_Redraw, MUIV_NList_Redraw_Active );
	return(0);
}
struct Hook PATH_Write_Hook = { {NULL,NULL},&PATH_Write_Func,NULL,NULL };

static unsigned long __saveds PATH_Delete_Func( char **array __asm__("a2"), long *flag __asm__("a1") )
{
	struct accessdata *access;

	DoMethod( PATH_List, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &access );
	if ( access )TOGGLE_BIT(access->delete,*flag);
	DoMethod( PATH_List, MUIM_NList_Redraw, MUIV_NList_Redraw_Active );
	return(0);
}
struct Hook PATH_Delete_Hook = { {NULL,NULL},&PATH_Delete_Func,NULL,NULL };

static unsigned long __saveds PATH_SubDirs_Func( char **array __asm__("a2"), long *flag __asm__("a1") )
{
	struct accessdata *access;

	DoMethod( PATH_List, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &access );
	if ( access )TOGGLE_BIT(access->subdirs,*flag);
	DoMethod( PATH_List, MUIM_NList_Redraw, MUIV_NList_Redraw_Active );
	return(0);
}
struct Hook PATH_SubDirs_Hook = { {NULL,NULL},&PATH_SubDirs_Func,NULL,NULL };

Object *PathGroup( void )
{
	Object *new;
	long i;

	new = VGroup,
		MUIA_HelpNode, "GROU",
		InnerSpacing(4,4),
		ReadListFrame,
		Child, PATH_List = NListviewObject,
			MUIA_NListview_NList, NListObject,
				VirtualFrame,
				MUIA_NList_Format, ",,,BAR SBAR,,,,BAR SBAR,,,,BAR SBAR,,,,BAR,BAR,BAR",
				MUIA_NList_DragSortable, TRUE,
				MUIA_NList_ConstructHook, &PATH_List_Construct_Hook,
				MUIA_NList_CompareHook, &PATH_List_Compare_Hook,
				MUIA_NList_DestructHook, &PATH_List_Destruct_Hook,
				MUIA_NList_DisplayHook, &PATH_List_Hook,
				MUIA_NList_Title, TRUE,
				MUIA_NList_Exports, MUIV_NList_Exports_Cols,
				MUIA_NList_MinColSortable, 64,
			End,
		End,
		Child, HGroup,
			Child, PATH_Add = Button( GetString(MSG_ADD) ),
			Child, PATH_Delete = Button( GetString(MSG_REMOVE) ),
			Child, PATH_Sort = Button( GetString(MSG_SORT) ),
		End,
		Child, ColGroup(18),
			GroupSpacing(0),
			Child, SmallText( GetString(MSG_PATH_READ) ),
			Child, PATH_FRead[0] = SmallToggle( "0" ),
			Child, PATH_FRead[1] = SmallToggle( "1" ),
			Child, PATH_FRead[2] = SmallToggle( "2" ),
			Child, PATH_FRead[3] = SmallToggle( "3" ),
			Child, PATH_FRead[4] = SmallToggle( "4" ),
			Child, PATH_FRead[5] = SmallToggle( "5" ),
			Child, PATH_FRead[6] = SmallToggle( "6" ),
			Child, PATH_FRead[7] = SmallToggle( "7" ),
			Child, PATH_FRead[8] = SmallToggle( "8" ),
			Child, PATH_FRead[9] = SmallToggle( "9" ),
			Child, PATH_FRead[10] = SmallToggle( "A" ),
			Child, PATH_FRead[11] = SmallToggle( "B" ),
			Child, PATH_FRead[12] = SmallToggle( "C" ),
			Child, PATH_FRead[13] = SmallToggle( "D" ),
			Child, PATH_FRead[14] = SmallToggle( "E" ),
			Child, PATH_FRead[15] = SmallToggle( "F" ),
			Child, HVSpace,
			Child, SmallText( GetString(MSG_PATH_WRITE) ),
			Child, PATH_FWrite[0] = SmallToggle( "0" ),
			Child, PATH_FWrite[1] = SmallToggle( "1" ),
			Child, PATH_FWrite[2] = SmallToggle( "2" ),
			Child, PATH_FWrite[3] = SmallToggle( "3" ),
			Child, PATH_FWrite[4] = SmallToggle( "4" ),
			Child, PATH_FWrite[5] = SmallToggle( "5" ),
			Child, PATH_FWrite[6] = SmallToggle( "6" ),
			Child, PATH_FWrite[7] = SmallToggle( "7" ),
			Child, PATH_FWrite[8] = SmallToggle( "8" ),
			Child, PATH_FWrite[9] = SmallToggle( "9" ),
			Child, PATH_FWrite[10] = SmallToggle( "A" ),
			Child, PATH_FWrite[11] = SmallToggle( "B" ),
			Child, PATH_FWrite[12] = SmallToggle( "C" ),
			Child, PATH_FWrite[13] = SmallToggle( "D" ),
			Child, PATH_FWrite[14] = SmallToggle( "E" ),
			Child, PATH_FWrite[15] = SmallToggle( "F" ),
			Child, HVSpace,
			Child, SmallText( GetString(MSG_PATH_DELETE) ),
			Child, PATH_FDelete[0] = SmallToggle( "0" ),
			Child, PATH_FDelete[1] = SmallToggle( "1" ),
			Child, PATH_FDelete[2] = SmallToggle( "2" ),
			Child, PATH_FDelete[3] = SmallToggle( "3" ),
			Child, PATH_FDelete[4] = SmallToggle( "4" ),
			Child, PATH_FDelete[5] = SmallToggle( "5" ),
			Child, PATH_FDelete[6] = SmallToggle( "6" ),
			Child, PATH_FDelete[7] = SmallToggle( "7" ),
			Child, PATH_FDelete[8] = SmallToggle( "8" ),
			Child, PATH_FDelete[9] = SmallToggle( "9" ),
			Child, PATH_FDelete[10] = SmallToggle( "A" ),
			Child, PATH_FDelete[11] = SmallToggle( "B" ),
			Child, PATH_FDelete[12] = SmallToggle( "C" ),
			Child, PATH_FDelete[13] = SmallToggle( "D" ),
			Child, PATH_FDelete[14] = SmallToggle( "E" ),
			Child, PATH_FDelete[15] = SmallToggle( "F" ),
			Child, HVSpace,
			Child, SmallText( GetString(MSG_PATH_SUBDIRS) ),
			Child, PATH_SubDirs[0] = SmallToggle( "0" ),
			Child, PATH_SubDirs[1] = SmallToggle( "1" ),
			Child, PATH_SubDirs[2] = SmallToggle( "2" ),
			Child, PATH_SubDirs[3] = SmallToggle( "3" ),
			Child, PATH_SubDirs[4] = SmallToggle( "4" ),
			Child, PATH_SubDirs[5] = SmallToggle( "5" ),
			Child, PATH_SubDirs[6] = SmallToggle( "6" ),
			Child, PATH_SubDirs[7] = SmallToggle( "7" ),
			Child, PATH_SubDirs[8] = SmallToggle( "8" ),
			Child, PATH_SubDirs[9] = SmallToggle( "9" ),
			Child, PATH_SubDirs[10] = SmallToggle( "A" ),
			Child, PATH_SubDirs[11] = SmallToggle( "B" ),
			Child, PATH_SubDirs[12] = SmallToggle( "C" ),
			Child, PATH_SubDirs[13] = SmallToggle( "D" ),
			Child, PATH_SubDirs[14] = SmallToggle( "E" ),
			Child, PATH_SubDirs[15] = SmallToggle( "F" ),
			Child, HVSpace,
		End,
		Child, HGroup,
			Child, SmallText( GetString(MSG_PATH_ALIAS) ),
			Child, PATH_Alias = StringObject,
				StringFrame,
				MUIA_String_MaxLen, 512,
				MUIA_Disabled, TRUE,
			End,
			Child, SmallText( GetString(MSG_PATH_PATH) ),
			Child, PopaslObject,
				ASLFR_DrawersOnly, TRUE,
				MUIA_Popstring_Button, PopButton(MUII_PopDrawer),
				MUIA_Popstring_String, PATH_Path = StringObject,
					StringFrame,
					MUIA_String_MaxLen, 512,
				MUIA_Disabled, TRUE,
				End,
			End,
		End,
	End;
	SetPath();
	set( PATH_Delete, MUIA_Disabled, TRUE );
	DoMethod( APP_Main, MUIM_MultiSet,MUIA_CycleChain,TRUE, PATH_List, PATH_Add, PATH_Delete, PATH_Alias, PATH_Path, NULL );
	for ( i = 0 ; i < 16 ; i++ )
	{
		DoMethod( APP_Main, MUIM_MultiSet, MUIA_Disabled, TRUE, PATH_FRead[i], PATH_FWrite[i], PATH_FDelete[i], PATH_SubDirs[i], NULL );
		DoMethod( PATH_FRead[i], MUIM_Notify, MUIA_Selected, MUIV_EveryTime, MUIV_Notify_Self, 3, MUIM_CallHook, &PATH_Read_Hook, (1<<i) );
		DoMethod( PATH_FWrite[i], MUIM_Notify, MUIA_Selected, MUIV_EveryTime, MUIV_Notify_Self, 3, MUIM_CallHook, &PATH_Write_Hook, (1<<i) );
		DoMethod( PATH_FDelete[i], MUIM_Notify, MUIA_Selected, MUIV_EveryTime, MUIV_Notify_Self, 3, MUIM_CallHook, &PATH_Delete_Hook, (1<<i) );
		DoMethod( PATH_SubDirs[i], MUIM_Notify, MUIA_Selected, MUIV_EveryTime, MUIV_Notify_Self, 3, MUIM_CallHook, &PATH_SubDirs_Hook, (1<<i) );
	}
	DoMethod( PATH_List,			MUIM_Notify, MUIA_NList_Active,			MUIV_EveryTime, MUIV_Notify_Application , 3, MUIM_CallHook, &PATH_List_Active_Hook );
	DoMethod( PATH_List,			MUIM_Notify, MUIA_NList_Active,			MUIV_EveryTime, MUIV_Notify_Application , 3, MUIM_MultiSet, MUIA_Disabled, FALSE, PATH_Alias, PATH_Path );
	DoMethod( PATH_Add,			MUIM_Notify, MUIA_Pressed,					FALSE, PATH_List, 3, MUIM_NList_InsertSingle, -1, MUIV_NList_Insert_Bottom );
	DoMethod( PATH_Add,			MUIM_Notify, MUIA_Pressed,					FALSE, PATH_List, 3, MUIM_Set, MUIA_NList_Active, MUIV_NList_Active_Bottom );
	DoMethod( PATH_Delete,		MUIM_Notify, MUIA_Pressed,					FALSE, PATH_List, 2, MUIM_NList_Remove, MUIV_NList_Remove_Active );
	DoMethod( PATH_Sort,			MUIM_Notify, MUIA_Pressed,					FALSE, PATH_List, 1, MUIM_NList_Sort );
	DoMethod( PATH_Alias,		MUIM_Notify, MUIA_String_Contents,		MUIV_EveryTime, MUIV_Notify_Application , 3, MUIM_CallHook, &PATH_Alias_Hook, MUIV_TriggerValue );
	DoMethod( PATH_Alias,		MUIM_Notify, MUIA_String_Acknowledge,	MUIV_EveryTime, MUIV_Notify_Window, 3, MUIM_Set, MUIA_Window_ActiveObject, PATH_Path );
	DoMethod( PATH_Path,			MUIM_Notify, MUIA_String_Contents,		MUIV_EveryTime, MUIV_Notify_Application , 3, MUIM_CallHook, &PATH_Path_Hook, MUIV_TriggerValue );
	return(new);
}
