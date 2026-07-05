#include <clib/muimaster_protos.h>
#include <pragmas/muimaster_pragmas.h>

extern struct Library *MUIMasterBase;

APTR MUI_NewObject(char *class, Tag tag1, ...)
{
 return(MUI_NewObjectA(class, (struct TagItem *) &tag1));
}

