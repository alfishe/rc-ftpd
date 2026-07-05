/*****************************************************************************************************
 * rc-ftpd �2000 Robin Cloutman <rycochet2@yahoo.com>                                                *
 * --------------------------------------------------                                                *
 * MUI ftp daemon, may be split into ftpd.library, ftpd, ftpserv and ftpgui later.                   *
 *****************************************************************************************************/

// We DON'T want bloody stdio.h!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#define	_STDIO_H_

//#define	NOHIDEPASS 1
//#define	PRINTF(x)	Printf(x)
#define	PRINTF(x)

#include <sys/types.h>

#include	<ctype.h>
#include	<dos/dos.h>
#include	<dos/exall.h>
#include	<dos/datetime.h>
#include	<dos/dostags.h>
#include	<dos/dosextens.h>
#include	<exec/memory.h>
#include	<netdb.h>
#include	<netinet/in.h>
#include	<string.h>
#include	<sys/socket.h>
#include	<sys/time.h>
#include	<sys/filio.h>
#include <libraries/gadtools.h>
#include <libraries/asl.h>
#include <libraries/mui.h>
#include	<mui/MUIundoc.h>
#include	<mui/NList_mcc.h>
#include	<mui/NListview_mcc.h>
#include	<workbench/startup.h>
#include	<workbench/workbench.h>

#include	<clib/alib_protos.h>
#include	<clib/muimaster_protos.h>

#include	<inline/exec.h>
#include	<inline/socket.h>
#include	<inline/dos.h>
#include	<inline/locale.h>
#include	<inline/icon.h>
#include	<inline/wb.h>

#include "ftpd.h"
#include	"asyncio.h"
#include "rc-ftpd_rev.h"

unsigned long __stack	= 32768;		// Minumum stack to use this program...

#define WINDOWNAME "RC-FTPd " SHORTVERS

const char	version[]	= SVER;
const char	shortversion[]	= SHORTVERS;
const char	banner[]		= "\ec(\ei" VERS "\en) Unregistered, please register";
const char	banner2[]	= "\ec(\ei" VERS "\en) Registered";
const char	windowname[]= WINDOWNAME;
char	lsfile[]				= "ftpd-%08.lx-tmp";
char	*configname			= "ftpd.config";

APTR				myPool;
struct Library	*SysBase;
struct Library	*DOSBase	;
struct Library	*SocketBase;
struct Library *MUIMasterBase;
struct Library *IntuitionBase;
struct Library	*LocaleBase;
struct Library	*IconBase;
struct Library	*WorkbenchBase;
struct Catalog	*catalog;
struct Locale  *locale;
extern struct WBStartup *_WBenchMsg;
struct AppIcon *appicon;
struct DiskObject *icon;
struct MsgPort *iconport;
struct ftpd_stat stats[3];
long stats_page = 0;

long	control	= -1;
char *registered;
long	serial;

// Initialised in ftpd.config
long	port								= 21;
long	connections						= 0;
long	opencount						= 0;
unsigned long	maxcps				= 0;
unsigned long	timeout				= 0;
BOOL	open								= FALSE;
BOOL	anon								= FALSE;
BOOL	noop								= FALSE;
BOOL	message							= TRUE;
BOOL	comment							= TRUE;
BOOL	nopasv							= FALSE;
BOOL	logfile							= FALSE;
BOOL	seperate							= FALSE;
char	list[64]							= "~(.#?|#?.info)";
char	listpath[64]					= "T:";
BOOL	async								= FALSE;
long	iconx								= NO_ICON_POSITION;
long	icony								= NO_ICON_POSITION;
BOOL	short_numbers					= TRUE;
char	*goodbye;
char	*welcome;
char	*greeting;
char	*closed;

struct userdata	*user_list		= NULL;
struct accessdata	*access_list	= NULL;
struct ftpdata		*ftp_list		= NULL;

unsigned long	my_cps = 0;

struct command commands[] =	// commands.ftpd_log is set by prefs, default is NULL
{
	{ "USER",	"<username>",								cmd_user,	NEED_LOGIN|NEED_ARGS					},
	{ "PASS",	"<password>",								cmd_pass,	NEED_LOGIN|NEED_ARGS					},
	{ "ACCT",	"<account information>",				cmd_acct,	NEED_LOGIN								},
	{ "REIN",	"",											NULL,			NEED_UNIMP								},
	{ "QUIT",	"(terminate service)",					cmd_quit,	NEED_LOGIN|NEED_XFER					},
	{ "PORT",	"h1,h2,h3,h4,p1,p2",						cmd_port,	NEED_ARGS								},
	{ "PASV",	"(set server in passive mode)",		cmd_pasv,	0											},
	{ "TYPE",	"<type code> (set transfer type)",	cmd_type,	NEED_ARGS								},
	{ "STRU",	"<structure code>",						cmd_stru,	0											},
	{ "MODE",	"<mode code>",								cmd_mode,	0											},
	{ "RETR",	"<pathname> (retrieve file)",			cmd_retr,	NEED_READ|NEED_ARGS|NEED_FILE		},
	{ "STOR",	"<pathname> (store file)",				cmd_stor,	NEED_WRITE|NEED_ARGS|NEED_FILE	},
	{ "APPE",	"<pathname> (append to file)",		cmd_appe,	NEED_WRITE|NEED_ARGS|NEED_FILE	},
//MLFL [<SP> <ident>] <CRLF>
//MAIL [<SP> <ident>] <CRLF>
//MSND [<SP> <ident>] <CRLF>
//MSOM [<SP> <ident>] <CRLF>
//MSAM [<SP> <ident>] <CRLF>
//MRSQ [<SP> <scheme>] <CRLF>
//MRCP <SP> <ident> <CRLF>
	{ "ALLO",	"[R] <decimal integar> (allocate storage)",cmd_allo,0										},
	{ "REST",	"marker (resume next file command)",cmd_rest,	NEED_ARGS								},
	{ "RNFR",	"<pathname> (rename file from)",		cmd_rnfr,	NEED_DELETE|NEED_ARGS|NEED_FILE	},
	{ "RNTO",	"<pathname> (rename file to)",		cmd_rnto,	NEED_DELETE|NEED_ARGS|NEED_FILE	},
	{ "ABOR",	"(abort operation)",						cmd_abor,	NEED_XFER								},
	{ "DELE",	"<pathname> (delete file)",			cmd_dele,	NEED_DELETE|NEED_ARGS|NEED_FILE	},
	{ "CWD",		"<pathname> (change working directory)",cmd_cwd,NEED_ARGS|NEED_FILE					},
	{ "LIST",	"[<pathname>] (retrieve dir list)",	cmd_list,	NEED_READ|NEED_FILE					},
	{ "NLST",	"[<pathname>] (retrieve name list)",cmd_nlst,	NEED_READ|NEED_FILE					},
	{ "SITE",	"<string>",									NULL,			NEED_UNIMP								},
	{ "STAT",	"[<pathname>]",							NULL,			NEED_UNIMP|NEED_XFER|NEED_FILE	},
	{ "HELP",	"[<string>] (shows help)",				cmd_help,	NEED_LOGIN								},
	{ "NOOP",	"(no-operation)",							cmd_noop,	0											},

	{ "CDUP",	"(change to parent directory)",		cmd_cdup,	0											},
	{ "MDTM",	"<pathname>",								cmd_mdtm,	NEED_READ|NEED_ARGS|NEED_FILE		},
	{ "SIZE",	"<pathname>",								cmd_size,	NEED_READ|NEED_ARGS|NEED_FILE		},
	{ "MKD",		"<pathname> (make directory)",		cmd_mkd,		NEED_WRITE|NEED_ARGS|NEED_FILE	},
	{ "PWD",		"(return current directory)",			cmd_pwd,		0											},
	{ "RMD",		"<pathname> (remove directory)",		cmd_rmd,		NEED_DELETE|NEED_ARGS|NEED_FILE	},
	{ "SYST",	"(get type of operating system)",	cmd_syst,	0											},

