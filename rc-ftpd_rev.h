/**************************************************
   This file was created automatically by Bump 7.0

   Do NOT edit by hand!
**************************************************/

#include	<exec/execbase.h>

#define	VERSION		2
#define	REVISION		60
#define	BETA			0
#define	DATE			"16.07.2002"
#define	NAME			"rc-ftpd"
#define	AUTHOR		"Robin Cloutman"

#if	defined(__mc68060__)
#define	CPUSTR	"[060]"
#define	CheckCPU	(((struct ExecBase*)SysBase)->AttnFlags&AFF_68060)
#elif	defined(__mc68040__)
#define	CPUSTR	"[040]"
#define	CheckCPU	(((struct ExecBase*)SysBase)->AttnFlags&AFF_68040)
#elif	defined(__mc68030__)
#define	CPUSTR	"[030]"
#define	CheckCPU	(((struct ExecBase*)SysBase)->AttnFlags&AFF_68030)
#elif	defined(__mc68020__)
#define	CPUSTR	"[020]"
#define	CheckCPU	(((struct ExecBase*)SysBase)->AttnFlags&AFF_68020)
#else
#define	CPUSTR	""
#define	CheckCPU	(-1)
#endif

#define	VERS			"rc-ftpd"##CPUSTR##" 2.60"
#define	SHORTVERS	"2.60"
#define	VSTRING		VERS" (16.07.02)\r\n"
#define	VERSTAG		"\0$VER: "##VERS##" ("##DATE##")"
#define	SVER			"$VER: "##VERS##" ("##DATE##") ©2000-2002 Robin Cloutman"
