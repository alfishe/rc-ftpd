/*****************************************************************************************************
 * rc-ftpd �2000 Robin Cloutman <rycochet2@yahoo.com>                                                *
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
#include	<dos/dosasl.h>
#include	<exec/memory.h>
#include	<string.h>
#include	<sys/socket.h>
#include	<sys/filio.h>

#include	"asyncio.h"

#include	<clib/alib_protos.h>
#include	<inline/exec.h>
#include	<inline/socket.h>
#include	<inline/dos.h>

//#include	<libraries/identify.h>
//#include	<inline/identify.h>

#include "ftpd.h"

BOOL cmd_feat(struct ftpdata *ftp)	// slight copy of output() ;-)
{
	unsigned long i;
	char *txt[] =
	{
		" MDTM",
		" SIZE",
		" REST STREAM",
		" TVFS",
		NULL
	};

	output( ftp, "211-Extensions supported:", -1 );
	for ( i = 0 ; txt[i] ; i++ )output( ftp, txt[i], -1 );
	output( ftp, "211 end", -1 );
	return FALSE;
}

void add_path( char *to, char *add )
{
	if ( !strcmp(add,":") )
	{
		if ( *to=='/' )to[1] = '\0';
		else to[0] = '\0';
	}
	else if ( strchr(add,':') || *add=='/' )strcpy( to, add );
	else
	{
		int i = strlen(to)-1;
		if ( to[i]!='/' && to[i]!=':' )strncat( to, "/", 512 );
		strncat( to, add, 512 );
	}
}

struct accessdata *real_dir( struct ftpdata *ftp, char *path, char *buffer )
{
	struct accessdata	*access = NULL, *last;
	char	*buf;
	int path_len = 0, last_len = 0, access_len = 0, i;

	if ( buffer )buffer[0] = '\0';
	if ( !*path )return(NULL);
	for ( buf = path ; *buf ; buf++ )if ( buf[0]==':' || ( buf[0]=='/' && buf[1]=='/' ) )
	{
		if ( buffer )output( ftp, REPLY_501, 501 );
		return NULL;
	}
	strip_trailing( path );
	path_len = strlen(path);
	for ( last = access_list ; last ; last = last->next )
	{
		if ( !last->alias )continue;
		if ( last->path && *last->path && !strchr(last->path,':') )continue;
		last_len = strlen(last->alias);
		if ( last_len>path_len )continue;
		if ( !strnicmp(last->alias,path,last_len)
		  && ( !access || access_len<last_len ) )
		{
			i = last_len;
			if ( i>1 && path[i] && path[i]!='/' )continue;
			if ( path[i] )i++;
			if ( strchr(&path[i],'/') && !IS_SET((ftp->user?ftp->user->access:1),last->subdirs) )
			{
				access = NULL;
				access_len = 0;
				continue;
			}
			access = last;
			access_len = strlen(access->alias);
		}
	}
	if ( !access || !IS_SET((ftp->user?ftp->user->access:1),(access->read|access->write|access->delete)) )
	{
		if ( buffer )output( ftp, REPLY_550, 550 );
		return(NULL);
	}
	if ( buffer && access->path )
	{
		strcpy( buffer, access->path );
		if ( strlen(path) > strlen(access->alias) )
		{
			i = access_len;
			if ( path[i]=='/' )i++;
			AddPart( buffer, &path[i], 512 );
		}
//		for ( buf = buffer ; *buf ; buf++ )if ( ( buf[0] == '/' || buf[0] == ':' ) && buf[1] == '/' )
//		{
//			output( ftp, REPLY_501, 501 );
//			return NULL;
//		}
	}
	return(access);
}

void do_file( struct ftpdata *ftp, long filemode )
{
	if ( ftp->data==-1 && ftp->pdata==-1 )
	{
		free_data(ftp);
		output( ftp, REPLY_503, 503 );
		return;
	}
	if ( IS_SET(ftp->flags,FLAG_LIST) )
	{
		if ( !OpenFile( ftp, ftp->filename, filemode ) )
		{
			output( ftp, REPLY_550, 550 );
			return;
		}
	}
	else
	{
		char path[BUFSIZE];

		strcpy( ftp->buffer, ftp->path );
		add_path( ftp->buffer, ftp->filename );
		if ( !real_dir(ftp,ftp->buffer,path) || !OpenFile( ftp, path, filemode ) )
		{
			output( ftp, REPLY_550, 550 );
			return;
		}
		ftp->current = ftp->start;
		if ( filemode==MODE_APPEND )ftp->current = ftp->start = SeekEnd( ftp );
		else if ( ftp->start )SeekFile( ftp, ftp->start );
	}
	if ( filemode==MODE_READ )
	{
		struct FileInfoBlock fib __attribute__ ((__aligned__(4)));
		if ( ( ftp->file && ExamineFH( ftp->file->af_File, &fib ) ) || ( ftp->file2 && ExamineFH( ftp->file2, &fib ) ) )ftp->size = fib.fib_Size;
	}
	DateStamp( &ftp->started );
//	if ( ftp->data>=0 )output( ftp, REPLY_125, 125 );
//	else
	output( ftp, REPLY_150, 150 );
	update_list( ftp );
	return;
}

BOOL cmd_abor(struct ftpdata *ftp)
{
	if ( ftp->file || ftp->file2 || ftp->data>=0 || ftp->pdata>=0 )
	{
		output( ftp, REPLY_226, 226 );
		free_data(ftp);
	}
	output( ftp, REPLY_200, 200 );
	return TRUE;
}

BOOL cmd_noop(struct ftpdata *ftp)
{
	output( ftp, REPLY_200, 200 );
	return FALSE;
}

BOOL cmd_allo(struct ftpdata *ftp)
{
	output( ftp, REPLY_202, 202 );
	return FALSE;
}

BOOL cmd_acct(struct ftpdata *ftp)
{
	output( ftp, REPLY_202, 202 );
	return FALSE;
}

BOOL cmd_stru(struct ftpdata *ftp)
{
	if ( UPPER(ftp->args[0])=='F' )output( ftp, REPLY_200, 200 );
	else output( ftp, REPLY_504, 504 );
	return FALSE;
}

BOOL cmd_mode(struct ftpdata *ftp)
{
	if ( UPPER(ftp->args[0])=='S' )output( ftp, REPLY_200, 200 );
	else output( ftp, REPLY_504, 504 );
	return FALSE;
}

BOOL cmd_syst(struct ftpdata *ftp)
{
	output( ftp, "AmigaOS Type: L8", 215 );
	return FALSE;
}

BOOL cmd_quit(struct ftpdata *ftp)
{
	if ( goodbye )output( ftp, goodbye, 0 );
	output( ftp, REPLY_221, 221 );
	SET_BIT(ftp->flags,FLAG_QUIT);
	return TRUE;
}

BOOL cmd_type(struct ftpdata *ftp)
{
	switch ( ftp->args[0] )
	{
		case 'a':
		case 'A':
			REMOVE_BIT(ftp->flags,FLAG_BINARY);
			break;

		case 'i':
		case 'I':
		case 'b':
		case 'B':
			SET_BIT(ftp->flags,FLAG_BINARY);
			break;

		default:
			output( ftp, REPLY_504, 504 );
			return FALSE;
	}
	output( ftp, REPLY_200, 200 );
	return TRUE;
}

BOOL cmd_user(struct ftpdata *ftp)
{
	struct userdata *user;
	char *username;

	if ( !stricmp("anon",ftp->args) || !stricmp("ftp",ftp->args) )username = "anonymous";
	else username = ftp->args;
	for ( user = user_list ; user ; user = user->next )if ( *user->user && !stricmp(user->user,username) )break;
	if ( !( ftp->user = user ) )ftp->user = (void*)-1;
	else if ( !ftp->loglevel && ( ftp->loglevel = ftp->user->loglevel ) )start_log( ftp );
	output( ftp, REPLY_331, 331 );
	return TRUE;
}

BOOL cmd_pass(struct ftpdata *ftp)
{
	if ( !ftp->user || IS_SET(ftp->flags,FLAG_ONLINE) )output( ftp, REPLY_503, 503 );
	else if ( ftp->user==(void*)-1 || ( !*ftp->user->pass && !anon ) )
	{
		ftp->user = NULL;
		output( ftp, REPLY_530, 530 );
		return FALSE;
	}
	else
	{
		struct ftpdata *prev;
		int i = ftp->user->sessions;
		char *pass = ftp->args;

		if ( pass[0] == '-' )
		{
			SET_BIT(ftp->flags,FLAG_QUIET);
			pass++;
		}
		else REMOVE_BIT(ftp->flags,FLAG_QUIET);
		if ( ftp->user->pass && *ftp->user->pass && strcmp(ftp->user->pass,ftp->args) )
		{
			output( ftp, REPLY_530, 530 );
			ftp->user = NULL;
			return FALSE;
		}
		if ( i )for ( prev = ftp_list ; prev ; prev = prev->next )
		{
			if ( prev->user==ftp->user )
			{
				if ( ( ftp->user->pass && *ftp->user->pass && prev->user->pass && *prev->user->pass ) || !strcmp(ftp->ip,prev->ip) )
				{
					if ( !i-- )
					{
						output( ftp, REPLY_530, 530 );
						ftp->user = NULL;
						return FALSE;
					}
				}
			}
		}
		if ( welcome )output( ftp, welcome, 0 );
		output( ftp, REPLY_230, 230 );
		SET_BIT(ftp->flags,FLAG_ONLINE);
		strcpy( ftp->buffer, "/" );
		if ( ( ftp->access = real_dir( ftp, ftp->buffer, NULL ) ) )
		{
			strcpy( ftp->path, ftp->access->alias );
			strcpy( ftp->cd, ftp->access->path );
		}
		if ( ftp->user->pass && *ftp->user->pass )
		{
			int i;
			for ( i = 0 ; i < 3 ; i++ )
			{
				stats[i].fs_users_account++;
				if ( stats[i].fs_users_anon-- == 0 )stats[i].fs_users_anon = 0;
			}
			save_stats( "PROGDIR:ftpd.stats" );
			update_stats();
		}
	}
	return TRUE;
}

BOOL cmd_cwd(struct ftpdata *ftp)
{
	struct accessdata	*access = NULL;
	char buffer[BUFSIZE], path[BUFSIZE];

	*buffer = '\0';
	if ( !strcmp("..",ftp->args) || !strcmp("/",ftp->args) )return(cmd_cdup(ftp));
	strncpy( path, ftp->path, BUFSIZE );
	add_path( path, ftp->args );
	if ( *path )
	{
		if ( !( access = real_dir(ftp,path,buffer) ) )return FALSE;
		if ( *buffer )
		{
			struct FileInfoBlock fib __attribute__ ((__aligned__(4)));
			BPTR	lock;

			if ( !( lock = Lock( buffer, ACCESS_READ ) ) )
			{
				output( ftp, REPLY_553, 553 );
				return FALSE;
			}
			if ( !Examine( lock, &fib ) || fib.fib_DirEntryType <= 0 )
			{
				output( ftp, REPLY_550, 550 );
				UnLock(lock);
				return FALSE;
			}
			UnLock(lock);
		}
	}
	ftp->access = access;
	strncpy( ftp->path, path, 512 );
	strncpy( ftp->cd, buffer, 512 );
	if ( serial && message && AddPart( buffer, ".message", 512 ) )sendfile( ftp, buffer );
	output( ftp, REPLY_250, 250 );
	*ftp->filename = '\0';
	return TRUE;
}

BOOL cmd_cdup(struct ftpdata *ftp)
{
	struct accessdata	*access = NULL;
	char buffer[BUFSIZE], path[BUFSIZE];
	int i;

	*buffer = '\0';
	strncpy( path, ftp->path, 512 );
	if ( *path )
	{
		strip_trailing( path );
		if ( strcmp(path,"/") )
		{
			i = strlen(path);
			while ( i>1 && path[i]!='/' )i--;
			path[i] = '\0';
		}
		if ( !( access = real_dir(ftp,path,buffer) ) )return FALSE;
		if ( *buffer )
		{
			struct FileInfoBlock fib __attribute__ ((__aligned__(4)));
			BPTR	lock;

			if ( !( lock = Lock( buffer, ACCESS_READ ) ) )
			{
				output( ftp, REPLY_553, 553 );
				return FALSE;
			}
			if ( !Examine( lock, &fib ) || fib.fib_DirEntryType <= 0 )
			{
				output( ftp, REPLY_550, 550 );
				UnLock(lock);
				return FALSE;
			}
			UnLock(lock);
		}
	}
	ftp->access = access;
	strncpy( ftp->path, path, 512 );
	strncpy( ftp->cd, buffer, 512 );
	if ( serial && message && AddPart( buffer, ".message", 512 ) )sendfile( ftp, buffer );
	output( ftp, REPLY_250, 250 );
	*ftp->filename = '\0';
	return TRUE;
}

BOOL cmd_clnt(struct ftpdata *ftp)
{
	SET_BIT(ftp->flags,FLAG_CLNT);
	output( ftp, "Command okay.", 200 );
	return TRUE;
}

char	*months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

#define	LIST_LONG	(1<<0)
#define	LIST_MLST	(1<<1)
#define	LIST_MLSD	(1<<2)

void list_dir(struct ftpdata *ftp, int flag )
{
	struct accessdata *access;
	struct DateStamp	ds;
	char path[BUFSIZE], buffer[BUFSIZE], *buf, *buf2, *read, *write, *delete;
	BPTR	lock, file = 0;
	BOOL	arg_F = FALSE, arg_l = IS_SET(flag,LIST_LONG)?TRUE:FALSE;

	DateStamp( &ds );
	access = ftp->access;
	strcpy( ftp->buffer, ftp->cd );
	strcpy( path, ftp->path );
	if ( !IS_SET(flag,LIST_MLST|LIST_MLSD) && *ftp->args )
	{
		buf = ftp->args;
		while ( *buf == '-' )
		{
			buf = one_word( ftp->buffer, buf );
			if ( strchr( ftp->buffer, 'F' ) )arg_F = TRUE;
			if ( strchr( ftp->buffer, 'l' ) )arg_l = TRUE;
		}
		if ( *buf )
		{
			add_path( path, buf );
			if ( !( access = real_dir(ftp,path,ftp->buffer) ) )
			{
				output( ftp, REPLY_550, 550 );
				free_data( ftp );
				return;
			}
		}
		else strcpy( ftp->buffer, ftp->cd );
	}
	sprintf( buffer, lsfile, (ULONG)ftp );
	strcpy( ftp->filename, listpath );
	AddPart( ftp->filename, buffer, 512 );
	if ( !( file = Open( ftp->filename, MODE_NEWFILE ) ) )
	{
		output( ftp, REPLY_550, 550 );
		free_data( ftp );
		return;
	}
	else
	{
		char	checkbuffer[BUFSIZE];
		int	pathlen = strlen(path), cdlen = strlen(ftp->buffer);

		if ( *ftp->buffer )
		{
			if ( !(lock = Lock( ftp->buffer, ACCESS_READ)) )
			{
				output( ftp, REPLY_550, 550 );
				free_data( ftp );
				Close( file );
				return;
			}
			else
			{
				struct AnchorPath ap __attribute__ ((__aligned__(4)));
				LONG	error;
				char *pat = list;
				BPTR oldlock ;

				oldlock = CurrentDir( lock );
				if ( !*pat )pat = "#?";
				ap.ap_BreakBits = SIGBREAKF_CTRL_C;
				ap.ap_Flags = 0;
				ap.ap_FoundBreak = 0;
				ap.ap_Strlen = 0;
				for ( error = MatchFirst(pat,&ap) ; !error && !ap.ap_FoundBreak ; error = MatchNext(&ap) )
				{
					if ( ap.ap_Info.fib_DirEntryType>0 )
					{
						strcpy( buffer, path );
						add_path( buffer, ap.ap_Info.fib_FileName );
						if ( buffer[strlen(buffer)-1]!='/' && buffer[strlen(buffer)-1]!=':' )strncat( buffer, "/", BUFSIZE );
					}
					if ( ap.ap_Info.fib_DirEntryType<0 || ( access = real_dir(ftp,buffer,NULL) ) )
					{
						char date[16], time[16];
						struct DateTime dt = { ap.ap_Info.fib_Date, FORMAT_CDN, 0, NULL, date, time };
						long day, month, year;

						read		= ((ftp&&ftp->user&&access&&IS_SET(ftp->user->access,access->read))?!IS_SET(ap.ap_Info.fib_Protection,FIBF_READ)?"r":"-":"-");
						write		= ((ftp&&ftp->user&&access&&IS_SET(ftp->user->access,access->write))?!IS_SET(ap.ap_Info.fib_Protection,FIBF_WRITE)?"w":"-":"-");
						delete	= ((ftp&&ftp->user&&access&&IS_SET(ftp->user->access,access->delete))?!IS_SET(ap.ap_Info.fib_Protection,FIBF_DELETE)?"x":"-":"-");
						DateToStr( &dt );
						StrToLong( &dt.dat_StrDate[0], &day );
						StrToLong( &dt.dat_StrDate[3], &month );
						StrToLong( &dt.dat_StrDate[6], &year );
						month--;
						if ( month>=12 || month<0 )month = 0;
						if ( ds.ds_Days<dt.dat_Stamp.ds_Days || ds.ds_Days-dt.dat_Stamp.ds_Days>365 )
						{
							if ( year>=78 )year += 1900;
							else year += 2000;
							sprintf( time, " %ld", year );
						}
						if ( !arg_l )
						{
							FPrintf( file, "%s%s\r\n",
							 (ULONG)ap.ap_Info.fib_FileName,
							 (ULONG)((arg_F&&ap.ap_Info.fib_DirEntryType>0)?"/":"") );
						}
						else
						{
							FPrintf( file, "%s%s%s%s%s%s%s%s%s%s   1 %8.8s %8.8s %8.ld %s %2.ld %5.5s %s%s\r\n",
							 (ULONG)(ap.ap_Info.fib_DirEntryType>0?"d":"-"),
							 (ULONG)read, (ULONG)write, (ULONG)delete,
							 (ULONG)read, (ULONG)write, (ULONG)delete,
							 (ULONG)read, (ULONG)write, (ULONG)delete,
							 (ULONG)"none", (ULONG)"none",
							 (ULONG)((arg_F&&ap.ap_Info.fib_DirEntryType>0)?4096:ap.ap_Info.fib_Size),
							 (ULONG)months[month],
							 (ULONG)day,
							 (ULONG)time,
							 (ULONG)ap.ap_Info.fib_FileName,
							 (ULONG)((arg_F&&ap.ap_Info.fib_DirEntryType>0)?"/":"") );
						}
					}
				}
				MatchEnd( &ap );
				CurrentDir( oldlock );
				UnLock(lock);
			}
		}
		for ( access = access_list ; access ; access = access->next )
		{
			if ( !access->alias )continue;
			if ( !IS_SET((ftp->user?ftp->user->access:1),(access->read|access->write|access->delete)) )continue;
			if ( strnicmp( access->alias, path, pathlen ) )continue;
			if ( cdlen && !strnicmp( access->path, ftp->buffer, cdlen ) )continue;
			if ( !IS_SET((ftp->user?ftp->user->access:1),access->subdirs) )
			{
				int i = strlen(access->alias)-1;
				if ( !i || access->alias[i]!='/' || access->alias[i]!=':' )i++;
				if ( strchr(&path[i],'/') )continue;
			}
			buf = &access->alias[pathlen];
			if ( *buf=='/' || *buf==':' )buf++;
			for ( buf2 = buf ; *buf2 && *buf2!='/' && *buf2!=':' ; buf2++ );
			if ( !*buf || ( buf2[0] && buf2[1] ) )continue;
			strcpy( buffer, buf );
			if ( *ftp->buffer )
			{
				strcpy( checkbuffer, ftp->buffer );
				if ( AddPart( checkbuffer, buffer, BUFSIZE ) )
				{
					if ( ( lock = Lock( checkbuffer, ACCESS_READ ) ) )
					{
						UnLock(lock);
						continue;
					}
				}
			}
			if ( buffer[strlen(buffer)-1]=='/' )buffer[strlen(buffer)-1] = '\0';
			if ( !arg_l )
			{
				FPrintf( file, "%s%s\r\n",
				 (ULONG)buffer,
				 (ULONG)(arg_F?"/":"") );
			}
			else
			{
				read		= (IS_SET(ftp->user->access,access->read)?"r":"-");
				write		= (IS_SET(ftp->user->access,access->write)?"w":"-");
				delete	= (IS_SET(ftp->user->access,access->delete)?"x":"-");
				FPrintf( file, "d%s%s%s%s%s%s%s%s%s   1 %8.8s %8.8s %8.ld Jan  1  1978 %s%s\r\n",
				 (ULONG)read, (ULONG)write, (ULONG)delete,
				 (ULONG)read, (ULONG)write, (ULONG)delete,
				 (ULONG)read, (ULONG)write, (ULONG)delete,
				 (ULONG)"none", (ULONG)"none",
				 (ULONG)4096,
				 (ULONG)buffer,
				 (ULONG)(arg_F?"/":"") );
			}
		}
		Close( file );
		SET_BIT(ftp->flags,FLAG_LIST|FLAG_SEND);
		do_file( ftp, MODE_READ );
	}
	return;
}

BOOL cmd_list(struct ftpdata *ftp)
{
	list_dir( ftp, LIST_LONG );
	return FALSE;
}

BOOL cmd_nlst(struct ftpdata *ftp)
{
	list_dir( ftp, 0 );
	return FALSE;
}

BOOL cmd_port(struct ftpdata *ftp)
{
	char *buf;
	unsigned long ip = 0, m;
	long port = 0, i = 0;
	struct sockaddr_in dport;
	struct sockaddr_in eport;
	long nonblocking = 1;

	eport.sin_family = AF_INET;
	for( buf = ftp->args ; i < 4 ; i++ )
	{
		buf += StrToLong(buf,&m);
		ip = m + (ip<<8);
		if ( *buf++ != ',' )
		{
			output( ftp, REPLY_501, 501 );
			return FALSE;
		}
	}
	eport.sin_addr.s_addr = ip;
/*
	10.*
	127.*
	172.16.*
	192.168.*
*/
	if ( !IS_SET(ftp->flags,FLAG_SPEEDY)
	 &&( (ip>>24)==10
	 ||  (ip>>24)==127
	 ||  (ip>>16)==((172<<8) + 16)
	 ||  (ip>>16)==((192<<8) + 168) ) )
	{
//		i = sizeof(struct sockaddr);
//		if ( getpeername( ftp->control, (struct sockaddr*)&eport, &i ) == 0 )eport.sin_addr.s_addr = ip;
		output( ftp, "You are on a LAN, and the IP-NAT is not translating your IP address correctly.", 501 );
	}
	buf += StrToLong(buf,&port);
	if ( *buf++ != ',' )
	{
		output( ftp, REPLY_501, 501 );
		return FALSE;
	}
	StrToLong(buf,&m);
	port = m + (port<<8);
	eport.sin_port = port;
	i = 1;
	memclr( &dport, sizeof(struct sockaddr_in) );
	dport.sin_family	= AF_INET;
	dport.sin_addr.s_addr= INADDR_ANY;
	dport.sin_port		= port-1;
	eport.sin_family	= AF_INET;
	if ( ( ftp->data = socket(AF_INET,SOCK_STREAM,0) ) == -1
	  || setsockopt( ftp->data, SOL_SOCKET, SO_REUSEADDR, (char*)&i, sizeof(i)) < 0
	  || bind( ftp->data, (struct sockaddr*)&dport, sizeof(struct sockaddr_in) )
	  || IoctlSocket( ftp->data, FIONBIO, (char*)&nonblocking ) == -1
	  || connect( ftp->data, (struct sockaddr *)&eport, sizeof(struct sockaddr_in) ) )
	{
		if ( Errno() != EINPROGRESS )
		{
			free_data(ftp);
			output( ftp, REPLY_501, 425 );
			return FALSE;
		}
	}
	SET_BIT(ftp->flags,FLAG_CONNECTED);
	output( ftp, REPLY_200, 200 );
	update_list(ftp);
	return TRUE;
}