	{ "CLNT",	"",											cmd_clnt,	0											},
	{ "FEAT",	"",											cmd_feat,	0											},

	{ "XCUP",	"(change to parent directory)",		cmd_cdup,	0											},
	{ "XCWD",	"directory-name",							cmd_cwd,		NEED_ARGS|NEED_FILE					},
	{ "XMKD",	"path-name",								cmd_mkd,		NEED_WRITE|NEED_ARGS|NEED_FILE	},
	{ "XPWD",	"(return current directory)",			cmd_pwd,		0											},
	{ "XRMD",	"path-name",								cmd_rmd,		NEED_DELETE|NEED_ARGS|NEED_FILE	},
	{NULL,NULL,NULL,0}
};

unsigned long range( unsigned long min, unsigned long x, unsigned long max )
{
	if ( x > max )return( max );
	if ( x < min )return( min );
	return( x );
}

void memclr( void *memory, int size )
{
	char *mem = (char *)memory;
	while( size )mem[--size] = 0;
}

/* These functions are provided by libnix, no need to redefine */
#ifdef __SASC
UWORD fmtfunc[] = { 0x16c0, 0x4e75 };
void __stdargs sprintf( char *to, char *fmt, ... )
{
	RawDoFmt( fmt, &fmt + 1, (APTR)fmtfunc, to );
}

size_t strlen( const char *a )
{
	size_t n = 0;
	while( *a++ )n++;
	return(n);
}

char *strcpy( char *a, const char *b )
{
	while( ( *a++ = *b++ ) );
	return(a);
}

char *strncpy( char *a, const char *b, size_t count )
{
	while( ( *a = *b++ ) && count-- )a++;
	*a = '\0';
	return(a);
}
#endif

char *strcat( char *a, const char *b )
{
	while( *a )a++;
	while( ( *a++ = *b++ ) );
	return(a);
}

char *strncat( char *a, const char *b, size_t count )
{
	while( *a && --count )a++;
	if ( count )while( ( *a = *b++ ) && --count )a++;
	*a = '\0';
	return(a);
}

int strcmp( const char *a, const char *b )
{
	while( *a && *b && *a == *b )a++, b++;
	return(*a - *b);
}

int strncmp( const char *a, const char *b, size_t count )
{
	while( *a && *b && *a == *b && --count )a++, b++;
	return(count ? *a - *b : 0);
}

int stricmp( const char *a, const char *b )
{
	while( *a && *b && LOWER(*a) == LOWER(*b) )a++, b++;
	return(LOWER(*a) - LOWER(*b));
}

int strnicmp( const char *a, const char *b, size_t count )
{
	while( *a && *b && LOWER(*a) == LOWER(*b) && --count )a++, b++;
	return(count ? LOWER(*a) - LOWER(*b) : 0);
}

BOOL strbool( const char *a )
{
	if ( toupper(*a)=='Y' )return TRUE;
	return FALSE;
}

long strlong( const char *a )
{
	long i;
	StrToLong((char*)a,&i);
	return(i);
}

char *strchr( const char *a, int c )
{
	while ( *a && *a != c )a++;
	if ( !*a )return(NULL);
	return((char*)c);
}

char *one_word( char *a, char *b )
{
	while ( isspace(*b) )b++;
	while ( *b && !isspace(*b) )*a++ = *b++;
	*a = '\0';
	while ( isspace(*b) )b++;
	return(b);
}

char *skip_word( char *a )
{
	while ( isspace(*a) )a++;
	while ( *a && !isspace(*a) )a++;
	while ( isspace(*a) )a++;
	return(a);
}

char *one_line( char *a, char *b )
{
	while ( *b && *b!='\r' && *b!='\n' )*a++ = *b++;
	*a = '\0';
	if ( *b=='\r' )b++;
	if ( *b=='\n' )b++;
	return(b);
}

char *strdup( const char *a )
{
	char *b;
	if ( ( b = AllocPooled(myPool,strlen(a)+1) ) )strcpy(b,a);
	return(b);
}

char *strappend( char *a, const char *b )
{
	char *c;
	long len = 1;

	if ( a )len += strlen(a);
	len += strlen(b);
	if ( ( c = AllocPooled(myPool,len) ) )
	{
		if ( a )
		{
			strcpy(c,a);
			FreePooled(myPool,a,strlen(a)+1);
		}
		strcat(c,b);
	}
	return(c);
}

void strip_trailing( char *a )
{
	int i;
	for ( i = strlen(a)-1 ; i>0 && ( a[i]=='/' || ( a[i]=='.' && a[i-1]=='/' ) ) ; i-- )a[i] = '\0';
}


BOOL OpenFile( struct ftpdata *ftp, char *filename, int mode )
{
	if ( async )
	{
		if ( !( ftp->file = OpenAsync( filename, mode, ASYNCBUF ) ) )return(FALSE);
	}
	else
	{
		long dosmode[3] = { MODE_OLDFILE, MODE_NEWFILE, MODE_READWRITE };
		if ( !( ftp->file2 = Open( filename, dosmode[mode] ) ) )return(FALSE);
	}
	return(TRUE);
}

void CloseFile( struct ftpdata *ftp )
{
	if ( ftp->file )			CloseAsync( ftp->file );
	else if (ftp->file2 )	Close( ftp->file2 );
	ftp->file = NULL;
	ftp->file2 = 0;
}

void SeekFile( struct ftpdata *ftp, long offset )
{
	if ( ftp->file )			SeekAsync( ftp->file, offset, MODE_START );
	else if ( ftp->file2 )	Seek( ftp->file2, offset, OFFSET_BEGINNING );
}

unsigned long SeekEnd( struct ftpdata *ftp )
{
	if ( ftp->file )			return( SeekAsync( ftp->file, 0, MODE_END ) );
	else if ( ftp->file2 )	return( Seek( ftp->file2, 0, OFFSET_END ) );
	return( 0 );
}

void SeekRelative( struct ftpdata *ftp, long offset )
{
	if ( ftp->file )			SeekAsync( ftp->file, offset, MODE_CURRENT );
	else if ( ftp->file2 )	Seek( ftp->file2, offset, OFFSET_CURRENT );
}

long WriteFile( struct ftpdata *ftp, char *buffer, long len )
{
	if ( ftp->file )			return( WriteAsync( ftp->file, buffer, len ) );
	else if ( ftp->file2 )	return( Write( ftp->file2, buffer, len ) );
	return(-1);
}

long ReadFile( struct ftpdata *ftp, char *buffer, long len )
{
	if ( ftp->file )			return( ReadAsync( ftp->file, buffer, len ) );
	else if ( ftp->file2 )	return( Read( ftp->file2, buffer, len ) );
	return(-1);
}


unsigned long timer( struct ftpdata *ftp )
{
	struct DateStamp ds;
	signed long seconds;

	DateStamp( &ds );
	seconds = ( ds.ds_Tick / TICKS_PER_SECOND ) - ( ftp->started.ds_Tick / TICKS_PER_SECOND );
	seconds +=  ( ds.ds_Minute * 60 ) - ( ftp->started.ds_Minute * 60 );
	if ( seconds<0 )seconds += ( ( 60 * 60 * 24 ) * ( ds.ds_Days - ftp->started.ds_Days ) );
	return( (unsigned long)seconds );
}

BOOL Exists( char *filename )
{
	BPTR lock;

	if ( !( lock = Lock(filename,ACCESS_READ) ) )return( FALSE );
	UnLock( lock );
	return( TRUE );
}

struct accessdata *get_access( long id )
{
	struct accessdata *access, *prev;

	for ( access = access_list ; access ; access = access->next )if ( access->id == id )break;
	if ( !access && ( access = AllocPooled( myPool, sizeof(struct accessdata) ) ) )
	{
		access->id = id;
		if ( !access_list || access_list->id > id )
		{
			access->next = access_list;
			access_list = access;
		}
		else
		{
			for ( prev = access_list ; prev->next ; prev = prev->next )if ( prev->next->id > id )break;
			access->next = prev->next;
			prev->next = access;
		}
	}
	return(access);
}

