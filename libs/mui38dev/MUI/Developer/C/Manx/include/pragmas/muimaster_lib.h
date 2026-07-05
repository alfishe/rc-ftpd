#ifndef	PRAGMAS_MUIMASTER_LIB_H

#define	PRAGMAS_MUIMASTER_LIB_H

/*
**	$Filename: pragmas/muimaster_lib.h $
**	$Release: 1.0 $
**	$Revision: 1.00 $
**	$Date: 12.08.93 $
**
**	pragmadefinitions for muimaster.library MANX Aztec C V 5.2
**
**	(C) Copyright 1993; Oliver Graf - T.O.M. Software Wuppertal
**	    this may be freely distributed
*/

#ifndef	CLIB_MUIMASTER_PROTOS_H
#include <clib/muimaster_protos.h>
#endif

#pragma amicall(MUIMasterBase, 0x1e, MUI_NewObjectA(a0,a1))
#pragma amicall(MUIMasterBase, 0x24, MUI_DisposeObject(a0))
#pragma amicall(MUIMasterBase, 0x2a, MUI_RequestA(d0,d1,d2,a0,a1,a2,a3))
#pragma amicall(MUIMasterBase, 0x30, MUI_AllocAslRequest(d0,a0))
#pragma amicall(MUIMasterBase, 0x36, MUI_AslRequest(a0,a1))
#pragma amicall(MUIMasterBase, 0x3c, MUI_FreeAslRequest(a0))
#pragma amicall(MUIMasterBase, 0x42, MUI_Error())

#endif