BOOL cmd_pwd(struct ftpdata *ftp)
{
	sprintf( ftp->buffer, "\"%s\" is current directory", ftp->path );
	output( ftp, ftp->buffer, 257 );
	return FALSE;
}

BOOL cmd_retr(struct ftpdata *ftp)
{
	struct accessdata	*access;
	struct FileInfoBlock fib __attribute__ ((__aligned__(4)));
	char path[BUFSIZE];
	BPTR	lock = 0;

	strcpy( ftp->buffer, ftp->path );
	add_path( ftp->buffer, ftp->args );
	if ( !( access = real_dir(ftp,ftp->buffer,path) )
	  || !IS_SET(ftp->user->access,access->read)
	  || !( lock = Lock( path, ACCESS_READ ) ) )
	{
		output( ftp, REPLY_550, 550 );
		return FALSE;
	}
	if ( !Examine( lock, &fib ) || fib.fib_DirEntryType >= 0 )
	{
		output( ftp, REPLY_553, 553 );
		UnLock(lock);
		return FALSE;
	}
	UnLock(lock);
	strcpy( ftp->filename, ftp->args );
	SET_BIT(ftp->flags,FLAG_SEND);
	do_file( ftp, MODE_READ );
	return TRUE;
}

BOOL cmd_stor(struct ftpdata *ftp)
{
	struct accessdata	*access;
	char	path[BUFSIZE];
	BPTR	lock = 0;

	strcpy( ftp->buffer, ftp->path );
	add_path( ftp->buffer, ftp->args );
	if ( !(access = real_dir(ftp,ftp->buffer,path)) )return FALSE;
	if ( !IS_SET(ftp->user->access,access->write)
	|| ( !IS_SET(ftp->user->access,access->delete) && ( lock = Lock( path, ACCESS_READ ) ) ) )
	{
		UnLock(lock);
		output( ftp, REPLY_553, 553 );
		return FALSE;
	}
	strcpy( ftp->filename, ftp->args );
	SET_BIT(ftp->flags,FLAG_RECV);
	do_file( ftp, ftp->start?MODE_APPEND:MODE_WRITE );
	return TRUE;
}