struct userdata *get_user( long id )
{
	struct userdata *user, *prev;

	for ( user = user_list ; user ; user = user->next )if ( user->id == id )break;
	if ( !user && ( user = AllocPooled( myPool, sizeof(struct userdata) ) ) )
	{
		user->id = id;
		user->access = 1;
		user->sessions = 1;
		if ( !user_list || user_list->id > id )
		{
			user->next = user_list;
			user_list = user;
		}
		else
		{
			for ( prev = user_list ; prev->next ; prev = prev->next )if ( prev->next->id > id )break;
			user->next = prev->next;
			prev->next = user;
		}
	}
	return(user);
}

char	port_txt[]			= "Port";
char	connections_txt[]	= "Connections";
char	maxcps_txt[]		= "MaxCPS";
char	timeout_txt[]		= "TimeOut";
char	noop_txt[]			= "Noop";
char	nopasv_txt[]		= "NoPASV";
char	message_txt[]		= "Message";
char	comment_txt[]		= "Comment";
char	open_txt[]			= "Open";
char	anon_txt[]			= "Anon";
char	list_txt[]			= "List";
char	listpath_txt[]		= "Listpath";
char	user_txt[]			= "User";
char	name_txt[]			= "Name";
char	password_txt[]		= "Password";
char	access_txt[]		= "Access";
char	log_txt[]			= "Log";
char	sessions_txt[]		= "Sessions";
char	path_txt[]			= "Path";
char	fake_txt[]			= "Fake";
char	real_txt[]			= "Real";
char	read_txt[]			= "Read";
char	write_txt[]			= "Write";
char	delete_txt[]		= "Delete";
char	subdirs_txt[]		= "SubDirs";
char	iconx_txt[]			= "IconX";
char	icony_txt[]			= "IconY";
char	logfile_txt[]		= "LogFile";
char	seperate_txt[]		= "Seperate";
char	async_txt[]			= "Async";
char	short_txt[]			= "Short";

char	configid[]			= "FTPD1 - FTPD Configuration";

char	ftpd_txt[]			= "[FTPd]";
char	closed_txt[]		= "[Closed]";
char	greeting_txt[]		= "[Greeting]";
char	welcome_txt[]		= "[Welcome]";
char	goodbye_txt[]		= "[Goodbye]";
char	nstringn_txt[]		= "\n%s\n";
char	nnstringn_txt[]	= "\n\n%s\n";
char	savelong_txt[]		= "%-17.17s= %lu\n";
char	savestring_txt[]	= "%-17.17s= %s\n";
char	userlong_txt[]		= "User%03.ld.%-9.9s= %lu\n";
char	userstring_txt[]	= "User%03.ld.%-9.9s= %s\n";
char	pathlong_txt[]		= "Path%03.ld.%-9.9s= %lu\n";
char	pathstring_txt[]	= "Path%03.ld.%-9.9s= %s\n";

BOOL save_config( char *filename )
{
	struct userdata *user;
	struct accessdata *access;
	char	buffer[BUFSIZE], *buf;
	BPTR	file;

	if ( !( file = Open( filename, MODE_NEWFILE ) ) )
	{
		ErrorString( MSG_SAVE_CONFIG );
		return FALSE;
	}
	FPuts( file, configid );
	sprintf( buffer, nnstringn_txt, ftpd_txt );							FPuts( file, buffer );
	sprintf( buffer, savelong_txt, port_txt, port );					FPuts( file, buffer );
	sprintf( buffer, savelong_txt, connections_txt, connections );	FPuts( file, buffer );
	sprintf( buffer, savelong_txt, maxcps_txt, maxcps );				FPuts( file, buffer );
	sprintf( buffer, savelong_txt, timeout_txt, timeout );			FPuts( file, buffer );
	sprintf( buffer, savestring_txt, noop_txt, noop?"Y":"N" );		FPuts( file, buffer );
	sprintf( buffer, savestring_txt, nopasv_txt, nopasv?"Y":"N" );	FPuts( file, buffer );
	sprintf( buffer, savestring_txt, message_txt, message?"Y":"N" );FPuts( file, buffer );
	sprintf( buffer, savestring_txt, comment_txt, comment?"Y":"N" );FPuts( file, buffer );
	sprintf( buffer, savestring_txt, open_txt, open?"Y":"N" );		FPuts( file, buffer );
	sprintf( buffer, savestring_txt, anon_txt, anon?"Y":"N" );		FPuts( file, buffer );
	sprintf( buffer, savestring_txt, logfile_txt, logfile?"Y":"N" );FPuts( file, buffer );
	sprintf( buffer, savestring_txt, seperate_txt, seperate?"Y":"N" );FPuts( file, buffer );
	sprintf( buffer, savestring_txt, list_txt, list );					FPuts( file, buffer );
	sprintf( buffer, savestring_txt, listpath_txt, listpath );		FPuts( file, buffer );
	sprintf( buffer, savestring_txt, async_txt, async?"Y":"N" );	FPuts( file, buffer );
	sprintf( buffer, savestring_txt, short_txt, short_numbers?"Y":"N" );FPuts( file, buffer );
	sprintf( buffer, savelong_txt, iconx_txt, iconx );					FPuts( file, buffer );
	sprintf( buffer, savelong_txt, icony_txt, icony );					FPuts( file, buffer );
	for ( user = user_list ; user ; user = user->next )
	{
		sprintf( buffer, userstring_txt, user->id, name_txt, user->user );			FPuts( file, buffer );
		if ( user->pass )
		 {	sprintf( buffer, userstring_txt, user->id, password_txt, user->pass );	FPuts( file, buffer ); }
		if ( user->access!=1 )
		 {	sprintf( buffer, userlong_txt, user->id, access_txt, user->access );		FPuts( file, buffer ); }
		if ( user->loglevel )
		 { sprintf( buffer, userlong_txt, user->id, log_txt, user->loglevel );		FPuts( file, buffer ); }
		if ( user->sessions!=1 )
		 { sprintf( buffer, userlong_txt, user->id, sessions_txt, user->sessions );FPuts( file, buffer ); }
	}
	for ( access = access_list ; access ; access = access->next )
	{
		sprintf( buffer, pathstring_txt, access->id, fake_txt, access->alias );		FPuts( file, buffer );
		sprintf( buffer, pathstring_txt, access->id, real_txt, access->path );		FPuts( file, buffer );
		if ( access->read )
		 {	sprintf( buffer, pathlong_txt, access->id, read_txt, access->read );		FPuts( file, buffer ); }
		if ( access->write )
		 {	sprintf( buffer, pathlong_txt, access->id, write_txt, access->write );	FPuts( file, buffer ); }
		if ( access->delete )
		 {	sprintf( buffer, pathlong_txt, access->id, delete_txt, access->delete );FPuts( file, buffer ); }
		if ( access->subdirs )
		 {	sprintf( buffer, pathlong_txt, access->id, subdirs_txt, access->subdirs );FPuts( file, buffer ); }
	}
	if ( ( buf = closed ) )
	{
		sprintf( buffer, nstringn_txt, closed_txt );	FPuts( file, buffer );
		while ( *buf )
		{
			buf = one_line( buffer, buf );
			strcat( buffer, "\n" );
			FPuts( file, buffer );
		}
	}
	if ( ( buf = greeting ) )
	{
		sprintf( buffer, nstringn_txt, greeting_txt );	FPuts( file, buffer );
		while ( *buf )
		{
			buf = one_line( buffer, buf );
			strcat( buffer, "\n" );
			FPuts( file, buffer );
		}
	}
	if ( ( buf = welcome ) )
	{
		sprintf( buffer, nstringn_txt, welcome_txt );	FPuts( file, buffer );
		while ( *buf )
		{
			buf = one_line( buffer, buf );
			strcat( buffer, "\n" );
			FPuts( file, buffer );
		}
	}
	if ( ( buf = goodbye ) )
	{
		sprintf( buffer, nstringn_txt, goodbye_txt );	FPuts( file, buffer );
		while ( *buf )
		{
			buf = one_line( buffer, buf );
			strcat( buffer, "\n" );
			FPuts( file, buffer );
		}
	}
	Close(file);
	return TRUE;
}

