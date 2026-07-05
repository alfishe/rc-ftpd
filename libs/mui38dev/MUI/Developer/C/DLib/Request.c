#include <clib/muimaster_protos.h>
#include <pragmas/muimaster_pragmas.h>

extern struct Library *MUIMasterBase;

LONG MUI_Request(APTR app, APTR win, LONGBITS flags, char *title, char *gadgets, char *format, ...)
{
 return(MUI_RequestA(app, win, flags, title, gadgets, format, (APTR) (((ULONG)&format)+4) ));
}