BOOL cmd_rest(struct ftpdata *ftp)
{
	if ( ftp->args[0]<'0' || ftp->args[0]>'9' )
	{
		output( ftp, REPLY_501, 501 );
		return FALSE;
	}
	StrToLong(ftp->args,&ftp->start);
	output( ftp, REPLY_350, 350 );
	return TRUE;
}

char	lu4comma[]	= "%lu,%lu,%lu,%lu";
char	reply_227[]= "Entering Passive Mode (%s,%ld,%ld)";

BOOL cmd_pasv(struct ftpdata *ftp)
{
	struct sockaddr_in port;

	free_data(ftp);
	if ( !nopasv && ( ftp->pdata = socket(AF_INET, SOCK_STREAM, 0) ) >= 0 )
	{
		long len = sizeof(struct sockaddr_in), nonblocking = 1;

		IoctlSocket( ftp->pdata, FIONBIO, (char*)&nonblocking );
		memclr(&port,sizeof(struct sockaddr_in));
		if ( getsockname( ftp->control, (struct sockaddr*)&port, &len ) == 0 )
		{
			len = port.sin_addr.s_addr;
			memclr(&port,sizeof(struct sockaddr_in));
			port.sin_addr.s_addr = len;
			port.sin_family = AF_INET;
			if ( bind(ftp->pdata, (struct sockaddr*)&port, sizeof(struct sockaddr_in) ) == 0 )
			{
				len = sizeof(struct sockaddr_in);
				if ( getsockname( ftp->pdata, (struct sockaddr*)&port, &len ) == 0 )
				{
					if ( listen(ftp->pdata, 1) == 0 )
					{
						char *addr, *portno, buffer[32], ip[32];

						portno = (char*)&port.sin_port;
						addr = (char*)&port.sin_addr.s_addr;
						sprintf( ip, lu4comma, (addr[0]&0xff), (addr[1]&0xff), (addr[2]&0xff), (addr[3]&0xff) );
						if ( !IS_SET(ftp->flags,FLAG_SPEEDY) && GetVar("IP",buffer,32,0)>=0 )
						{
							char *buf;
							long i;

							for ( i = 0, buf = buffer ; *buf ; buf++ )
							{
								if ( *buf=='.' || *buf==',' )
								{
								   i++;
								   *buf = ',';
								}
								else if ( *buf<'0' || *buf>'9' )break;
							}
							if ( !*buf && i==3 )sprintf( ip, "%s", buffer );
						}
						sprintf( ftp->buffer, reply_227, ip, (portno[0]&0xff), (portno[1]&0xff) );
						output( ftp, ftp->buffer, 227 );
						return TRUE;
					}
				}
			}
		}
		CloseSocket(ftp->pdata);
		ftp->pdata = -1;
	}
	output( ftp, REPLY_425, 425 );
	return FALSE;
}