BOOL load_config( char *filename )
{
	char	buffer[BUFSIZE], buffer2[BUFSIZE], *buf;
	BPTR	file;
	int	i;

	if ( !( file = Open( filename, MODE_OLDFILE ) ) )return TRUE;
	FGets( file, buffer, BUFSIZE-1 );
	if ( strncmp( buffer, configid, strlen(configid) ) )
	{
		ErrorString( MSG_BAD_CONFIG );
		Close( file );
		return FALSE;
	}
	else
	{
		int loading = 0;

		while ( ( buf = FGets( file, buffer, BUFSIZE-1 ) ) )
		{
			i = strlen(buf)-1;
			while ( isspace(buf[i]) )buf[i--] = '\0';

			if ( !*buf )loading = 0;
			else if ( !loading && !stricmp(buf,ftpd_txt) )loading = 1;
			else if ( !loading && !stricmp(buf,closed_txt) )loading = 2;
			else if ( !loading && !stricmp(buf,greeting_txt) )loading = 3;
			else if ( !loading && !stricmp(buf,welcome_txt) )loading = 4;
			else if ( !loading && !stricmp(buf,goodbye_txt) )loading = 5;
			else if ( loading==1 )
			{
				buf = one_word( buffer2, buf );
				buf = skip_word( buf );

					  if ( !stricmp( buffer2, port_txt ) )			port = strlong(buf);
				else if ( !stricmp( buffer2, connections_txt ) )connections = strlong(buf);
				else if ( !stricmp( buffer2, maxcps_txt ) )		maxcps = strlong(buf);
				else if ( !stricmp( buffer2, timeout_txt ) )		timeout = strlong(buf);
				else if ( !stricmp( buffer2, noop_txt ) )			noop = strbool( buf );
				else if ( !stricmp( buffer2, nopasv_txt ) )		nopasv = strbool( buf );
				else if ( !stricmp( buffer2, message_txt ) )		message = strbool( buf );
				else if ( !stricmp( buffer2, comment_txt ) )		comment = strbool( buf );
				else if ( !stricmp( buffer2, open_txt ) )			open = strbool( buf );
				else if ( !stricmp( buffer2, anon_txt ) )			anon = strbool( buf );
				else if ( !stricmp( buffer2, logfile_txt ) )		logfile = strbool( buf );
				else if ( !stricmp( buffer2, seperate_txt ) )	seperate = strbool( buf );
				else if ( !stricmp( buffer2, list_txt ) )			strncpy( list, buf, 64 );
				else if ( !stricmp( buffer2, listpath_txt ) )	strncpy( listpath, buf, 64 );
				else if ( !stricmp( buffer2, async_txt ) )		async = strbool( buf );
				else if ( !stricmp( buffer2, short_txt ) )		short_numbers = strbool( buf );
				else if ( !stricmp( buffer2, iconx_txt ) )		iconx = strlong(buf);
				else if ( !stricmp( buffer2, icony_txt ) )		icony = strlong(buf);
				else if ( !strnicmp( buffer2, user_txt, 4 ) )
				{
					struct userdata *user;
					char	*buf2 = buffer2;

					while ( *buf2 )if ( *buf2++ == '.' )break;
					if ( *buf2 && ( user = get_user( strlong(&buffer2[4]) ) ) )
					{
							  if ( !stricmp( buf2, name_txt ) )		strncpy( user->user, buf, 16 );
						else if ( !stricmp( buf2, password_txt ) )strncpy( user->pass, buf, 16 );
						else if ( !stricmp( buf2, access_txt ) )	user->access = (unsigned short)strlong( buf );
						else if ( !stricmp( buf2, sessions_txt ) )user->sessions = (unsigned short)strlong( buf );
						else if ( !stricmp( buf2, log_txt ) )		user->loglevel = (short)strlong( buf );
					}
				}
				else if ( !strnicmp( buffer2, path_txt, 4 ) )
				{
					struct accessdata *access;
					char	*buf2 = buffer2;

					while ( *buf2 )if ( *buf2++ == '.' )break;
					if ( *buf2 && ( access = get_access( strlong(&buffer2[4]) ) ) )
					{
							  if ( !stricmp( buf2, fake_txt ) )
						{
							if ( buf[0]=='/' )strncpy( access->alias, buf, 512 );
							else
							{
								access->alias[0] = '/';
								strncpy( &access->alias[1], buf, 511 );
							}
							strip_trailing( access->alias );
						}
						else if ( !stricmp( buf2, real_txt ) )
						{
							strncpy( access->path, buf, 512 );
							strip_trailing( access->path );
						}
						else if ( !stricmp( buf2, subdirs_txt ) )	access->subdirs = (unsigned short)strlong( buf );
						else if ( !stricmp( buf2, read_txt ) )		access->read = (unsigned short)strlong( buf );
						else if ( !stricmp( buf2, write_txt ) )	access->write = (unsigned short)strlong( buf );
						else if ( !stricmp( buf2, delete_txt ) )	access->delete = (unsigned short)strlong( buf );
					}
				}
			}
			else if ( loading==2 )
			{
				strcat( buffer, "\n" );
				closed = strappend( closed, buffer );
			}
			else if ( loading==3 )
			{
				strcat( buffer, "\n" );
				greeting = strappend( greeting, buffer );
			}
			else if ( loading==4 )
			{
				strcat( buffer, "\n" );
				welcome = strappend( welcome, buffer );
			}
			else if ( loading==5 )
			{
				strcat( buffer, "\n" );
				goodbye = strappend( goodbye, buffer );
			}
		}
		set( MAIN_Open, MUIA_Cycle_Active, open );
		update_users();
	}
	Close( file );
	return TRUE;
}

void load_stats( char *filename )
{
	BPTR file;

	memclr( &stats, sizeof(stats) );
	if ( ( file = Open(filename,MODE_OLDFILE) ) )
	{
		Read( file, &stats, sizeof(stats) );
		Close( file );
	}
}

void save_stats( char *filename )
{
	BPTR file;

	if ( ( file = Open(filename,MODE_NEWFILE) ) )
	{
		Write( file, &stats, sizeof(stats) );
		Close( file );
	}
}

void exit_ftpd( void )
{
	struct ftpdata *ftp, *ftp_next;

	for ( ftp = ftp_list ; ftp ; ftp = ftp_next )
	{
		ftp_next = ftp->next;
		free_ftp(ftp);
	}
	if ( control >= 0 )	CloseSocket(control);
	if ( SocketBase )		CloseLibrary(SocketBase);
	if ( APP_Main )
	{
		DoMethod( APP_Main, MUIM_Application_Save, MUIV_Application_Load_ENV );
		DoMethod( APP_Main, MUIM_Application_Save, MUIV_Application_Load_ENVARC );
		MUI_DisposeObject( APP_Main );
	}
	if ( appicon )			RemoveAppIcon( appicon );
	if ( WorkbenchBase )	CloseLibrary(WorkbenchBase);
	if ( icon )				FreeDiskObject( icon );
	if ( IconBase )		CloseLibrary(IconBase);
	if ( iconport )
	{
		struct Message *msg;
		while ( ( msg = GetMsg(iconport) ) )ReplyMsg( msg );
		DeleteMsgPort( iconport );
	}
	if ( MUIMasterBase )	CloseLibrary(MUIMasterBase);
	if ( DOSBase )			CloseLibrary(DOSBase);
	if ( myPool )			DeletePool( myPool );
}

