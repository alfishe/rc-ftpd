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
#include	<mui/Textinput_mcc.h>

#include	<clib/alib_protos.h>
#include	<clib/muimaster_protos.h>

#include	<inline/exec.h>
#include	<inline/socket.h>
#include	<inline/dos.h>

#include "ftpd.h"

Object	*WELCOME_Text;

void SetWelcome( void )
{
	if ( welcome )set( WELCOME_Text, MUIA_Textinput_Contents, welcome );
}

void GetWelcome( void )
{
	char *string, buffer[BUFSIZE];

	get( WELCOME_Text, MUIA_Textinput_Contents, &string );
	if ( welcome )FreePooled( myPool, welcome, strlen(welcome)+1 );
	welcome = NULL;
	while ( *string )
	{
		string = one_line( buffer, string );
		if ( !*buffer || *buffer=='\n' )welcome = strappend( welcome, "." );
		if ( buffer[strlen(buffer)-1]!='\n' )strcat( buffer, "\n" );
		welcome = strappend( welcome, buffer );
	}
}

Object *WelcomeGroup( void )
{
	Object *new;

	new = VGroup,
		MUIA_HelpNode, "MESS",
		InnerSpacing(4,4),
		ReadListFrame,
		Child, VGroup,
			Child, ReplaceCodes(),
			Child, WELCOME_Text = TextinputscrollObject,
				StringFrame,
				MUIA_Textinput_Styles, MUIV_Textinput_Styles_None,
				MUIA_Textinput_Font, MUIV_Textinput_Font_Fixed,
				MUIA_Textinput_Tabs, MUIV_Textinput_Tabs_Spaces,
				MUIA_Textinput_Multiline, TRUE,
			End,
		End,
	End;
	SetWelcome();
	return(new);
}
