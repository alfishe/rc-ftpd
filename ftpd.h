#ifndef LIBRARIES_FTPD_H
#define	LIBRARIES_FTPD_H
/*
**	$VER: ftpd.h 40.0 (2.3.2000)
**
**	Used for ftpd library.
**
**	(C) Copyright 2000 Robin Cloutman <rycochet2@yahoo.com>
**	    All Rights Reserved
*/

#ifndef _SYS_TYPES_H_
#include <sys/types.h>
#endif

#ifndef _NETINET_IN_H_
#include	<netinet/in.h>
#endif

#define	CATCOMP_NUMBERS
#include	"ftpd_cat.h"

/* MUI_Command structure - defined here to avoid including full mui.h */
#ifndef LIBRARIES_MUI_H
struct MUI_Command
{
	char        *mc_Name;
	char        *mc_Template;
	long         mc_Parameters;
	struct Hook *mc_Hook;
};
#endif

#define	BUFSIZE	2048
#define	ASYNCBUF	8192

/*****************************************************************************************************/

struct pathdata;
struct groupdata;
struct newuserdata;
struct file;
struct user;

struct pathdata
{
	struct pathdata	*next;
	struct pathdata	*parent;
	struct pathdata	*child;

	unsigned long		id;
	unsigned short		type;
	unsigned short		flags;

// Where flags are blank then assume same as parent
	unsigned short		allow_append;		// APPE (needs create too)
	unsigned short		allow_create;		// STOU STOR APPE RNTO
	unsigned short		allow_remdir;		// RMD
	unsigned short		allow_changedir;	// CWD CDUP
	unsigned short		allow_rename;		// RNFR
	unsigned short		allow_list;			// LIST NLST MLSD (STAT <>)
	unsigned short		allow_makedir;		// MKD
	unsigned short		allow_delete;		// DELE
	unsigned short		allow_retrieve;	// RETR
	unsigned short		allow_overwrite;	// STOR (overwrite)

	char					*name;				// Relative name, no slash (except root dir)
	char					*path;				//	Depends on type
};

#define	PATH_TYPE_FAKE			(1<<0)	/* no *path, virtual only */
#define	PATH_TYPE_RELATIVE	(1<<1)	/* no *path, use the same as *name */
#define	PATH_TYPE_ABSOLUTE	(1<<2)	/* *path is absolute dir */
#define	PATH_TYPE_ALIAS		(1<<3)	/* *path replaces currentdir, can be relative or absolute */

struct groupdata
{
	struct groupdata	*next;
	struct newuserdata	*first;

	unsigned long		id;
	unsigned short		flags;
	unsigned char		log;
	unsigned char		sessions;
	unsigned long		maxcps;

	char					*name;
};

// Used for both groups and users
#define	LOG_COMMANDS			(1<<0)	/* Log commands */
#define	LOG_REPLY				(1<<1)	/* Log replies */
#define	LOG_MESSAGES			(1<<2)	/* Log messages */
#define	LOG_FAILED				(1<<3)	/* Log failed commands/replies/messages */
#define	LOG_FILE					(1<<4)	/* Log to file */

struct newuserdata
{
	struct newuserdata	*next;
	struct groupdata	*group;

	unsigned long		id;
	unsigned short		flags;
	unsigned char		log;
	unsigned char		sessions;
	unsigned long		maxcps;

	char					*name;
	char					*pass;				// no pass means not needed - discouraged though
};

struct file
{
	struct file			*next;
	struct user			*user;

	struct sockaddr_in	port;
	struct DateStamp	started;

	BPTR					file;
	unsigned long		size;
	unsigned long		current;
	unsigned long		cps;
	unsigned long		last_current;
	unsigned long		last_cps;

	char					buffer[BUFSIZE];
	char					*name;
};

struct user
{
	struct user			*next;
	struct outdata		*output;

	struct DateStamp	idle;
	unsigned short		flags;
	unsigned char		log;
	unsigned char		sessions;
	unsigned long		maxcps;

	long					control;
	long					data;
	long					pdata;
	long					tries;