void free_data( struct ftpdata *ftp )
{
PRINTF( "enter -> free_data()\n" );
	if ( ftp->pdata >= 0 )
	{
		shutdown( ftp->pdata, 2 );
		CloseSocket( ftp->pdata );
	}
	if ( ftp->data >= 0 )
	{
		shutdown( ftp->data, 2 );
		CloseSocket( ftp->data );
	}
	if ( ftp->file || ftp->file2 )
	{
		unsigned long secs, i, cps_change, cps_change2;

		CloseFile( ftp );
		ftp->current +=  ftp->xfer;
		secs = timer(ftp);
		if ( !secs )secs = 1;
		cps_change = ( ftp->current - ftp->start ) / secs;
		for ( i = 0 ; i<3 ; i++ )
		{
			if ( IS_SET(ftp->flags,FLAG_RECV) )
			{
				cps_change2 = cps_change + stats[i].fs_recv_cps * stats[i].fs_recv_files;
				stats[i].fs_recv_files++;
				stats[i].fs_recv_data += ftp->current - ftp->start;
				stats[i].fs_recv_cps = cps_change2 / stats[i].fs_recv_files;
			}
			else
			{
				cps_change2 = cps_change + stats[i].fs_sent_cps * stats[i].fs_sent_files;
				stats[i].fs_sent_files++;
				stats[i].fs_sent_data += ftp->current - ftp->start;
				stats[i].fs_sent_cps = cps_change2 / stats[i].fs_sent_files;
			}
		}
		save_stats( "PROGDIR:ftpd.stats" );
		update_stats();
	}
	ftp->data = ftp->pdata = -1;
	if ( IS_SET(ftp->flags,FLAG_LIST) )DeleteFile(ftp->filename);
	else if ( serial && IS_SET(ftp->flags,FLAG_RECV) && comment )
	{
		char buffer[1024];

		strcpy( ftp->buffer, ftp->path );
		add_path( ftp->buffer, ftp->filename );
		if ( real_dir(ftp,ftp->buffer,buffer) )
		{
//			sprintf( ftp->buffer, "%s%s@%s", (CheckSFV(buffer)?"OK ":"BAD "):"", ftp->user->user, ftp->ip );
			sprintf( ftp->buffer, "%s@%s", ftp->user->user, ftp->ip );
			SetComment( buffer, ftp->buffer );
		}
	}
	ftp->filename[0] = '\0';
	ftp->file = NULL;
	ftp->file2 = 0;
	ftp->start = ftp->current = ftp->size = ftp->cps = ftp->xfer = 0;
	REMOVE_BIT(ftp->flags,FLAG_SEND|FLAG_RECV|FLAG_LIST|FLAG_CONNECTED);
	DateStamp( &ftp->started );
	update_list( ftp );
	update_cps();
PRINTF( "exit -> free_data()\n" );
	return;
}

char	showip[]		= "%lu.%lu.%lu.%lu";
char	hello[]		= VERS " (" DATE ") %segistered [%08.lx]. Ready.\n";

void new_ftp( long control )
{
	struct ftpdata *ftp;
	struct sockaddr_in sock;
	long desc, len = sizeof(struct sockaddr);
	long nonblocking = 1;

	memclr(&sock,sizeof(struct sockaddr_in));
	if ( (desc = accept(control, (struct sockaddr *)&sock, &len) ) < 0)return;
	IoctlSocket( desc, FIONBIO, (char*)&nonblocking );
	if ( !open || ( connections && opencount >= connections ) )
	{
		if ( closed )
		{
			char	buffer[BUFSIZE], *txt = closed, *buf;

			strcpy( buffer, "421-" );
			while( *txt )
			{
				txt = one_line( &buffer[4], txt );
				for ( buf = buffer ; *buf && *buf!='\r' && *buf!='\n' ; buf++ );
				strcpy( buf, "\r\n" );
				send( desc, buffer, strlen(buffer), 0 );
			}
		}
		send( desc, REPLY_421, strlen(REPLY_421), 0 );
		CloseSocket(desc);
		return;
	}
	if ( ( ftp = AllocPooled( myPool, sizeof(struct ftpdata) ) ) )
	{
		struct sockaddr_in port;
		int i;

		ftp->control	= desc;
		ftp->data = ftp->pdata = -1;
		ftp->next		= ftp_list;
		ftp_list			= ftp;
		len = sizeof(struct sockaddr);
		if ( getpeername( ftp->control, (struct sockaddr*)&port, &len ) == 0 )
		{
			char *addr = (char*)&port.sin_addr.s_addr;
			sprintf( ftp->ip, showip, (addr[0]&0xff), (addr[1]&0xff), (addr[2]&0xff), (addr[3]&0xff) );
/*
	10.*
	127.*
	172.16.*
	192.168.*
*/
			if ( (addr[0]&0xff)==10
			 ||  (addr[0]&0xff)==127
			 || ((addr[0]&0xff)==172 && (addr[1]&0xff)==16 )
			 || ((addr[0]&0xff)==192 && (addr[1]&0xff)==168 ) )
			{
				SET_BIT(ftp->flags,FLAG_SPEEDY);
			}
		}
		DateStamp( &ftp->started );
		if ( greeting )output( ftp, greeting, 0 );
		sprintf( ftp->buffer, hello, registered?"R":"Unr", serial );
		output( ftp, ftp->buffer, 220 );
		opencount++;
		update_users();
		DoMethod( MAIN_List, MUIM_NList_InsertSingle, ftp, MUIV_NList_Insert_Bottom );
		for ( i = 0 ; i < 3 ; i++ )stats[i].fs_users_anon++;
		save_stats( "PROGDIR:ftpd.stats" );
		update_stats();
	}
	else CloseSocket(desc);
	return;
}

void free_ftp( struct ftpdata *ftp )
{
	struct outdata *out, *out_next;
	struct ftpdata *prev;
	long i;

	if ( ftp->Win )
	{
		set( ftp->Win, MUIA_Window_Open, FALSE );
		DoMethod( APP_Main, OM_REMMEMBER, ftp->Win );
		MUI_DisposeObject( ftp->Win );
	}
	opencount--;
	update_users();
	for ( i = 0 ; ; i++ )
	{
		DoMethod( MAIN_List, MUIM_NList_GetEntry, i, &prev );
		if ( !prev )break;
		if ( prev != ftp )continue;
		DoMethod( MAIN_List, MUIM_NList_Remove, i );
		break;
	}
	if ( ftp_list == ftp )ftp_list = ftp->next;
	else
	{
		for ( prev = ftp_list ; prev && prev->next != ftp ; prev = prev->next );
		if ( prev )prev->next = ftp->next;
	}
	if ( ftp->control >= 0 )CloseSocket(ftp->control);
	free_data(ftp);
	for ( out = ftp->output ; out ; out = out_next )
	{
		out_next = out->next;
		FreePooled( myPool, out, strlen(out->txt)+sizeof(struct outdata) );
	}
	FreePooled( myPool, ftp, sizeof(struct ftpdata) );
}

char *logfilepath = "PROGDIR:logs";
char *onelogpath = "PROGDIR:ftpd.ftpd_log";

void LogToFile( struct ftpdata *ftp, char *arg1, char *arg2 )
{
	BPTR	file;
	char filename[128];

	if ( !ftp->user )return;
	if ( seperate )
	{
		BPTR	lock;

		if ( !( lock = Lock( logfilepath, ACCESS_READ ) ) )lock = CreateDir( logfilepath );
		UnLock( lock );
		sprintf( filename, "%s/%s.ftpd_log", logfilepath, ftp->user->user );
	}
	else strcpy(filename, onelogpath );
	if ( ( file = Open(filename,MODE_READWRITE) ) )
	{
		char	date[11] = { "XX.XX.XX\0\0\0" }, time[9] = { "XX:XX:XX\0" };
		struct DateTime dt = { { 0, 0, 0 }, FORMAT_CDN, 0, NULL, date, time };

		Seek( file, 0, OFFSET_END );
		DateStamp( &dt.dat_Stamp );
		DateToStr( &dt );
  		FPrintf( file, "%s %s : %s@%s : %s%s%s\n", (ULONG)date, (ULONG)time, (ULONG)ftp->user->user, (ULONG)ftp->ip, (ULONG)arg1, (ULONG)(arg2?" ":""), (ULONG)(arg2?arg2:"") );
		Close( file );
	}
}

