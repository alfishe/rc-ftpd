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
#include	<sys/socket.h>

#include	<clib/alib_protos.h>
#include	<inline/exec.h>
#include	<inline/socket.h>
#include	<inline/dos.h>

#include "ftpd.h"

BOOL cmd_feat(struct ftpdata *ftp)	// slight copy of output() ;-)
{
	unsigned long i;
	char *txt[] =
	{
		" MDTM\n",
		" SIZE\n",
		" REST STREAM\n",
		" TVFS\n",
		NULL
	};

	output( ftp, "211-Extensions supported:\n", -1 );
	for ( i = 0 ; txt[i] ; i++ )output( ftp, txt[i], -1 );
	output( ftp, "211 end\n", -1 );
	return FALSE;
}

void quick_parent( char *path )
{
	int i = strlen(path) - 1;

	while( --i >= 0 )if ( to[i] == '/' )break;
	to[i++] = '/';
	to[i] = '\0';
}

// *to must be at least 512 bytes
void add_path( char *to, char *add )
{
	if ( !add || !to )return;
	if ( *add == '/' )*to = '\0';
	while ( *add )
	{
		if ( *add == '.' )
		{
			int i = 1;
			while( add[i] == '.' )i++;
			if ( add[i] == '/' )
			{
				for( add++ ; *add == '.' ; add++ )quick_parent( to );
				continue;
			}
		}
		if ( *add == '/' )quick_parent( to );
		else
		{
			int i = strlen(to)-1;
			while ( *add )if ( ( to[i++] = *add++ ) == '/' )break;
			to[i] = '\0';
		}
	}
}

struct FtpdPath *real_dir( struct FtpdConnection *ftp, char *relative, char *realpath )
{
	struct FtpdPath	*path, *best = NULL;
	char					*buf, buffer[512];

	if ( realpath )realpath[0] = '\0';
	strcpy( buffer, fc_Path );
	add_path( buffer, relative );
	for ( path = (struct FtpdPath*)(ftpb->fb_Paths.mlh_Head) ; path ; path = (struct FtpdPath*)(path->fp_Node.mln_Succ) )
	{
		if ( !path->fp_Path || !path->fp_Virtual )continue;
		if ( !strnicmp( path->fp_Virtual, buffer, strlen(path->fp_Virtual) )
		  && ( !best || strlen(best->fb_Virtual)<strlen(path->fb_Virtual) ) )
		{
			if ( !stricmp(ftp->fc_User,path->fp_Owner) || !stricmp(ftp->fc_Group,path->fp_Group) || path->fp_GuestAccess )
				best = path;
		}
	}
	if ( !best )return(NULL);
	if ( realpath )strcpy( realpath, buffer );
	return(access);
}

void do_file( struct ftpdata *ftp, long filemode )
{
	if ( ftp->data==-1 && ftp->pdata==-1 && ( ftp->port.sin_addr.s_addr==0 || ftp->port.sin_port==0 ) )
	{
		free_data(ftp);
		output( ftp, REPLY_503, 503 );
		return;
	}
	if ( !ftp->file )
	{
		char path[BUFSIZE];

		strcpy( ftp->buffer, ftp->path );
		add_path( ftp->buffer, ftp->filename );
		if ( !( real_dir(ftp,ftp->buffer,path) ) || !( ftp->file = Open( path, filemode ) ) )
		{
			output( ftp, REPLY_550, 550 );
			return;
		}
		if ( ftp->start )Seek( ftp->file, ftp->start, OFFSET_BEGINNING );
		ftp->current = ftp->start;
	}
	Flush( ftp->file );
	if ( filemode == MODE_OLDFILE )
	{
		struct FileInfoBlock fib __attribute__ ((__aligned__(4)));
		if ( ExamineFH( ftp->file, &fib ) )ftp->size = fib.fib_Size;
	}
	ftp->tries = 0;
	if ( ftp->data>=0 )output( ftp, REPLY_125, 125 );
	else output( ftp, REPLY_150, 150 );
	ftp->started.ds_Tick = ftp->started.ds_Minute =	ftp->started.ds_Days = 0;
	update_list( ftp );
	return;
}