	char					input[BUFSIZE];
	char					command[6];
	char					args[BUFSIZE];
	char					ip[64];
	char					path[512];

	Object	*Win;
	Object	*Info;
	Object	*List;
	Object	*Clear;
	Object	*Kick;
	Object	*Abort;
	Object	*Log;
};

/*****************************************************************************************************/

struct	accessdata;
struct	userdata;
struct	ftpdata;
struct	outdata;

struct accessdata
{
	struct accessdata *next;
	struct accessdata	*edit;
	long	id;

	char	alias[512];
	char	path[512];

	unsigned short read;
	unsigned short write;
	unsigned short delete;
	unsigned short subdirs;
};

struct userdata
{
	struct userdata *next;
	struct userdata *edit;
	long	id;

	char	user[16];
	char	pass[16];

	unsigned short access;
	unsigned short sessions;
	short	loglevel;
};

struct ftpdata
{
	struct ftpdata		*next;

	struct userdata	*user;
	struct accessdata	*access;
	struct outdata		*output;

	long	control;
	long	data;
	long	pdata;

	Object	*Win;
	Object	*Info;
	Object	*List;
	Object	*Clear;
	Object	*Kick;
	Object	*Abort;
	Object	*Log;

	struct AsyncFile *file;
	long	start;
//	unsigned long	size;
//	unsigned long	current;
	unsigned long long	size;
	unsigned long long	current;
	unsigned long	cps;
	unsigned long	maxcps;
	unsigned long	xfer;
	unsigned long	update;
	struct DateStamp started;

	char	buffer[BUFSIZE];
	char	input[BUFSIZE];
	char	command[5];
	char	args[BUFSIZE];

	char	ip[64];
	char	filename[512];
	char	path[512];
	char	cd[512];

	short	flags;
	short	loglevel;

	BPTR	file2;				// Only if ~async
};

struct outdata
{
	struct outdata *next;
	char	txt[1];
};

#define	FLAG_BINARY		(1<<0)	// Send as Image/Binary only
#define	FLAG_SEND		(1<<1)	// ftpdata.file is being sent
#define	FLAG_RECV		(1<<2)	// ftpdata.file is being recieved
#define	FLAG_QUIT		(1<<3)	// User is trying to quit, send goodbye
#define	FLAG_ONLINE		(1<<4)	// User has finished login process
#define	FLAG_LIST		(1<<5)	// Delete file after sending
#define	FLAG_QUIET		(1<<6)	// Don't send multi-line replies
#define	FLAG_CLNT		(1<<7)	// They sent CLNT command
#define	FLAG_CONNECTED	(1<<8)	// PASV or PORT is now connected
#define	FLAG_SPEEDY		(1<<9)	// Is on some sort of local connection - ignore speed limits ;-)

struct ftpd_stat
{
	unsigned long long	 fs_sent_data;
	unsigned long long	 fs_recv_data;
	unsigned long			 fs_sent_files;
	unsigned long			 fs_recv_files;
	unsigned long			 fs_sent_cps;
	unsigned long			 fs_recv_cps;
	unsigned long			 fs_users_account;
	unsigned long			 fs_users_anon;
};

#define	IS_SET(flag, bit)			((flag) & (bit))
#define	SET_BIT(var, bit)			((var) |= (bit))
#define	REMOVE_BIT(var, bit)		((var) &= ~(bit))
#define	TOGGLE_BIT(var, bit)    ((var) ^= (bit))

#define	LOWER(char)					((char) & (0xDF))
#define	UPPER(char)					((char) | (0x20))

struct accessdata *real_dir( struct ftpdata *ftp, char *path, char *buffer );

struct command
{
	char	*name;
	char	*help;
	BOOL	(*command)(struct ftpdata *ftp);
	unsigned char	flags;
	unsigned char	log;
};

