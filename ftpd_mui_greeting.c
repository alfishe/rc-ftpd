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

Object	*GREETING_Text;

void SetGreeting( void )
{
	if ( greeting )set( GREETING_Text, MUIA_Textinput_Contents, greeting );
}

void GetGreeting( void )
{
	char *string, buffer[BUFSIZE];

	get( GREETING_Text, MUIA_Textinput_Contents, &string );
	if ( greeting )FreePooled( myPool, greeting, strlen(greeting)+1 );
	greeting = NULL;
	while ( *string )
	{
		string = one_line( buffer, string );
		if ( !*buffer || *buffer=='\n' )greeting = strappend( greeting, "." );
		if ( buffer[strlen(buffer)-1]!='\n' )strcat( buffer, "\n" );
		greeting = strappend( greeting, buffer );
	}
}

Object *GreetingGroup( void )
{
	Object *new;

	new = VGroup,
		MUIA_HelpNode, "MESS",
		InnerSpacing(4,4),
		ReadListFrame,
		Child, VGroup,
			Child, ReplaceCodes(),
			Child, GREETING_Text = TextinputscrollObject,
				StringFrame,
				MUIA_Textinput_Styles, MUIV_Textinput_Styles_None,
				MUIA_Textinput_Font, MUIV_Textinput_Font_Fixed,
				MUIA_Textinput_Tabs, MUIV_Textinput_Tabs_Spaces,
				MUIA_Textinput_Multiline, TRUE,
			End,
		End,
	End;
	SetGreeting();
	return(new);
}
