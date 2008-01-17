#ifndef	RCFTPD_H
#define	RCFTPD_H
/*
**	$VER: rc-ftpd.h 3.0 (4.5.2001)
**
**	Server data and process messages.
**
**	(C) Copyright 2000-2001 Robin Cloutman
**	    All Rights Reserved
*/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif /* EXEC_TYPES_H */

#ifndef _SYS_TYPES_H
#include <sys/types.h>
#endif /* _SYS_TYPES_H */

#ifndef	EXEC_NODES_H
#include <exec/nodes.h>
#endif /* EXEC_NODES_H */

#ifndef	EXEC_LISTS_H
#include <exec/lists.h>
#endif /* EXEC_LISTS_H */

#ifndef	EXEC_SEMAPHORES_H
#include	<exec/semaphores.h>
#endif /* EXEC_SEMAPHORES_H */

#ifndef UTILITY_TAGITEM_H
#include	<utility/tagitem.h>
#endif /* UTILITY_TAGITEM_H */

#ifndef _NETINET_IN_H_
#include	<netinet/in.h>
#endif /* _NETINET_IN_H_ */

typedef signed long bool;

#define	TAG_TEMP		((ULONG)(1L<<30))
#define	TAG_STRING	((ULONG)(1L<<29))

/*
 * The Ftpd Semaphore base
 */
struct FtpdBase
{
	struct SignalSemaphore fb_Semaphore;
	u_long				 fb_Count;			/* How many are open? */
	BPTR					 fb_Pool;			/* Memory pool used by all processes */

	struct MinList		 fb_Servers;		/* Must run seperately */
	struct MinList		 fb_Users;			/* Set by prefs */
	struct MinList		 fb_Paths;			/* Set by prefs */

	char					*fb_Parsed;			/* Created by Library, parsed fb_Pattern */

	u_long				 fb_MaxUsers;		/* Set by prefs, maximum number of connections */
	u_long				 fb_MaxCPS;			/* Set by prefs, maximum cps shared between users */
	u_long				 fb_Timeout;		/* Set by prefs, how long before they are thrown out */
	char					*fb_Pattern;		/* Set by prefs, string for MatchPatternNoCase() */
	u_long				 fb_Port;			/* Set by prefs, only needed for standalone */
	u_long				 fb_MaxUsers;		/* Set by prefs, max users */
	u_long				 fb_MaxCPS;			/* Set by prefs, max CPS/user */
	u_long				 fb_Timeout;		/* Set by prefs, timeout in secs */
	bool					 fb_Anon;			/* Set by prefs, allow anonymous */
	bool					 fb_Closed;			/* Set by prefs, are we open? */
	bool					 fb_Noop;			/* Set by prefs, can we idle? */
	bool					 fb_Message;		/* Set by prefs, do we auto-send ".message"? */
	bool					 fb_Comment;		/* Set by prefs, do we set the comment? */
};

#define	SEMAPHORENAME	"rc-ftpd"

/*
 * Every server process has one of these
 */
struct FtpdServer
{
	struct MsgPort		 fs_Port;			/* Nice little earner ;-) */

	struct MinList		 fs_Connections;	/* List of connections */
	u_short				 fs_Count;			/* How many connections does this process have */
	u_short				 fs_Maximum;		/* And how many could we have? */
};

/*
 * Every connection has one of these
 */
struct FtpdConnection
{
	struct MinNode		 fc_Node;

	long					 fc_FD;				/* Socket FD */
	struct sockaddr_in fc_Data;			/* Needed quite often... */
	struct TagItem		*fc_Tags;			/* All needed information */
};

#define	TAG_FLAGS		(TAG_USER|0x0001)
#define	TAG_PORT			(TAG_USER|TAG_TEMP|0x0002)
#define	TAG_USER			(TAG_USER|TAG_STRING|0x0003)
#define	TAG_GROUP		(TAG_USER|TAG_STRING|0x0004)
#define	TAG_PATH			(TAG_USER|TAG_STRING|0x0005)
#define	TAG_HOST			(TAG_USER|TAG_STRING|0x0006)
#define	TAG_INSIZE		(TAG_USER|TAG_TEMP|0x0007)
#define	TAG_INBUF		(TAG_USER|TAG_TEMP|0x0008)
#define	TAG_OUTSIZE		(TAG_USER|TAG_TEMP|0x0009)
#define	TAG_OUTBUF		(TAG_USER|TAG_TEMP|0x000a)
#define	TAG_PASSWORD	(TAG_USER|TAG_STRING|0x000b)
#define	TAG_MAXUSERS	(TAG_USER|0x000c)
#define	TAG_NOWUSERS	(TAG_USER|0x000d)

struct FtpdUser
{
	struct MinNode		 fu_Node;

	char					*fu_Name;			/* Match with fc_User and fa_Owner */
	char					*fu_Password;		/* Not encrypted (yet) */
	char					*fu_Group;			/* Match with fa_Group */

	u_short				 fu_MaxUsers;		/* Maximum allowed for this user */
	u_short				 fu_NowUsers;		/* Must be recalculated on LoadPrefs() */

	long					 fu_Xfer;			/* ±(X * fu_ULoad|fu_DLoad) */
	u_char				 fu_ULoad;			/* For ratio stuff */
	u_char				 fu_DLoad;			/* For ratio stuff */

	u_short				 fu_Flags;			/* Individual flags */
};

struct FtpdPath
{
	struct MinNode		 fp_Node;
	char					*fp_Owner;			/* "ftp" */
	char					*fp_Group;			/* "guest" */
	char					*fp_Path;			/* "Internet:FTP/" (absolute) */
	char					*fp_Virtual;		/* "/" */
	u_short				 fp_OwnerAccess;	/* See below */
	u_short				 fp_GroupAccess;	/* See below */
	u_short				 fp_GuestAccess;	/* See below */
	u_short				 fp_Flags;			/* See below */
};

#define	FTPD_PATH_NEW			(1<<0)	/* STOR + RNTO */
#define	FTPD_PATH_APPEND		(1<<1)	/* APPE */
#define	FTPD_PATH_REMDIR		(1<<2)	/* RMD */
#define	FTPD_PATH_RENAME		(1<<3)	/* RNFR + RNTO */
#define	FTPD_PATH_MAKEDIR		(1<<4)	/* MKD */
#define	FTPD_PATH_DELETE		(1<<5)	/* DELE */
#define	FTPD_PATH_READ			(1<<6)	/* RETR */
#define	FTPD_PATH_WRITE		(1<<7)	/* STOR + APPE */
#define	FTPD_PATH_CD			(1<<8)	/* CWD */
#define	FTPD_PATH_LIST			(1<<9)	/* LIST */

#define	FTPD_FLAG_SUBDIRS		(1<<0)	/* Default ON, allows access to subdirs */
#define	FTPD_FLAG_OVERRIDE	(1<<1)	/* Default OFF, override AmigaOS flags */

struct FtpdCommand
{
	struct MinNode		 fc_Node;
	char					*fc_Name;
	BOOL				  (*fc_Command)(struct FtpdConnection*);
	u_long				 fc_Flags;
	char					*fc_Notify;
};

#define	FTPD_CMD_NOLOG			(1<<0)	/* Don't log me - ie. HELP */
#define	FTPD_CMD_SECRET		(1<<1)	/* Hide args in log - ie. PASS */

#endif	/* RCFTPD_SERVER_H */