#define	NEED_LOGIN	(1<<0)	// Can be used before they're logged in
#define	NEED_READ	(1<<1)	// They must have read access for this
#define	NEED_WRITE	(1<<2)	// They must have write access for this
#define	NEED_DELETE	(1<<3)	// They must have delete access for this
#define	NEED_ARGS	(1<<4)	// This command *must* have arguments
#define	NEED_FILE	(1<<5)	// This command has a file arg - so no "/:" chars
#define	NEED_UNIMP	(1<<6)	// This command is unimplemented
#define	NEED_XFER	(1<<7)	// This command may be used during file transfer
#define	LOG_ON		(1<<1)	// Log after succesful...
#define	LOG_ALWAYS	(1<<2)	// Always log after, not just succesful...
#define	LOG_BEFORE	(1<<3)	// Always log before...

BOOL cmd_abor(struct ftpdata *ftp);
BOOL cmd_acct(struct ftpdata *ftp);
BOOL cmd_allo(struct ftpdata *ftp);
BOOL cmd_appe(struct ftpdata *ftp);
BOOL cmd_cdup(struct ftpdata *ftp);
BOOL cmd_clnt(struct ftpdata *ftp);
BOOL cmd_cwd(struct ftpdata *ftp);
BOOL cmd_dele(struct ftpdata *ftp);
BOOL cmd_feat(struct ftpdata *ftp);
BOOL cmd_help(struct ftpdata *ftp);
BOOL cmd_list(struct ftpdata *ftp);
BOOL cmd_mdtm(struct ftpdata *ftp);
BOOL cmd_mkd(struct ftpdata *ftp);
BOOL cmd_mode(struct ftpdata *ftp);
BOOL cmd_noop(struct ftpdata *ftp);
BOOL cmd_nlst(struct ftpdata *ftp);
BOOL cmd_pass(struct ftpdata *ftp);
BOOL cmd_pasv(struct ftpdata *ftp);
BOOL cmd_port(struct ftpdata *ftp);
BOOL cmd_pwd(struct ftpdata *ftp);
BOOL cmd_quit(struct ftpdata *ftp);
BOOL cmd_rest(struct ftpdata *ftp);
BOOL cmd_retr(struct ftpdata *ftp);
BOOL cmd_rmd(struct ftpdata *ftp);
BOOL cmd_rnfr(struct ftpdata *ftp);
BOOL cmd_rnto(struct ftpdata *ftp);
BOOL cmd_size(struct ftpdata *ftp);
BOOL cmd_stor(struct ftpdata *ftp);
BOOL cmd_stru(struct ftpdata *ftp);
BOOL cmd_syst(struct ftpdata *ftp);
BOOL cmd_type(struct ftpdata *ftp);
BOOL cmd_user(struct ftpdata *ftp);

/*
 * Standard replies.
 */

#define	REPLY_100	"Restart marker reply."
	// In this case the text is exact and not left to the
	// particular implementation; it must read:
	// MARK yyyy = mmmm
	// where yyyy is User-process data stream marker, and mmmm
	// server's equivalent marker.  (note the spaces between
	// markers and "=".)
#define	REPLY_119	"Terminal not available, will try mailbox."
#define	REPLY_120	"Service ready in nnn minutes"
#define	REPLY_125	"Data connection already open; transfer starting"
#define	REPLY_150	"File status okay; about to open data connection."
#define	REPLY_151	"User not local; Will forward to <user>@<host>."
#define	REPLY_152	"User Unknown; Mail will be forwarded by the operator."

#define	REPLY_200	"Command okay"
#define	REPLY_202	"Command not implemented, superfluous at this site."
#define	REPLY_211	"System status, or system help reply"
#define	REPLY_212	"Directory status"
#define	REPLY_213	"File status"
#define	REPLY_214	"Help message"
	// (on how to use the server or the meaning of a particular
	// non-standard command.  This reply is useful only to the
	// human user.)
#define	REPLY_215	"<scheme> is the preferred scheme."
#define	REPLY_220	"Service ready for new user"
#define	REPLY_221	"Service closing TELNET connection"
	// (logged out if appropriate)
#define	REPLY_225	"Data connection open; no transfer in progress"
#define	REPLY_226	"Closing data connection; requested file action successful"
	// (for example, file transfer or file abort.)
#define	REPLY_227	"Entering Passive Mode (%lu,%lu,%lu,%lu,%ld,%ld)"
#define	REPLY_230	"User logged in, proceed"
#define	REPLY_250	"Requested file action okay, completed."