BOOL read_from_control( struct ftpdata *ftp )
{
	int iStart, nRead;
	char *buf, *buf2;

	for ( iStart = strlen(ftp->input) ; ; )
	{
		nRead = recv( ftp->control, ftp->input + iStart, BUFSIZE - 10 - iStart, 0 );
		if (nRead > 0)
		{
			iStart += nRead;
			if ( ftp->input[iStart-1] == '\n' || ftp->input[iStart-1] == '\r' )break;
			if ( iStart >= BUFSIZE - 10 )return FALSE;
		}
		else if (nRead == 0)return FALSE;
		else if (Errno() == EWOULDBLOCK)return TRUE;
		else return FALSE;
	}
	ftp->input[iStart] = '\0';
	for ( buf = ftp->input ; *buf && *buf != '\r' && *buf != '\n' ; buf++ );
	if ( *buf )
	{
		BOOL cmd = TRUE;
		int len = 0;

		for ( buf = ftp->input ; isspace(*buf) ; buf++ );
		*ftp->args = '\0';
		for ( buf2 = ftp->command ; *buf && *buf != '\r' && *buf != '\n' ; )
		{
			if ( *buf==(char)0x08 && len>0 )
			{
				buf2--;
				len--;
				buf++;
				continue;
			}
			if ( *buf=='\e' )
			{
				buf++;
				continue;
			}
			if ( *buf==(char)0xff )	// Telnet control sequences can appear *anywhere* - discouraged
			{
				switch( *++buf )
				{
					case (char)0xff:	buf++;						break;
					case (char)0xfe:	if ( *++buf )buf++;		break;
					case (char)0xfd:	if ( *++buf )
											{
												char tmp[3]={0xff,0xfc,*buf++};
												send( ftp->control, tmp, 3, 0);	// Reply *immediately*
											}
											break;
					case (char)0xfc:	if ( *++buf )buf++;		break;
					case (char)0xfb:	if ( *++buf )
											{
												char tmp[3] = { 0xff, 0xfe, *buf++ };
												send( ftp->control, tmp, 3, 0);	// Reply *immediately*
											}
											break;
					case (char)0xf4:	if ( *++buf )buf++;		break;		// Needed for AmFTP...
					case (char)0x00:									break;
					default:				buf++;						break;
				}
				continue;
			}
//			if ( !isascii(*buf) )	// Safety buffer - nothing should be non-ascii...
//			{
//				buf++;
//				continue;
//			}
			if ( cmd && *buf==' ' )
			{
				cmd = FALSE;
				len = 0;
				*buf2 = '\0';
				buf2 = ftp->args;
				while ( isspace(*buf) )buf++;
				continue;
			}
			if ( cmd )
			{
				if ( len < 4 )*buf2++ = toupper(*buf++);
				else buf++;
				len++;
				continue;
			}
			*buf2++ = *buf++;
		}
		*buf2 = '\0';
		len = strlen(ftp->args);
		while ( isspace(ftp->args[len]) )ftp->args[--len] = '\0';
		while ( isspace(*buf) )buf++;
		for ( buf2 = ftp->input ; *buf ; )*buf2++ = *buf++;
		*buf2 = '\0';
		if ( !*ftp->command )return TRUE;
		if ( ftp->List && ftp->loglevel >= 2 )
		{
#ifdef NOHIDEPASS
			sprintf( ftp->buffer, "\eb%s\en %s", ftp->command, ftp->args );
#else
			sprintf( ftp->buffer, "\eb%s\en %s", ftp->command, (stricmp(ftp->command,"pass")?ftp->args:"********") );
#endif
			DoMethod( ftp->List, MUIM_NList_InsertSingle, ftp->buffer, MUIV_NList_Insert_Bottom );
			DoMethod( ftp->List, MUIM_NList_Jump, MUIV_NList_Jump_Bottom );
		}
		if ( logfile && ftp->loglevel>=2 )LogToFile( ftp, ftp->command, ftp->args );
		update_ftp(ftp);
		for ( iStart = 0 ; commands[iStart].name ; iStart++ )
		{
			if ( !stricmp( commands[iStart].name, ftp->command ) )
			{
				BOOL res;
				if ( IS_SET(commands[iStart].flags,NEED_UNIMP) )
				{
					output( ftp, REPLY_502, 502 );
					return TRUE;
				}
				if ( ( ftp->file || ftp->file2 ) && !IS_SET(commands[iStart].flags,NEED_XFER) )
				{
					output( ftp, REPLY_503, 503 );
					return TRUE;
				}
				if ( !IS_SET(commands[iStart].flags,NEED_LOGIN) && !IS_SET(ftp->flags,FLAG_ONLINE) )
				{
					output( ftp, REPLY_530, 530 );
					return TRUE;
				}
				if ( ftp->access
				&& ( ( IS_SET(commands[iStart].flags,NEED_READ) && !IS_SET(ftp->user->access,ftp->access->read) )
				  || ( IS_SET(commands[iStart].flags,NEED_WRITE) && !IS_SET(ftp->user->access,ftp->access->write) )
				  || ( IS_SET(commands[iStart].flags,NEED_DELETE) && !IS_SET(ftp->user->access,ftp->access->delete) ) ) )
				{
					output( ftp, REPLY_550, 550 );
					return TRUE;
				}
				if ( ( IS_SET(commands[iStart].flags,NEED_ARGS) && !ftp->args[0] ) )
				{
					output( ftp, REPLY_501, 501 );
					return TRUE;
				}
				if ( ftp->List && ftp->loglevel==1 )
				{
#ifdef NOHIDEPASS
					sprintf( ftp->buffer, "\eb%s\en %s", ftp->command, ftp->args );
#else
					sprintf( ftp->buffer, "\eb%s\en %s", ftp->command, (stricmp(ftp->command,"pass")?ftp->args:"********") );
#endif
					DoMethod( ftp->List, MUIM_NList_InsertSingle, ftp->buffer, MUIV_NList_Insert_Bottom );
					DoMethod( ftp->List, MUIM_NList_Jump, MUIV_NList_Jump_Bottom );
					update_list( ftp );
				}
#ifdef NOHIDEPASS
				if ( logfile && ftp->loglevel==1 )LogToFile( ftp, ftp->command, ftp->args );
#else
				if ( logfile && ftp->loglevel==1 )LogToFile( ftp, ftp->command, (stricmp(ftp->command,"pass")?ftp->args:"********") );
#endif
				if ( !( ftp->file || ftp->file2 ) && ( noop || ( !noop && stricmp(commands[iStart].name,"NOOP") ) ) )DateStamp( &ftp->started );
				{
					struct Process	*myProcess = (APTR) FindTask(NULL);
					APTR				 myWindow = myProcess->pr_WindowPtr;
					myProcess->pr_WindowPtr = (APTR)-1;
					res = (commands[iStart].command)(ftp);
					myProcess->pr_WindowPtr = myWindow;
				}
				update_list(ftp);
				return TRUE;
			}
		}
		output( ftp, REPLY_500, 500 );
	}
	return TRUE;
}

char o3lu[] = "%03.lu";