BOOL cmd_dele(struct ftpdata *ftp)
{
	struct accessdata	*access;
	struct FileInfoBlock fib __attribute__ ((__aligned__(4)));
	BPTR	lock;
	char	path[BUFSIZE];

	strcpy( ftp->buffer, ftp->path );
	add_path( ftp->buffer, ftp->args );
	if ( !( access = real_dir(ftp,ftp->buffer,path) ) )return FALSE;
	if ( !IS_SET(ftp->user->access,access->delete) )
	{
		output( ftp, REPLY_550, 550 );
		return FALSE;
	}
	if ( !( lock = Lock( path, ACCESS_READ ) ) )
	{
		output( ftp, REPLY_550, 550 );
		return FALSE;
	}
	if ( !Examine( lock, &fib ) || fib.fib_DirEntryType >= 0 )
	{
		output( ftp, REPLY_550, 550 );
		UnLock(lock);
		return FALSE;
	}
	UnLock(lock);
	if ( DeleteFile(path) )output( ftp, REPLY_250, 250 );
	else output( ftp, REPLY_550, 550 );
	return TRUE;
}

BOOL cmd_mkd(struct ftpdata *ftp)
{
	struct accessdata	*access;
	BPTR	lock;
	char	path[BUFSIZE];

	strcpy( ftp->buffer, ftp->path );
	add_path( ftp->buffer, ftp->args );
	if ( !( access = real_dir(ftp,ftp->buffer,path) ) )return FALSE;
	if ( !IS_SET(ftp->user->access,access->write) )
	{
		output( ftp, REPLY_550, 550 );
		return FALSE;
	}
	if ( !( lock = CreateDir(path) ) )
	{
		output( ftp, REPLY_553, 553 );
		return FALSE;
	}
	UnLock(lock);
	output( ftp, REPLY_200, 200 );
	return TRUE;
}

