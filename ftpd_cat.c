/*****************************************************************************************************
 * rc-ftpd ©2000 Robin Cloutman <rycochet2@yahoo.com>                                                *
 * --------------------------------------------------                                                *
 * MUI ftp daemon, may be split into ftpd.library, ftpd, ftpserv and ftpgui later.                   *
 *****************************************************************************************************/

// We DON'T want bloody stdio.h!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#define	_STDIO_H_

#include	<sys/types.h>
#include	<intuition/intuition.h>
#include	<inline/locale.h>
#include	<inline/intuition.h>

#define	CATCOMP_ARRAY
#include "ftpd.h"

char *GetString( LONG id )
{
	char *str = "";
	int i;

	for ( i = 0 ; i < 150 ; i++ )
	{
		if ( CatCompArray[i].cca_ID == id )
		{
			str = CatCompArray[i].cca_Str;
			break;
		}
	}
	if ( LocaleBase )return( GetCatalogStr( catalog, id, str ) );
	return( str );
}

void ErrorString( LONG id )
{
	struct EasyStruct req =
	{
		sizeof(struct EasyStruct),
		0,
		GetString(MSG_ERROR),
		"%s",
		GetString(MSG_OK)
	};

	EasyRequest( NULL, &req, NULL, (ULONG)GetString(id) );
}
