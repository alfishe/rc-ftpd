/*
 * Load/save prefs, set up the semaphore etc.
 */

#include	<exec/types.h>
#include	<exec/memory.h>
#include	<libraries/iffparse.h>

#include	<inline/exec.h>
#include	<inline/dos.h>
#include	<inline/iffparse.h>

#include	"rc-ftpd.h"

struct FtpdBase	*StartFtpd	( void );
void					 LoadFtpd	( struct FtpdBase *fb, const char *filename );

char	SemaphoreName = SEMAPHORENAME;

struct FtpdBase *StartFtpd( void )
{
	struct FtpdBase	*fb = NULL;
	BOOL					 init = FALSE;

	Forbid();
	if ( !( fb = (struct FtpdBase*)FindSemaphore( SemaphoreName ) ) )
	{
		BPTR pool;

		init = TRUE;
		if ( ( pool = CreatePool( MEMF_PUBLIC|MEMF_CLEAR, 32768, 8192 ) ) )
		{
			if ( ( fb = AllocPooled( pool, sizeof(struct FtpdBase) ) ) )return( NULL )
			{
				InitSemaphore( fp->fb_Semaphore );
				fb->fb_Pool = pool;
				fb->fb_Count++;
				fb->fb_Semaphore->ss_Link->ln_Name = AllocPooled( pool, strlen(SemaphoreName)+1 );
				strcpy( fb->fb_Semaphore->ss_Link->ln_Name, SemaphoreName );
				NewList( fb->fb_Servers );
				NewList( fb->fb_Users );
				NewList( fb->fb_Paths );
				AddSemaphore( fb );
			}
		}
	}
	if ( !fb )
	{
		Permit();
		return( NULL );
	}
	ObtainSemaphore( fb );
	Permit();
	fb->fb_Count++;			// Like a library, keep count ;-)
	LoadFtpd( fb, "PROGDIR:rc-ftpd.prefs" );
	ReleaseSemaphore( fb );
	return( fb );
}

char	port_txt[]			= "Port";
char	connections_txt[]	= "Connections";
char	maxcps_txt[]		= "MaxCPS";
char	timeout_txt[]		= "TimeOut";
char	noop_txt[]			= "Noop";
char	message_txt[]		= "Message";
char	comment_txt[]		= "Comment";
char	open_txt[]			= "Open";
char	anon_txt[]			= "Anon";
char	list_txt[]			= "List";
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

enum {
	loading_none = 0,
	loading_main,
	loading_closed,
	loading_greeting,
	loading_welcome,
	loading_goodbye
};

void LoadFtpd( struct FtpdBase *fb, const char *filename )
{
	BPTR	file;

	ObtainSemaphore( fb );
	if ( ( file = Open( filename, MODE_OLDFILE ) ) )
	{
		FGets( file, buffer, BUFSIZE-1 );
		if ( !strcmp( buffer, configid, strlen(configid) ) )
		{
			char	buffer[BUFSIZE], buffer2[BUFSIZE], *buf;
			int	i, loading = loading_none;

			while ( ( buf = FGets( file, buffer, BUFSIZE-1 ) ) )
			{
				i = strlen(buf)-1;
				while ( isspace(buf[i]) )buf[i--] = '\0';

				if ( !*buf )										loading = loading_none;
				else if ( loading == loading_none )
				{
						  if ( !stricmp(buf,ftpd_txt) )		loading = loading_main;
					else if ( !stricmp(buf,closed_txt) )	loading = loading_closed;
					else if ( !stricmp(buf,greeting_txt) )	loading = loading_greeting;
					else if ( !stricmp(buf,welcome_txt) )	loading = loading_welcome;
					else if ( !stricmp(buf,goodbye_txt) )	loading = loading_goodbye;
				}
				else switch( loading )
				{
					case loading_main:
					{
						buf = one_word( buffer2, buf );
						buf = skip_word( buf );

							  if ( !stricmp( buffer2, port_txt ) )			port = strlong(buf);
						else if ( !stricmp( buffer2, connections_txt ) )connections = strlong(buf);
						else if ( !stricmp( buffer2, maxcps_txt ) )		maxcps = strlong(buf);
						else if ( !stricmp( buffer2, timeout_txt ) )		timeout = strlong(buf);
						else if ( !stricmp( buffer2, noop_txt ) )			noop = strbool( buf );
						else if ( !stricmp( buffer2, message_txt ) )		message = strbool( buf );
						else if ( !stricmp( buffer2, comment_txt ) )		comment = strbool( buf );
						else if ( !stricmp( buffer2, open_txt ) )			open = strbool( buf );
						else if ( !stricmp( buffer2, anon_txt ) )			anon = strbool( buf );
						else if ( !stricmp( buffer2, list_txt ) )			strncpy( list, buf, 64 );
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
									  if ( !stricmp( buf2, fake_txt ) )		strncpy( access->alias, buf, 512 );
								else if ( !stricmp( buf2, real_txt ) )		strncpy( access->path, buf, 512 );
								else if ( !stricmp( buf2, subdirs_txt ) )	access->subdirs = (unsigned short)strlong( buf );
								else if ( !stricmp( buf2, read_txt ) )		access->read = (unsigned short)strlong( buf );
								else if ( !stricmp( buf2, write_txt ) )	access->write = (unsigned short)strlong( buf );
								else if ( !stricmp( buf2, delete_txt ) )	access->delete = (unsigned short)strlong( buf );
							}
						}
					}
					break;

				case loading_closed:
					strcat( buffer, "\n" );
					closed = strappend( closed, buffer );
					break;

				case loading_greeting:
					strcat( buffer, "\n" );
					greeting = strappend( greeting, buffer );
					break;

				case loading_welcome:
					strcat( buffer, "\n" );
					welcome = strappend( welcome, buffer );
					break;

				case loading_goodbye:
					strcat( buffer, "\n" );
					goodbye = strappend( goodbye, buffer );
					break;

				default:
					break;
			}
		}
		Close( file );
	}
	ReleaseSemaphore( fb );
}