void output( struct ftpdata *ftp, char *txt, long reply )
{
	struct outdata *out;
	char buffer[BUFSIZE], temp[BUFSIZE], *buf, *buf2;
	unsigned long len;

	if ( reply>0 && ( reply<100 || reply>599 ) )return;
	if ( reply>=0 )
	{
		if ( IS_SET(ftp->flags,FLAG_QUIET) && !reply )return;
		sprintf( temp, o3lu, reply );
		temp[3] = reply?' ':'-';
	}
	else if ( reply!=-1 )
	{
		sprintf( temp, o3lu, -reply );
		buffer[3] = ' ';
	}
	for ( out = ftp->output ; out && out->next ; out = out->next );
	while( *txt )
	{
		if ( reply!=-1 )txt = one_line( &temp[4], txt );
		else txt = one_line( temp, txt );
		for ( buf = temp, buf2 = buffer ; *buf ; )
		{
			if ( *buf=='$' )
			{
				BOOL nolimit = FALSE;
				*buf2 = '\0';
				buf++;
				switch( *buf++ )
				{
					case 'u':
					case 'U':
						sprintf( buf2, "%lu", opencount );
						break;
					case 'm':
					case 'M':
						if ( connections )sprintf( buf2, "%lu", connections );
						else nolimit = TRUE;
						break;
					case 'c':
					case 'C':
						if ( maxcps )sprintf( buf2, "%lu", maxcps );
						else nolimit = TRUE;
						break;
					case 's':
					case 'S':
						if ( maxcps && opencount )sprintf( buf2, "%lu", maxcps/opencount );
						else nolimit = TRUE;
						break;
					default:
						// unknown code...
						break;
				}
				if ( nolimit )sprintf( buf2, "%s", GetString(MSG_UNLIMITED) );
				buf2 += strlen(buf2);
			}
			else *buf2++ = *buf++;
		}
		strcpy( buf2, "\r\n" );
		len = strlen( buffer ) + sizeof(struct outdata);
		if ( out )
		{
			if ( reply && reply!=-1 )
			{
				struct outdata *tmp;
				for ( tmp = ftp->output ; tmp ; tmp = tmp->next )if ( !strncmp(tmp->txt,"000-",4) )
				{
					tmp->txt[0] = buffer[0];
					tmp->txt[1] = buffer[1];
					tmp->txt[2] = buffer[2];
					tmp->txt[3] = '-';
				}
			}
			out->next = AllocPooled( myPool, len );
			out = out->next;
		}
		else out = ftp->output = AllocPooled( myPool, len );
		strcpy( out->txt, buffer );
	}
}

void sendfile( struct ftpdata *ftp, char *filename )
{
	BPTR	file;
	char buffer[128];
	int i;

	if ( registered && ( file = Open( filename, MODE_OLDFILE ) ) )
	{
		while ( FGets( file, buffer, 100 ) )
		{
			i = strlen(buffer)-1;
			if ( buffer[i]!='\n' )
			{
				buffer[++i] = '\n';
				buffer[++i] = '\0';
			}
			output( ftp, buffer, 0 );
		}
		Close( file );
	}
}

BOOL write_to_control( struct ftpdata *ftp )
{
	struct outdata *out;
	unsigned long len;

	for ( out = ftp->output ; out ; out = ftp->output )
	{
		if ( out->txt[0]=='0' )return TRUE;
		len = strlen(out->txt);
		if ( send( ftp->control, out->txt, len, 0) < 0 )return FALSE;
		if ( ftp->List && ftp->loglevel>2 )
		{
			char buffer[BUFSIZE], *buf, *from;

			for ( from = out->txt, buf = buffer ; *from && *from!='\r' && *from!='\n' ; *buf++ = *from++ );
			*buf = '\0';
			DoMethod( ftp->List, MUIM_NList_InsertSingle, buffer, MUIV_NList_Insert_Bottom );
			DoMethod( ftp->List, MUIM_NList_Jump, MUIV_NList_Jump_Bottom );
			if ( logfile )LogToFile( ftp, buffer, NULL );
		}
		ftp->output = out->next;
		FreePooled( myPool, out, len+sizeof(struct outdata) );
	}
	update_ftp(ftp);
	update_list(ftp);
	return TRUE;
}

#define	MAX_FILEBUF	ASYNCBUF
char	filebuffer[MAX_FILEBUF];

void read_from_data( struct ftpdata *ftp )
{
	long	len, maxlen = 1024;

PRINTF( "enter -> read_from_data()\n" );
	if ( !IS_SET(ftp->flags,FLAG_SPEEDY) && my_cps )maxlen = my_cps - ftp->xfer;
	else if ( ftp->cps )maxlen = ftp->cps * 2;
	maxlen = range( 16, maxlen, MAX_FILEBUF );
	len = recv( ftp->data, filebuffer, MAX_FILEBUF, 0 );
	if (len > 0)
	{
		if ( WriteFile(ftp,filebuffer,len)==-1 )free_data(ftp);
		else ftp->xfer += len;
		return;
	}
	else if (len == 0)
	{
		free_data(ftp);
		output( ftp, REPLY_226, 226 );
	}
	else if (Errno() != EAGAIN)
	{
		free_data(ftp);
		output( ftp, REPLY_226, 226 );
	}
	update_ftp(ftp);
PRINTF( "exit -> read_from_data()\n" );
	return;
}

void write_to_data( struct ftpdata *ftp )
{
	long	len, maxlen = 1024, sent;

PRINTF( "enter -> write_to_data()\n" );
	if ( !IS_SET(ftp->flags,FLAG_SPEEDY) && my_cps )maxlen = my_cps - ftp->xfer;
	else if ( ftp->cps )maxlen = ftp->cps * 2;
	maxlen = range( 16, maxlen, MAX_FILEBUF );
	len = ReadFile( ftp, filebuffer, maxlen );
	if ( len == 0 )
	{
		free_data(ftp);
		output( ftp, REPLY_226, 226 );
	}
	else if ( len==-1 )
	{
		free_data(ftp);
		output( ftp, REPLY_426, 426 );
	}
	else
	{
		sent = send( ftp->data, filebuffer, len, 0 );
		if (sent >= 0)
		{
			if ( sent < len )SeekRelative( ftp, sent-len );
			ftp->xfer += sent;
			return;
		}
		else if ( Errno() != EAGAIN )
		{
			free_data(ftp);
			output( ftp, REPLY_426, 426 );
		}
	}
	update_ftp(ftp);
PRINTF( "exit -> write_to_data()\n" );
	return;
}

#define	KEYSIZE	(1<<10)
#define	OR1a		3337
#define	OR1b		8652
#define	OFFSET1a	(726^OR1a)
#define	OFFSET1b	(385^OR1b)

