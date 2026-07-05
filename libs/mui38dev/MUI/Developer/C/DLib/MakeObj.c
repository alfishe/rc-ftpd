#include <clib/muimaster_protos.h>
#include <pragmas/muimaster_pragmas.h>

extern struct Library *MUIMasterBase;

Object *MUI_MakeObject(LONG type, ...)
{
	return(MUI_MakeObjectA(type, (ULONG *)(((ULONG)&type)+4)));
}