BOOL cmd_rmd(struct ftpdata *ftp)
{
	struct accessdata	*access;
	struct FileInfoBlock fib __attribute__ ((__aligned__(4)));
	BPTR	lock;
	char	path[BUFSIZE];

	strcpy( ftp->buffer, ftp->path );
	add_path( ftp->buffer, ftp->args );
	if ( !( access = real_dir(ftp,ftp->buffer,path) ) )return FALSE;
	if ( !IS_SET(ftp->user->access,access->delete) )
	{
		output( ftp, REPLY_550, 550 );
		return FALSE;
	}
	if ( !( lock = Lock( path, ACCESS_READ ) ) )
	{
		output( ftp, REPLY_553, 553 );
		return FALSE;
	}
	if ( !Examine( lock, &fib ) || fib.fib_DirEntryType <= 0 )
	{
		output( ftp, REPLY_550, 550 );
		UnLock(lock);
		return FALSE;
	}
	UnLock(lock);
	if ( DeleteFile(path) )output( ftp, REPLY_250, 250 );
	else output( ftp, REPLY_550, 550 );
	return TRUE;
}

char	mdtmok[]			= "%04.ld%02.ld%02.ld%02.ld%02.ld%02.ld";

BOOL cmd_mdtm(struct ftpdata *ftp)
{
	struct accessdata	*access;
	struct FileInfoBlock fib __attribute__ ((__aligned__(4)));
	BPTR	lock;
	char	path[BUFSIZE], date[16], time[16];
	struct DateTime dt = { { fib.fib_Date.ds_Days, fib.fib_Date.ds_Minute, fib.fib_Date.ds_Tick }, FORMAT_CDN, 0, NULL, date, time };
	long year, month, day, hours, mins, secs;

	strcpy( ftp->buffer, ftp->path );
	add_path( ftp->buffer, ftp->args );
	if ( !( access = real_dir(ftp,ftp->buffer,path) ) )return FALSE;
	if ( !( lock = Lock( path, ACCESS_READ ) ) )
	{
		output( ftp, REPLY_550, 550 );
		return FALSE;
	}
	if ( !Examine( lock, &fib ) )
	{
		output( ftp, REPLY_550, 550 );
		UnLock(lock);
		return FALSE;
	}
	DateToStr( &dt );
	StrToLong( &dt.dat_StrDate[0], &day );
	StrToLong( &dt.dat_StrDate[3], &month );
	StrToLong( &dt.dat_StrDate[6], &year );
	if ( year>=78 )year += 1900;	else year += 2000;
	StrToLong( &dt.dat_StrTime[0], &hours );
	StrToLong( &dt.dat_StrTime[3], &mins );
	StrToLong( &dt.dat_StrTime[6], &secs );
	sprintf( ftp->buffer, mdtmok, year, month, day, hours, mins, secs );
	output( ftp, ftp->buffer, 213 );
	UnLock(lock);
	return TRUE;
}