int main(void)
{
	struct ftpdata *ftp, *ftp_next;
	int ret = RETURN_OK;

	SysBase = (*((struct Library **)4));
	if ( ( LocaleBase = OpenLibrary( "locale.library", 38 ) ) != NULL )
	{
		catalog = OpenCatalogA( NULL, "ftpd.catalog", NULL );
		locale  = OpenLocale( NULL );
	}
	if ( !init_ftpd() )ret = RETURN_ERROR;
	else
	{
		char	keyname[] = { 'f'^85, 't'^85, 'p'^85, 'd'^85, '.'^85, 'k'^85, 'e'^85, 'y'^85, '\0' };
		char	keyloc1[] = { 'S'^47, ':'^47, '\0' };
		char	keyloc2[] = { 'K'^93, 'E'^93, 'Y'^93, 'F'^93, 'I'^93, 'L'^93, 'E'^93, 'S'^93, '\0' };
		char filename[512];
		ULONG signals, x;
		BPTR	keyfile = 0;
		int	i = 0;

		for ( i = 0 ; keyname[i] ; keyname[i++] ^= 85 );
		for ( i = 0 ; keyloc1[i] ; keyloc1[i++] ^= 47 );
		for ( i = 0 ; keyloc2[i] ; keyloc2[i++] ^= 93 );
		strcpy( filename, keyloc1 );
		registered = NULL;
		keyfile = Open( keyname, MODE_OLDFILE );
		if ( !keyfile && ( !AddPart( filename, keyname, 512 ) || !( keyfile = Open( filename, MODE_OLDFILE ) ) ) )
		{
			if ( GetVar( keyloc2, filename, 512, 0 ) != -1 && AddPart( filename, keyname, 512 ) )
				keyfile = Open( filename, MODE_OLDFILE );
		}
		if ( keyfile )
		{
			char	*n, *m;
			long	myid = 0;
			char	key[KEYSIZE+1], o1, o2, crc = 0, c;
			int	p1 = OFFSET1a, p2 = OFFSET1b;
			BOOL	ok = TRUE;

			if ( Read( keyfile, key, KEYSIZE ) < KEYSIZE )ok = FALSE;
			else if ( ( ok = !strncmp(key,keyname,4) ) )ok = key[4]=='\1';
			p1 ^= OR1a;
			p2 ^= OR1b;
			for ( n = filename, m = (char*)&myid, i = 0 ; ok ; i++ )
			{
				o1 = key[p1];
				o2 = key[p2];
				p1 = (p1+o1)&(KEYSIZE-1);
				p2 = (p2+o2)&(KEYSIZE-1);
				c = o1^o2;

					  if ( i==0 )m[0] = c;
				else if ( i==1 )m[1] = c;
				else if ( i==2 )m[2] = c;
				else if ( i==3 )m[3] = c;
				else if ( i>=4 && i%4==0 )
				{
					if ( c!=crc )ok = FALSE;
				}
				else if ( !( *n++ = c ) )break;
				crc ^= c;
			}
			if ( ok )
			{
				serial = myid;
				registered = strdup( filename );
				set( APP_Main, MUIA_Application_Commands, mui_commands );
			}
			Close( keyfile );
		}
		set( PREFS_List, MUIA_List_Active, 0 );
		set( WIN_Main, MUIA_Window_Open, TRUE );
//		set( WIN_Stats, MUIA_Window_Open, stats_open );
		while ( ( x = DoMethod(APP_Main,MUIM_Application_NewInput,&signals) ) != MUIV_Application_ReturnID_Quit )
		{
			struct timeval	waittime = { 1, 0 };
			fd_set	in_set;
			fd_set	out_set;
			fd_set	exc_set;
			long		maxdesc = 0;

			SET_BIT(signals,SIGBREAKF_CTRL_C|SIGBREAKF_CTRL_D);
			if ( iconport )SET_BIT(signals,1<<(iconport->mp_SigBit));
			FD_ZERO(&in_set);
			FD_ZERO(&out_set);
			FD_ZERO(&exc_set);
			if ( control != -1 )
			{
				FD_SET(control, &in_set);
				maxdesc = control;
			}

			if ( maxcps || !registered )
			{
				for ( my_cps = 0, ftp = ftp_list ; ftp ; ftp = ftp->next )if ( (ftp->file || ftp->file2) && !IS_SET(ftp->flags,FLAG_SPEEDY) )my_cps++;
				if ( my_cps )my_cps = (registered?maxcps:3072>maxcps?3072:maxcps) / my_cps;
			}
			else my_cps = 0;
			for ( ftp = ftp_list ; ftp ; ftp = ftp->next )
			{
				unsigned long seconds;

				if ( ftp->control >= 0 )
				{
					maxdesc = maxdesc>ftp->control?maxdesc:ftp->control;
					FD_SET(ftp->control, &in_set);
					if ( ftp->output && ftp->output->txt[0]!='0' )FD_SET(ftp->control, &out_set);
					FD_SET(ftp->control, &exc_set);
				}
				if ( ftp->data>=0 && IS_SET(ftp->flags,FLAG_CONNECTED) )
				{
					maxdesc = maxdesc>ftp->data?maxdesc:ftp->data;
					if ( IS_SET(ftp->flags,FLAG_RECV|FLAG_SEND) )
					{
						if ( IS_SET(ftp->flags,FLAG_SPEEDY) || !maxcps || ( (ftp->file || ftp->file2) && maxcps && my_cps && ftp->xfer<my_cps ) )
						{
							if ( IS_SET(ftp->flags,FLAG_RECV) )FD_SET(ftp->data, &in_set);
							if ( IS_SET(ftp->flags,FLAG_SEND) )FD_SET(ftp->data, &out_set);
						}
					}
					FD_SET(ftp->data, &exc_set);
				}
				else if ( ftp->pdata >= 0 )
				{
					maxdesc = maxdesc>ftp->pdata?maxdesc:ftp->pdata;
					FD_SET(ftp->pdata, &in_set);
				}
				seconds = timer(ftp);
				if ( !(ftp->file || ftp->file2) )
				{
					if ( seconds==30 || seconds==60 )update_list(ftp);
					if ( timeout>0 && seconds>(timeout*60) )
					{
						output( ftp, "Connection timeout - goodbye\n", 421 );
						SET_BIT(ftp->flags,FLAG_QUIT);
					}
				}
				else if ( seconds!=ftp->update )
				{
					ftp->update		= seconds;
					if ( ftp->xfer )ftp->cps = ftp->xfer;
					else ftp->cps >>= 1;
					ftp->current  += ftp->xfer;
					ftp->xfer		= 0;
					update_ftp(ftp);
					update_cps();
				}
			}
			if ( WaitSelect(maxdesc+1, &in_set, &out_set, &exc_set, &waittime, &signals) < 0 )return(25);
			if ( IS_SET(signals,SIGBREAKF_CTRL_C) )break;
//			if ( IS_SET(signals,SIGBREAKF_CTRL_D) )
//			{
//				char	buffer[128];
//				strcpy( buffer, "ENV:" );
//				AddPart( buffer, configname, 128 );
//				load_config( buffer );
//				continue;
//			}
			if ( signals )
			{
				if ( iconport && IS_SET(signals,1<<(iconport->mp_SigBit)) )
				{
					struct AppMessage *msg;
					while ( ( msg = (struct AppMessage*)GetMsg(iconport) ) )
					{
						switch( msg->am_Class )
						{
							case AMCLASSICON_Open:
								set( APP_Main, MUIA_Application_Iconified, FALSE );
								break;
							case AMCLASSICON_Snapshot:
								iconx = icon->do_CurrentX;
								icony = icon->do_CurrentY;
								break;
							case AMCLASSICON_UnSnapshot:
								iconx = NO_ICON_POSITION;
								icony = NO_ICON_POSITION;
								break;
							default:
								break;
						}
						ReplyMsg( (struct Message*)msg );
					}
				}
				continue;
			}
			if ( FD_ISSET(control, &in_set))new_ftp(control);
			for ( ftp = ftp_list ; ftp ; ftp = ftp_next )
			{
				ftp_next = ftp->next;
				if ( FD_ISSET(ftp->control, &exc_set) )
				{
					free_ftp(ftp);
					continue;
				}
				if ( FD_ISSET(ftp->control, &out_set) )
				{
					if ( !write_to_control(ftp) || IS_SET(ftp->flags,FLAG_QUIT) )free_ftp(ftp);
					continue;
				}
				if ( FD_ISSET(ftp->control, &in_set) )
				{
					if ( !read_from_control(ftp) )free_ftp(ftp);
					continue;
				}
				if (ftp->data >= 0 )
				{
					if (FD_ISSET(ftp->data, &exc_set))
					{
						free_data(ftp);
						continue;
					}
					if (IS_SET(ftp->flags,FLAG_RECV) && FD_ISSET(ftp->data, &in_set))read_from_data(ftp);
					if (IS_SET(ftp->flags,FLAG_SEND) && FD_ISSET(ftp->data, &out_set))write_to_data(ftp);
				}
				else if ( ftp->pdata>=0  )
				{
					if ( FD_ISSET(ftp->pdata, &in_set))
					{
						struct sockaddr_in sock;
						long size = sizeof(sock);

						memclr(&sock,sizeof(struct sockaddr_in));
						if ( ( ftp->data = accept(ftp->pdata, (struct sockaddr*)&sock, &size) ) < 0 )
						{
							free_data(ftp);
							output( ftp, REPLY_425, 425 );
						}
						else
						{
							long nonblocking = 1;

							IoctlSocket( ftp->data, FIONBIO, (char*)&nonblocking );
							SET_BIT(ftp->flags,FLAG_CONNECTED);
							DateStamp( &ftp->started );
						}
						update_list( ftp );
					}
					else if ( FD_ISSET(ftp->pdata, &exc_set) )
					{
						free_data(ftp);
						continue;
					}
				}
			}
		}
	}
	exit_ftpd();
	if ( LocaleBase )
	{
		CloseLocale( locale );
		CloseCatalog( catalog );
		CloseLibrary( LocaleBase );
	}
	return(ret);
}
