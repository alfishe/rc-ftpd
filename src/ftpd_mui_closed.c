/*****************************************************************************************************
 * rc-ftpd ©2000 Robin Cloutman <rycochet2@yahoo.com>                                                *
 * --------------------------------------------------                                                *
 * MUI ftp daemon, may be split into ftpd.library, ftpd, ftpserv and ftpgui later.                   *
 *****************************************************************************************************/

// We DON'T want bloody stdio.h!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#define	_STDIO_H_

#include <sys/types.h>

#include	<ctype.h>
#include	<exec/memory.h>
#include	<string.h>
#include <libraries/gadtools.h>
#include <libraries/asl.h>
#include <libraries/mui.h>
#include	<mui/MUIundoc.h>
#include	<mui/textinput_mcc.h>

#include	<clib/alib_protos.h>
#include	<clib/muimaster_protos.h>

#include	<inline/exec.h>
#include	<inline/socket.h>
#include	<inline/dos.h>

#include "ftpd.h"

Object	*CLOSED_Text;

void SetClosed( void )
{
	if ( closed )set( CLOSED_Text, MUIA_Textinput_Contents, closed );
}

void GetClosed( void )
{
	char *string, buffer[BUFSIZE];

	get( CLOSED_Text, MUIA_Textinput_Contents, &string );
	if ( closed )FreePooled( myPool, closed, strlen(closed)+1 );
	closed = NULL;
	while ( *string )
	{
		string = one_line( buffer, string );
		if ( !*buffer || *buffer=='\n' )closed = strappend( closed, "." );
		if ( buffer[strlen(buffer)-1]!='\n' )strcat( buffer, "\n" );
		closed = strappend( closed, buffer );
	}
}

Object *ClosedGroup( void )
{
	Object *new;

	new = VGroup,
		MUIA_HelpNode, "MESS",
		InnerSpacing(4,4),
		ReadListFrame,
		Child, VGroup,
			Child, ReplaceCodes(),
			Child, CLOSED_Text = TextinputscrollObject,
				StringFrame,
				MUIA_Textinput_Styles, MUIV_Textinput_Styles_None,
				MUIA_Textinput_Font, MUIV_Textinput_Font_Fixed,
				MUIA_Textinput_Tabs, MUIV_Textinput_Tabs_Spaces,
				MUIA_Textinput_Multiline, TRUE,
			End,
		End,
	End;
	SetClosed();
	return(new);
}
