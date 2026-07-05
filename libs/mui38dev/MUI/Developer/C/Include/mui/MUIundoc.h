/*
** MUIundoc.h - Undocumented MUI features
** Used by rc-ftpd
*/

#ifndef MUI_MUIUNDOC_H
#define MUI_MUIUNDOC_H

/* Undocumented method for setting window sleep state */
#ifndef MUIM_Window_SetCycleChain
#define MUIM_Window_SetCycleChain 0x80426510
#endif

/* Application sleep */
#ifndef MUIA_Application_Sleep
#define MUIA_Application_Sleep 0x80425711
#endif

#endif /* MUI_MUIUNDOC_H */