#define	REPLY_331	"User name okay, need password"
#define	REPLY_332	"Need account for login"
#define	REPLY_350	"Requested file action pending further information"
#define	REPLY_354	"Start mail input; end with <CR><LF>.<CR><LF>"

#define	REPLY_421	"Service not available, closing TELNET connection."
	// (This may be a reply to any command if the service knows it must shut down.)
#define	REPLY_425	"Can't open data connection"
#define	REPLY_426	"Connection closed; transfer aborted."
#define	REPLY_427	"Requested file action not taken: file unavailable"
	// (e.g. file busy)
#define	REPLY_451	"Requested action aborted: local error in processing"
#define	REPLY_452	"Requested action not taken: insufficient storage space in system"

#define	REPLY_500	"Syntax error, command unrecognized"
	// (This may include errors such as command line too long.)
#define	REPLY_501	"Syntax error in parameters or arguments"
#define	REPLY_502	"Command not implemented"
#define	REPLY_503	"Bad sequence of commands"
#define	REPLY_504	"Command not implemented for that parameter"
#define	REPLY_530	"Not logged in"
#define	REPLY_532	"Need account for storing files"
#define	REPLY_550	"Requested action not taken: file unavailable"
	// (e.g. file not found, no access)
#define	REPLY_551	"Requested action aborted: page type unknown"
#define	REPLY_552	"Requested file action aborted: exceeded storage allocation"
	// (for current directory or dataset)
#define	REPLY_553	"Requested action not taken: file name not allowed"

extern APTR					myPool;
extern struct Library	*SysBase;
extern struct Library	*DOSBase	;
extern struct Library	*SocketBase;
extern struct Library	*MUIMasterBase;
extern struct Library	*IntuitionBase;
extern struct Library	*LocaleBase;
extern struct Library	*IconBase;
extern struct Library	*WorkbenchBase;
extern struct Catalog	*catalog;
extern struct Locale		*locale;
extern struct WBStartup *_WBenchMsg;
extern struct AppIcon	*appicon;
extern struct DiskObject	*icon;
extern struct MsgPort	*iconport;

extern long	control;

extern char		*log[];
extern char		*groups[];
extern char		*configname;

extern Object	*APP_Main;

extern Object	*WIN_Prefs;

extern Object	*WIN_Main;
extern Object	*MAIN_User;
extern Object	*MAIN_Path;
extern Object	*MAIN_Xtra;
extern Object	*MAIN_Stats;
extern Object	*MAIN_Users;
extern Object	*MAIN_List;
extern Object	*MAIN_Display;
extern Object	*MAIN_Log;
extern Object	*MAIN_Kick;
extern Object	*MAIN_Abort;
extern Object	*MAIN_Open;

extern Object	*WIN_Stats;

extern Object	*PREFS_List;

extern Object	*WIN_User;
extern Object	*USER_List;
extern Object	*USER_Pass;
extern Object	*USER_Random;
extern Object	*USER_Name;
extern Object	*USER_Log;
extern Object	*USER_Add;
extern Object	*USER_Delete;
extern Object	*USER_Sessions;
extern Object	*USER_Sessions_Up;
extern Object	*USER_Sessions_Down;

extern Object	*WIN_Path;
extern Object	*PATH_List;
extern Object	*PATH_Path;
extern Object	*PATH_Alias;
extern Object	*PATH_Add;
extern Object	*PATH_Delete;

extern Object	*WIN_Xtra;
extern Object	*XTRA_Port;
extern Object	*XTRA_Port_Up;
extern Object	*XTRA_Port_Down;
extern Object	*XTRA_List;
extern Object	*XTRA_Welcome;
extern Object	*XTRA_Goodbye;
extern Object	*XTRA_Anon;
extern Object	*XTRA_Cps;
extern Object	*XTRA_Cps_Up;
extern Object	*XTRA_Cps_Down;
extern Object	*XTRA_Users;
extern Object	*XTRA_Users_Up;
extern Object	*XTRA_Users_Down;
extern Object	*XTRA_Save;