BOOL cmd_abor(struct ftpdata *ftp)
{
	free_data(ftp);
	output( ftp, REPLY_226, 226 );
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
//	struct Library	*IdentifyBase;

//	if ( ( IdentifyBase = OpenLibrary( "identify.library", 11 ) ) )
//	{
//		IdFormatString( "$SYSTEM$/$CPU$@$CPUCLOCK$", ftp->buffer, BUFSIZE, NULL );
//		CloseLibrary( IdentifyBase );
//	}
//	else strcpy( ftp->buffer, "AmigaOS Type: L8" );
//	strncat( ftp->buffer, " system type.\n", BUFSIZE );
//	output( ftp, ftp->buffer, 215 );
	output( ftp, "AmigaOS Type: L8\n", 215 );
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

	for ( user = user_list ; user ; user = user->next )if ( *user->user && !stricmp(user->user,ftp->args) )break;
	if ( !( ftp->user = user ) )ftp->user = (void*)-1;
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
		if ( *ftp->user->pass )
		{
			if ( strcmp(ftp->user->pass,ftp->args) )
			{
				output( ftp, REPLY_530, 530 );
				ftp->user = NULL;
				return FALSE;
			}
			if ( i )for ( prev = ftp_list ; prev ; prev = prev->next )
			{
				if ( prev->user == ftp->user && !i-- )
				{
					output( ftp, REPLY_530, 530 );
					ftp->user = NULL;
					return FALSE;
				}
			}
		}
		if ( welcome )output( ftp, welcome, 0 );
		output( ftp, REPLY_230, 230 );
		if ( !ftp->loglevel && ( ftp->loglevel = ftp->user->loglevel ) )start_log( ftp );
		SET_BIT(ftp->flags,FLAG_ONLINE);
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
		if ( path[strlen(path)-1]!='/' && path[strlen(path)-1]!=':' )strncat( path, "/", BUFSIZE );
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
	char buffer[BUFSIZE], path[BUFSIZE], *buf;

	*buffer = '\0';
	strncpy( path, ftp->path, 512 );
	if ( strcmp(path,"/") && strcmp(path,":") )
	{
		buf = &path[strlen(path)-1];
		if ( *buf=='/' || *buf==':' )buf--;
		while ( buf>path && *buf != '/' && *buf != ':' )buf--;
		if ( *buf=='/' || *buf==':' )buf++;
		*buf = '\0';
	}
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

BOOL cmd_clnt(struct ftpdata *ftp)
{
	SET_BIT(ftp->flags,FLAG_CLNT);
	output( ftp, "Command okay. Using EPLF reply for LIST.\n", 200 );
	return TRUE;
}

char	*months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

#define	LIST_LONG	(1<<0)
#define	LIST_EPLF	(1<<1)
#define	LIST_MLST	(1<<2)
#define	LIST_MLSD	(1<<3)

void list_dir(struct ftpdata *ftp, int flag )
{
	struct accessdata *access;
	struct ExAllControl	*eac;
	char path[BUFSIZE], buffer[BUFSIZE], *buf, *buf2, *read, *write, *delete;
	BPTR	lock;
	BOOL	arg_F = FALSE, arg_l = IS_SET(flag,LIST_LONG)?TRUE:FALSE;

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
			if ( !( access = real_dir(ftp,path,ftp->buffer) ) )return;
		}
		else strcpy( ftp->buffer, ftp->cd );
	}
	sprintf( ftp->filename, lsfile, (ULONG)ftp );
	if ( *ftp->buffer )
	{
		if ( !( lock = Lock( ftp->buffer, ACCESS_READ ) ) )output( ftp, REPLY_550, 550 );
		else
		{
			if ( !( eac = AllocDosObject(DOS_EXALLCONTROL,NULL) ) )output( ftp, REPLY_550, 550 );
			else if ( !( ftp->file = Open( ftp->filename, MODE_NEWFILE ) ) )output( ftp, REPLY_550, 550 );
			else
			{
				struct ExAllData *ead;
				char	*EAData[BUFSIZE] __attribute__ ((__aligned__(4)));
				char	pattern[130];
				BOOL more = TRUE;

				ParsePatternNoCase( *list?list:"#?", pattern, 128 );
				eac->eac_LastKey = 0;
				eac->eac_MatchString = pattern;
				while ( more )
				{
					more = ExAll( lock, (void*)EAData, sizeof(EAData), ED_DATE, eac );
					if ( !more && IoErr() != ERROR_NO_MORE_ENTRIES )break;
					if ( eac->eac_Entries==0 )continue;
					for ( ead = (struct ExAllData*)EAData ; ead ; ead = ead->ed_Next )
					{
						if ( ead->ed_Type>0 )
						{
							strcpy( buffer, path );
							add_path( buffer, ead->ed_Name );
							if ( buffer[strlen(buffer)-1]!='/' && buffer[strlen(buffer)-1]!=':' )strncat( buffer, "/", BUFSIZE );
						}
						if ( ead->ed_Type<0 || ( access = real_dir(ftp,buffer,NULL) ) )
						{
							if ( IS_SET(flag,LIST_EPLF) )
							{
								unsigned long long seconds = 252460800;	// seconds from 01/01/1970 00:00.00
								char	secs[32], *oops = "%lu%09.lu";

								seconds += ead->ed_Days * ( 24 * 60 * 60 );
								seconds += ead->ed_Mins * 60;
								seconds += ead->ed_Ticks / 50;
								if ( seconds < 1000000000 )sprintf( secs, "%lu", (ULONG)seconds );
								else sprintf( secs, oops, seconds/1000000000, seconds );
								if ( ead->ed_Type>0 )
								{
									FPrintf( ftp->file, "+m%s,/,\t%s\n",
									 (ULONG)secs,
									 (ULONG)ead->ed_Name );
								}
								else
								{
									FPrintf( ftp->file, "+m%s,r,s%lu,\t%s\n",
									 (ULONG)secs,
									 (ULONG)ead->ed_Size,
									 (ULONG)ead->ed_Name );
								}
							}
							else
							{
								char date[16], time[16];
								struct DateTime dt = { { ead->ed_Days, ead->ed_Mins, ead->ed_Ticks }, FORMAT_CDN, 0, NULL, date, time };
								long day, month, year;

								read		= (IS_SET(ftp->user->access,access->read)?!IS_SET(ead->ed_Prot,FIBF_READ)?"r":"-":"-");
								write		= (IS_SET(ftp->user->access,access->write)?!IS_SET(ead->ed_Prot,FIBF_WRITE)?"w":"-":"-");
								delete	= (IS_SET(ftp->user->access,access->delete)?!IS_SET(ead->ed_Prot,FIBF_DELETE)?"d":"-":"-");
								DateToStr( &dt );
								StrToLong( &dt.dat_StrDate[0], &day );
								StrToLong( &dt.dat_StrDate[3], &month );
								StrToLong( &dt.dat_StrDate[6], &year );
								month--;
								if ( month>=12 || month<0 )month = 0;
								if ( year>=78 )sprintf( time, "19%ld", year );
								if ( !arg_l )
								{
									FPrintf( ftp->file, "%s%s\n",
									 (ULONG)ead->ed_Name,
									 (ULONG)((arg_F&&ead->ed_Type>0)?"/":"") );
								}
								else
								{
									FPrintf( ftp->file, "%s%s%s%s%s%s%s%s%s%s  1 1 %8.ld %s %02.ld %-5.5s %s%s\n",
									 (ULONG)(ead->ed_Type>0?"d":"-"),
									 (ULONG)read, (ULONG)write, (ULONG)delete,
									 (ULONG)read, (ULONG)write, (ULONG)delete,
									 (ULONG)read, (ULONG)write, (ULONG)delete,
									 (ULONG)ead->ed_Size,
									 (ULONG)months[month],
									 (ULONG)day,
									 (ULONG)time,
									 (ULONG)ead->ed_Name,
									 (ULONG)((arg_F&&ead->ed_Type>0)?"/":"") );
								}
							}
						}
					}
				}
			}
			FreeDosObject( DOS_EXALLCONTROL, eac );
		}
		UnLock(lock);
	}
	else if ( !( ftp->file = Open( ftp->filename, MODE_NEWFILE ) ) )output( ftp, REPLY_550, 550 );
	if ( ftp->file )
	{
		char	checkbuffer[BUFSIZE];
		int	pathlen = strlen(path), cdlen = strlen(ftp->buffer);

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
			if ( IS_SET(flag,LIST_EPLF) )
			{
				FPrintf( ftp->file, "+m946684800,/,\t%s\n",
				 (ULONG)buffer );
			}
			else if ( !arg_l )
			{
				FPrintf( ftp->file, "%s%s\n",
				 (ULONG)buffer,
				 (ULONG)(arg_F?"/":"") );
			}
			else
			{
				read		= (IS_SET(ftp->user->access,access->read)?"r":"-");
				write		= (IS_SET(ftp->user->access,access->write)?"w":"-");
				delete	= (IS_SET(ftp->user->access,access->delete)?"d":"-");
				FPrintf( ftp->file, "d%s%s%s%s%s%s%s%s%s  1 1        0 Jan 01 1978  %s%s\n",
				 (ULONG)read, (ULONG)write, (ULONG)delete,
				 (ULONG)read, (ULONG)write, (ULONG)delete,
				 (ULONG)read, (ULONG)write, (ULONG)delete,
				 (ULONG)buffer,
				 (ULONG)(arg_F?"/":"") );
			}
		}
	}
	if ( ftp->file )
	{
		SET_BIT(ftp->flags,FLAG_LIST|FLAG_SEND);
		Seek( ftp->file, 0, OFFSET_BEGINNING );
		do_file( ftp, MODE_OLDFILE );
	}
	return;
}

