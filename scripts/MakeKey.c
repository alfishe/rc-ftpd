/*
 * MakeKey ID/N/A,NAME/A,EMAIL/A
 */

#include <dos/dos.h>
#include	<dos/rdargs.h>
#include	<dos/var.h>
#include	<exec/memory.h>
#include	<libraries/locale.h>
#include	<string.h>
#include	<inline/dos.h>
#include	<inline/exec.h>
#include	<inline/locale.h>

/*
 * GCC libnix options...
 */
int	__nocommandline = 1;
int	__initlibraries = 0;

/*
 * Prototypes...
 */
int main( void );

/*
 * Global data...
 */
struct Library *SysBase;
const char version[] = "$VER: rc-ftpd_makekey 1.4 (8.8.2000) ฉ2000 Robin Cloutman <rycochet2@yahoo.com>";
char	keyname[]	= "%08.lx.key";
char	cname[]		= "%08.lx ";
char	template[]	= "ID/K/N,TO/K,NAME=EMAIL=ADDRESS/A/F";

enum {
	ARGS_id,
	ARGS_to,
	ARGS_name,
	ARGS_count
};

#define	KEYSIZE	(1<<10)
#define	OFFSET1a	726
#define	OFFSET1b	385

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

unsigned long Seed;
signed char Rand( void )
{
	unsigned long a = Seed;
	unsigned char i = 255;
	do
	{
		unsigned long b = a;

		a <<= 1;
		if ( (long)b <= 0 )a ^= 0x1d872b41;
	}while( ( i >>= 1 ) );
	Seed = a;
	return(a&0xff);
}

int main( void )
{
	struct Library *DOSBase = NULL;
	int ret = RETURN_OK;

	SysBase = (*((struct Library **)4));
	if ( ( DOSBase = OpenLibrary( DOSNAME, 37 ) ) == NULL )ret = RETURN_FAIL;
	else {
		struct RDArgs	*rargs;
		long args[ARGS_count] = { 0, NULL };
		BOOL restart = TRUE;

		if ( !( rargs = ReadArgs( template, args, NULL ) ) )ret = RETURN_ERROR, PrintFault( ERROR_REQUIRED_ARG_MISSING, NULL );
		else
		{
			unsigned long	myid = 0, fakeid = 0, i = 0;
			unsigned char *tmp = (char*)args[ARGS_name], *tmp2 = "5\2fW\1[\5+Z\3\4mฉบ", test[6] = "@<>\n.";

			for ( ; tmp[i] && *tmp2 ; i++ )
			{
				if ( *tmp2>='\6' )tmp2++;
				else if ( tmp[i]==test[*tmp2-1] )tmp2++;
			}
			while ( tmp[i] )i++;
			if ( *tmp2 || i>=512 )
			{
				Printf( "invalid string format and/or length\n", i );
				ret = RETURN_FAIL;
				restart = FALSE;
			}

			if ( args[ARGS_id] )fakeid = (long)*(long*)args[ARGS_id];
			else if ( restart )
			{
				BPTR file;

				if ( ( file = Open( "ftpd.index", MODE_OLDFILE ) ) )
				{
					char	last[16];
					Read( file, last, 16 );
					Close( file );
					StrToLong( last, &fakeid );
				}
				fakeid++;
				if ( !( file = Open( "ftpd.index", MODE_NEWFILE ) ) )
				{
					Printf( "unable to save `%s'\n", (ULONG)"ftpd.index" );
					ret = RETURN_FAIL;
					restart = FALSE;
				}
				else
				{
					VFPrintf( file, "%lu\n", &fakeid );
					Close( file );
				}
			}

			while ( restart && !CheckSignal(SIGBREAKF_CTRL_C) )
			{
				char *name	= (char*)args[ARGS_name], *n, *m;
				char *id = (char*)&fakeid;
				char	key[KEYSIZE], o1, o2, crc = 0, c;
				int	i, j = 0, p1, p2;
				struct DateStamp ds;

				restart = FALSE;

				DateStamp( &ds );
				Seed = (ds.ds_Days ^ ds.ds_Minute ^ ds.ds_Tick );

				for ( i = 0 ; i < KEYSIZE ; i++ )key[i] = '\0';
				key[0] = 'f';
				key[1] = 't';
				key[2] = 'p';
				key[3] = 'd';
				key[4] = '\1';

				p1 = OFFSET1a;
				p2 = OFFSET1b;
				key[p1] = '\1';
				key[p2] = '\1';
				for ( i = 0, n = name ; !restart ; i++ )
				{
					if ( i==0 )c = id[0];
					else if ( i==1 )c = id[1];
					else if ( i==2 )c = id[2];
					else if ( i==3 )c = id[3];
					else if ( i>=4 && i%4==0 )c = crc;
					else c = *n++;
					for ( o1 = o2 = j = 0 ; ( o1==0 || o2==0 || key[(p1+o1)&(KEYSIZE-1)] || key[(p2+o2)&(KEYSIZE-1)] ) && j<255 ; j++ )
					{
						o1 = Rand();
						o2 = o1^c;
					}
					if ( j<255 )
					{
						key[p1] = o1;
						key[p2] = o2;
						p1 = (p1+o1)&(KEYSIZE-1);
						p2 = (p2+o2)&(KEYSIZE-1);
						crc ^= ( o1 ^ o2 );
					}
					else restart = TRUE;
					if ( i>=4 && i%4!=0 && c==0 )break;
				}
				if ( !restart )
				{
					for ( c = j = 0 ; ( c==0 || key[(p1+c)&(KEYSIZE-1)] || key[(p2+c)&(KEYSIZE-1)] ) && j<255 ; j++ )c = Rand();
					key[p1] = c;
					key[p2] = c;
					for ( i = 0 ; i < KEYSIZE ; i++ )while ( key[i]=='\0' )key[i] = Rand();
					name = (char*)args[ARGS_name];
					p1 = OFFSET1a;
					p2 = OFFSET1b;
					for ( i = 0, crc = 0, m = (char*)&myid ; !restart ; i++ )
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
							if ( c!=crc )restart = TRUE;
						}
						else
						{
							if ( c!=*name++ )restart = TRUE;
							Printf( "%c", (ULONG)c );
							if ( c==0 )break;
						}
						crc ^= c;
					}
					if ( myid != fakeid )restart = TRUE;
					if ( !restart )
					{
						BPTR	file	= NULL;
						char filename[128];

						if ( args[ARGS_to] )sprintf( filename, "%s", (char*)args[ARGS_to] );
						else sprintf( filename, keyname, myid );
						if ( !( file = Open( filename, MODE_NEWFILE ) ) )
						{
							Printf( "unable to create keyfile.\n", 0 );
							ret = RETURN_ERROR;
						}
						else
						{
							char comment[512];

							sprintf( comment, cname, myid );
							n = &comment[9];
							name = (char*)args[ARGS_name];
							while ( *name != '\n' && *name != '\r' && *name != '\0' )*n++ = *name++;
							*n = '\0';
							Write( file, key, KEYSIZE );
							Close( file );
							SetComment( filename, comment );
							Printf( "keyfile `%s' created\n", (ULONG)filename );
						}
					}
				}
			}
			FreeArgs( rargs );
		}
		CloseLibrary( DOSBase );
	}
	return ret;
}