BOOL cmd_size(struct ftpdata *ftp)
{
	struct accessdata	*access;
	struct FileInfoBlock fib __attribute__ ((__aligned__(4)));
	BPTR	lock;
	char	path[BUFSIZE];

	strcpy( ftp->buffer, ftp->path );
	add_path( ftp->buffer, ftp->args );
	if ( !( access = real_dir(ftp,ftp->buffer,path) ) )return FALSE;
	if ( !( lock = Lock( path, ACCESS_READ ) ) )
	{
		output( ftp, REPLY_550, 550 );
		return FALSE;
	}
	if ( !Examine( lock, &fib ) )
	{
		output( ftp, REPLY_550, 550 );
		UnLock(lock);
		return FALSE;
	}
	sprintf( ftp->buffer, "%lu", fib.fib_Size );
	output( ftp, ftp->buffer, 213 );
	UnLock(lock);
	return TRUE;
}

BOOL cmd_appe(struct ftpdata *ftp)
{
	struct accessdata	*access;
	char	path[BUFSIZE];

	strcpy( ftp->buffer, ftp->path );
	add_path( ftp->buffer, ftp->args );
	if ( !(access = real_dir(ftp,ftp->buffer,path)) )return FALSE;
	if ( !IS_SET(ftp->user->access,access->delete)
	  || !IS_SET(ftp->user->access,access->write) )
	{
		output( ftp, REPLY_550, 550 );
		return FALSE;
	}
	strcpy( ftp->filename, ftp->args );
	SET_BIT(ftp->flags,FLAG_RECV);
	do_file( ftp, MODE_APPEND );
	return TRUE;
}