BOOL cmd_list(struct ftpdata *ftp)
{
	if ( IS_SET(ftp->flags,FLAG_CLNT) )list_dir( ftp, LIST_EPLF );
	else list_dir( ftp, LIST_LONG );
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
	buf += StrToLong(buf,&port);
	if ( *buf++ != ',' )
	{
		output( ftp, REPLY_501, 501 );
		return FALSE;
	}
	StrToLong(buf,&m);
	port = m + (port<<8);
	ftp->port.sin_family			= AF_INET;
	ftp->port.sin_addr.s_addr	= ip;
	ftp->port.sin_port			= port;
	output( ftp, REPLY_200, 200 );
	return TRUE;
}

BOOL cmd_pwd(struct ftpdata *ftp)
{
	sprintf( ftp->buffer, "\"%s\" is current directory\r\n", ftp->path );
	output( ftp, ftp->buffer, 257 );
	return FALSE;
}

BOOL cmd_retr(struct ftpdata *ftp)
{
	struct accessdata	*access;
	struct FileInfoBlock fib __attribute__ ((__aligned__(4)));
	char path[BUFSIZE];
	BPTR	lock;

	strcpy( ftp->buffer, ftp->path );
	add_path( ftp->buffer, ftp->args );
	if ( !( access = real_dir(ftp,ftp->buffer,path) ) )return FALSE;
	if ( !( lock = Lock( path, ACCESS_READ ) ) )
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
	do_file( ftp, MODE_OLDFILE );
	return TRUE;
}

