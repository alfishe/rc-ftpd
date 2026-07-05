/****************************************************************************/
/* Includes                                                                 */
/****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <exec/memory.h>
#include <prefs/prefhdr.h>
#include <datatypes/pictureclass.h>
#include <libraries/locale.h>
#include <libraries/asl.h>
#include <libraries/gadtools.h>
#include <libraries/mui.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/utility_protos.h>
#include <clib/dos_protos.h>
#include <clib/locale_protos.h>
#include <clib/muimaster_protos.h>

#include <clib/alib_protos.h>

#ifdef __SASC
#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/locale_pragmas.h>
#include <pragmas/muimaster_pragmas.h>
#endif

#define CATCOMP_NUMBERS
#define CATCOMP_ARRAY
#include "psi.lh"

#define USE_PSI_SIZES_BODY
#define USE_PSI_SIZES_COLORS
#include "psi_sizes.bh"

#define USE_PSI_COLORS_BODY
#include "psi_colors.bh"

#define USE_PSI_FREQS_BODY
#include "psi_freqs.bh"



/****************************************************************************/
/* Compiler Stuff                                                           */
/****************************************************************************/

#ifdef _DCC
#define REG(x) __ ## x
#define ASM
#define SAVEDS __geta4
#else
#define REG(x) register __ ## x
#define ASM    __asm
#define SAVEDS __saveds
#endif



/****************************************************************************/
/* Some Definitions                                                         */
/****************************************************************************/

#define MUISERIALNR_STUNTZI 1
#define TAGBASE_STUNTZI (TAG_USER | ( MUISERIALNR_STUNTZI << 16))

#define MUIA_DispIDinfo_ID            (TAGBASE_STUNTZI | 0x1010)

#define MUIA_DispIDlist_CurrentID     (TAGBASE_STUNTZI | 0x1020)
#define MUIA_DispIDlist_Quiet         (TAGBASE_STUNTZI | 0x1021)
#define MUIM_DispIDlist_Change        (TAGBASE_STUNTZI | 0x1022)

#define MUIM_EditPanel_SetScreen      (TAGBASE_STUNTZI | 0x1030)
#define MUIM_EditPanel_GetScreen      (TAGBASE_STUNTZI | 0x1031)
#define MUIM_EditPanel_Update         (TAGBASE_STUNTZI | 0x1032)
#define MUIM_EditPanel_DefColors      (TAGBASE_STUNTZI | 0x1035)
/*
#define MUIM_EditPanel_ToggleForeign  (TAGBASE_STUNTZI | 0x1036)
*/

#define MUIA_EditWindow_Title         (TAGBASE_STUNTZI | 0x1040)
#define MUIA_EditWindow_Originator    (TAGBASE_STUNTZI | 0x1041)
#define MUIM_EditWindow_Close         (TAGBASE_STUNTZI | 0x1042)

#define MUIM_ScreenList_Save          (TAGBASE_STUNTZI | 0x1050)
#define MUIM_ScreenList_Load          (TAGBASE_STUNTZI | 0x1051)
#define MUIM_ScreenList_Find          (TAGBASE_STUNTZI | 0x1052)

#define MUIM_ScreenPanel_Create       (TAGBASE_STUNTZI | 0x1060)
#define MUIM_ScreenPanel_Copy         (TAGBASE_STUNTZI | 0x1061)
#define MUIM_ScreenPanel_Delete       (TAGBASE_STUNTZI | 0x1062)
#define MUIM_ScreenPanel_Edit         (TAGBASE_STUNTZI | 0x1063)
#define MUIM_ScreenPanel_Finish       (TAGBASE_STUNTZI | 0x1064)
#define MUIM_ScreenPanel_CloseWindows (TAGBASE_STUNTZI | 0x1065)
#define MUIM_ScreenPanel_SetStates    (TAGBASE_STUNTZI | 0x1066)
#define MUIM_ScreenPanel_Open         (TAGBASE_STUNTZI | 0x1067)
#define MUIM_ScreenPanel_Close        (TAGBASE_STUNTZI | 0x1068)
#define MUIM_ScreenPanel_Jump         (TAGBASE_STUNTZI | 0x1069)
#define MUIM_ScreenPanel_Update       (TAGBASE_STUNTZI | 0x106a)
#define MUIM_ScreenPanel_Foo          (TAGBASE_STUNTZI | 0x106b)

#define MUIM_MainWindow_Finish        (TAGBASE_STUNTZI | 0x1070)
#define MUIM_MainWindow_About         (TAGBASE_STUNTZI | 0x1071)
#define MUIM_MainWindow_Restore       (TAGBASE_STUNTZI | 0x1072)
#define MUIM_MainWindow_Open          (TAGBASE_STUNTZI | 0x1073)
#define MUIM_MainWindow_SaveAs        (TAGBASE_STUNTZI | 0x1074)

#define MUIM_ColorEdit_SetColors      (TAGBASE_STUNTZI | 0x1082)
#define MUIM_ColorEdit_GetColors      (TAGBASE_STUNTZI | 0x1083)

struct MUIP_EditPanel_SetScreen { ULONG MethodID; struct MUI_PubScreenDesc *desc; };
struct MUIP_EditPanel_GetScreen { ULONG MethodID; struct MUI_PubScreenDesc *desc; };
struct MUIP_EditPanel_Update    { ULONG MethodID; LONG level; };
struct MUIP_EditPanel_DefColors { ULONG MethodID; LONG nr; };

struct MUIP_ScreenList_Save     { ULONG MethodID; char *name; };
struct MUIP_ScreenList_Load     { ULONG MethodID; char *name; LONG clear; };
struct MUIP_ScreenList_Find     { ULONG MethodID; char *name; struct MUI_PubScreenDesc **desc; };

struct MUIP_ScreenPanel_Finish  { ULONG MethodID; Object *win; LONG ok; };

struct MUIP_MainWindow_Finish   { ULONG MethodID; LONG level; };
struct MUIP_MainWindow_Restore  { ULONG MethodID; LONG envarc; };
struct MUIP_MainWindow_Open     { ULONG MethodID; LONG append; };

struct MUIP_ColorEdit_SetColors  { ULONG MethodID; struct MUI_RGBcolor *palette; BYTE *syspens; struct MUI_PenSpec *muipens; };
struct MUIP_ColorEdit_GetColors  { ULONG MethodID; struct MUI_RGBcolor *palette; BYTE *syspens; struct MUI_PenSpec *muipens; };

#define RectangleWidth(r)  ((r).MaxX-(r).MinX+1)
#define RectangleHeight(r) ((r).MaxY-(r).MinY+1)

#define SYSPEN_OFFSET   1
#define MUIPEN_OFFSET   1

#define ForEntries(list,entry,succ) for (entry=(APTR)((struct Node *)(((struct List *)list)->lh_Head));succ=(APTR)((struct Node *)entry)->ln_Succ;entry=(APTR)succ)