BOOL cmd_help(struct ftpdata *ftp)
{
	int i;

	if ( *ftp->args )
	{
		for ( i = 0 ; commands[i].name ; i++ )
		{
			if ( strnicmp( commands[i].name, ftp->args, strlen(commands[i].name) ) )continue;
			sprintf( ftp->buffer, "Usage: %s %s", commands[i].name, commands[i].help );
			output( ftp, ftp->buffer, 214 );
			return FALSE;
		}
	}
	output( ftp, "The following commands are recognized.", 000 );
	ftp->buffer[0] = '\0';
	for ( i = 0 ; commands[i].name ; )
	{
		strncat( ftp->buffer, "    ", BUFSIZE );
		strncat( ftp->buffer, commands[i].name, BUFSIZE );
		if ( !commands[i].name[3] )strncat( ftp->buffer, " ", BUFSIZE );
		if ( ++i % 5 == 0 )strncat( ftp->buffer, "\n", BUFSIZE );
	}
//	if ( i % 5 != 0 )strncat( ftp->buffer, "\n", BUFSIZE );
	output( ftp, ftp->buffer, 0 );
	output( ftp, "Use HELP <cmd> for more help.", 214 );
	return FALSE;
}

BOOL cmd_rnfr(struct ftpdata *ftp)
{
	struct accessdata	*access;
	char	path[BUFSIZE];

	strcpy( ftp->buffer, ftp->path );
	add_path( ftp->buffer, ftp->args );
	if ( !( access = real_dir(ftp,ftp->buffer,path) ) )return FALSE;
	if ( !IS_SET(ftp->user->access,access->delete) )
	{
		output( ftp, REPLY_550, 550 );
		return FALSE;
	}
	strncpy( ftp->filename, ftp->args, 512 );
	output( ftp, REPLY_350, 350 );
	return TRUE;
}