extern char *registered;
extern long	serial;
extern long ver;
extern long rev;

extern const char	windowname[];
extern const char	version[];
extern const char	shortversion[];
extern const char	banner[];
extern char			lsfile[];
extern char			*log[];
extern long	port;
extern long	connections;
extern long	opencount;
extern unsigned long	maxcps;
extern unsigned long	timeout;
extern BOOL	noop;
extern BOOL	nopasv;
extern BOOL	message;
extern BOOL	comment;
extern BOOL	open;
extern BOOL	anon;
extern BOOL	logfile;
extern BOOL	seperate;
extern long	iconx;
extern long	icony;
extern char	list[64];
extern char	listpath[64];
extern BOOL	async;
extern BOOL	short_numbers;
extern char	*closed;
extern char	*greeting;
extern char	*welcome;
extern char	*goodbye;
extern struct userdata		*user_list;
extern struct accessdata	*access_list;
extern struct ftpdata		*ftp_list;
extern struct command commands[];
extern struct MUI_Command mui_commands[];
extern struct ftpd_stat stats[3];
extern long stats_page;

// prototypes
char	*GetString		( LONG id );
void	 ErrorString	( LONG id );
unsigned long timer	( struct ftpdata *ftp );
void	 output			( struct ftpdata *ftp, char *txt, long reply );
void	 sendfile		( struct ftpdata *ftp, char *filename );
void	 memclr			( void *memory, int size );
/* sprintf is already declared in stdio.h */
struct userdata *get_user( long id );
struct accessdata *get_access( long id );
void	 add_path		( char *to, char *add );
void	 new_ftp			( long control );
void	 free_ftp		( struct ftpdata *ftp );
void	 free_data		( struct ftpdata *ftp );
void	 update_list	( struct ftpdata *ftp );
void	 update_ftp		( struct ftpdata *ftp );
void	 update_cps		( void );
void	 start_log		( struct ftpdata *ftp );
void	 update_users	( void );
BOOL	 init_ftpd		( void );
BOOL	 save_config	( char *filename );
BOOL	 load_config	( char *filename );
char	*one_word		( char *a, char *b );
char	*skip_word		( char *a );
char	*one_line		( char *a, char *b );
char	*strdup			( const char *a );
long	 strlong			( const char *a );
char	*strappend		( char *a, const char *b );
void	 strip_trailing( char *a );
BOOL	 Exists			( char *filename );
BOOL	 CheckSFV		( char *filename );
void	 load_stats		( char *filename );
void	 save_stats		( char *filename );
void	 update_stats	( void );
char	*add_comma		( long long number, BOOL short_num );

BOOL OpenFile( struct ftpdata *ftp, char *filename, int mode );
void CloseFile( struct ftpdata *ftp );
void SeekFile( struct ftpdata *ftp, long offset );
unsigned long SeekEnd( struct ftpdata *ftp );

Object	*SmallText		( char *text __asm__("a0") );
Object	*Button			( char *text __asm__("a0") );
Object	*SmallButton	( char *text __asm__("a0") );
Object	*ReplaceCodes	( void );
Object	*MiniButton		( void );
Object	*SmallToggle	( char *text __asm__("a0") );

Object	*PrefsWindow( void );
Object	*UserGroup( void );
Object	*PathGroup( void );
Object	*GeneralGroup( void );
Object	*ClosedGroup( void );
Object	*GreetingGroup( void );
Object	*WelcomeGroup( void );
Object	*GoodbyeGroup( void );

void		GetUsers( void );
void		GetPath( void );
void		GetGeneral( void );
void		GetClosed( void );
void		GetGreeting( void );
void		GetWelcome( void );
void		GetGoodbye( void );

ULONG DoMethod( Object *, ULONG, ... );
ULONG DoMethodA( Object *, Msg );

#ifndef MAKE_ID
#define MAKE_ID(a,b,c,d) ((ULONG) (a)<<24 | (ULONG) (b)<<16 | (ULONG) (c)<<8 | (ULONG) (d))
#endif

#endif	/* LIBRARIES_FTPD_H */