BOOL cmd_stor(struct ftpdata *ftp)
{
	struct accessdata	*access;
	char	path[BUFSIZE];
	BPTR	lock;

	strcpy( ftp->buffer, ftp->path );
	add_path( ftp->buffer, ftp->args );
	if ( !( access = real_dir(ftp,ftp->buffer,path) ) )return FALSE;
	if ( !IS_SET(ftp->user->access,access->delete) && ( lock = Lock( path, ACCESS_READ ) ) )
	{
		UnLock(lock);
		output( ftp, REPLY_553, 553 );
		return FALSE;
	}
	strcpy( ftp->filename, ftp->args );
	SET_BIT(ftp->flags,FLAG_RECV);
	do_file( ftp, ftp->start?MODE_READWRITE:MODE_NEWFILE );
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

char	reply_227[]	= "Entering Passive Mode (%lu,%lu,%lu,%lu,%ld,%ld)\n";

BOOL cmd_pasv(struct ftpdata *ftp)
{
	free_data(ftp);
	if ( ( ftp->pdata = socket(AF_INET, SOCK_STREAM, 0) ) >= 0 )
	{
		long len = sizeof(struct sockaddr_in);

		memclr(&ftp->port,sizeof(struct sockaddr_in));
		if ( getsockname( ftp->control, (struct sockaddr*)&ftp->port, &len ) == 0 )
		{
			len = ftp->port.sin_addr.s_addr;
			memclr(&ftp->port,sizeof(struct sockaddr_in));
			ftp->port.sin_addr.s_addr = len;
			ftp->port.sin_family = AF_INET;
			if ( bind(ftp->pdata, (struct sockaddr*)&ftp->port, sizeof(struct sockaddr_in) ) == 0 )
			{
				len = sizeof(struct sockaddr_in);
				if ( getsockname( ftp->pdata, (struct sockaddr*)&ftp->port, &len ) == 0 )
				{
					if ( listen(ftp->pdata, 1) == 0 )
					{
						char *addr, *port;

						port = (char*)&ftp->port.sin_port;
						addr = (char*)&ftp->port.sin_addr.s_addr;
						sprintf( ftp->buffer, reply_227, (addr[0]&0xff), (addr[1]&0xff), (addr[2]&0xff), (addr[3]&0xff), (port[0]&0xff), (port[1]&0xff) );
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

char	mdtmok[]			= "%04.ld%02.ld%02.ld%02.ld%02.ld%02.ld\r\n";

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
	if ( !( access = real_dir(ftp,ftp->buffer,path) ) )return FALSE;
	if ( !IS_SET(ftp->user->access,access->delete)
	  || !IS_SET(ftp->user->access,access->write)
	  || !( ftp->file = Open( path, MODE_READWRITE ) ) )
	{
		if ( ftp->file )Close(ftp->file);
		ftp->file = 0;
		output( ftp, REPLY_550, 550 );
		return FALSE;
	}
	Seek( ftp->file, 0, OFFSET_END );
	strcpy( ftp->filename, ftp->args );
	SET_BIT(ftp->flags,FLAG_RECV);
	do_file( ftp, MODE_READWRITE );
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
		if ( ++i % 5 == 0 )strncat( ftp->buffer, "\r\n", BUFSIZE );
	}
	if ( i % 5 != 0 )strncat( ftp->buffer, "\r\n", BUFSIZE );
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