BOOL cmd_rnto(struct ftpdata *ftp)
{
	struct accessdata	*access;
	char	to[BUFSIZE], from[BUFSIZE];

	strcpy( ftp->buffer, ftp->path );
	add_path( ftp->buffer, ftp->filename );
	if ( !( access = real_dir(ftp,ftp->buffer,from) ) )return FALSE;
	if ( !IS_SET(ftp->user->access,access->delete) )
	{
		output( ftp, REPLY_550, 550 );
		return FALSE;
	}
	strcpy( ftp->buffer, ftp->path );
	AddPart( ftp->buffer, ftp->args, BUFSIZE );
	if ( !( access = real_dir(ftp,ftp->buffer,to) ) )return FALSE;
	if ( !IS_SET(ftp->user->access,access->delete) )
	{
		output( ftp, REPLY_550, 550 );
		return FALSE;
	}
	if ( !Rename( from, to ) )
	{
		output( ftp, REPLY_550, 550 );
		return FALSE;
	}
	output( ftp, REPLY_250, 250 );
	return TRUE;
}

BOOL cmd_rein(struct ftpdata *ftp)
{
	ftp->user = NULL;
	ftp->path[0] = '\0';
	ftp->cd[0] = '\0';
	ftp->filename[0] = '\0';
	ftp->flags = 0;
	output( ftp, REPLY_220, 220 );
	return FALSE;
}

BOOL cmd_stat(struct ftpdata *ftp)
{
	if ( *ftp->args )
	{
		output( ftp, REPLY_502, 502 );
		return FALSE;
	}
	sprintf( ftp->buffer, "FTP server status:\n"
		" Connected to %s\n"
		" Logged in as %s\n"
		" TYPE: %s\n"
		" No data connection",
		ftp->ip,
		ftp->user ? ftp->user->user : "(not logged in)",
		IS_SET(ftp->flags, FLAG_BINARY) ? "BINARY" : "ASCII"
	);
	output( ftp, ftp->buffer, 211 );
	return FALSE;
}

BOOL cmd_site(struct ftpdata *ftp)
{
	output( ftp, "SITE commands not supported.", 502 );
	return FALSE;
}
