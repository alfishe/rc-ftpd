
#include <exec/types.h>
#if defined __MAXON__ || defined __STORM__
#include <pragma/exec_lib.h>
#else
#include <proto/exec.h>
#endif
#include <exec/libraries.h>
#if defined __MAXON__ || defined __STORM__
#include <pragma/muimaster_lib.h>
#else
#include <proto/muimaster.h>
#endif
#include <libraries/mui.h>
#if defined __MAXON__ || defined __STORM__
#include <pragma/intuition_lib.h>
#else
#include <proto/intuition.h>
#endif
#include <intuition/classusr.h>
#if defined __MAXON__ || defined __STORM__
#include <pragma/dos_lib.h>
#else
#include <proto/dos.h>
#endif
#include <dos/dos.h>
#include <workbench/workbench.h>
#include <workbench/startup.h>
#include <clib/alib_protos.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#if defined __MAXON__ || defined __STORM__
#include <wbstartup.h>
#endif

#include <mui/BetterBalance_mcc.h>


Object         *apMain        = NULL;
Object         *  wiMain;
Object         *    btSave;

struct Library *MUIMasterBase = NULL;


int main(void)
{
  int ret = RETURN_FAIL;

  if (!(MUIMasterBase = OpenLibrary(MUIMASTER_NAME, MUIMASTER_VMIN)))
    goto bye;


  if (!(apMain = ApplicationObject,
    MUIA_Application_Title,       "BetterBalance-Demo",
    MUIA_Application_Version,     "$VER: BetterBalance-Demo 1.0 (8.6.98) © by Maik \"bZ!\" Schreiber",
    MUIA_Application_Copyright,   "Copyright © 08-Jun-1998 by Maik Schreiber <bZ@iq-computing.de>",
    MUIA_Application_Author,      "Maik Schreiber <bZ@iq-computing.de>",
    MUIA_Application_Description, "Demonstrates BetterBalance.mcc's features",
    MUIA_Application_Base,        "BETTERBALANCEDEMO",

    SubWindow, wiMain = WindowObject,
      MUIA_Window_Title, "BetterBalance-Demo 1.0",
      MUIA_Window_Width, MUIV_Window_Width_Visible(75),
      MUIA_Window_Height, MUIV_Window_Height_Visible(75),
      WindowContents, VGroup,
        Child, TextObject,
          MUIA_Text_Contents, "\n\033cPlease visit \033bThe IQ Computing Web Site \033nat \033bwww.IQ-Computing.de\n",
        End,
        Child, HGroup,
          Child, RectangleObject,
            TextFrame,
          End,
          Child, BetterBalanceObject,
            MUIA_ObjectID, 1,
          End,
          Child, RectangleObject,
            TextFrame,
          End,
          Child, BetterBalanceObject,
            MUIA_ObjectID, 2,
          End,
          Child, RectangleObject,
            TextFrame,
          End,
          Child, BetterBalanceObject,
            MUIA_ObjectID, 3,
          End,
          Child, RectangleObject,
            TextFrame,
          End,
        End,
        Child, BetterBalanceObject,
          MUIA_ObjectID, 4,
        End,
        Child, HGroup,
          Child, RectangleObject,
            TextFrame,
          End,
          Child, BetterBalanceObject,
            MUIA_ObjectID, 5,
          End,
          Child, RectangleObject,
            TextFrame,
          End,
          Child, BetterBalanceObject,
            MUIA_ObjectID, 6,
          End,
          Child, RectangleObject,
            TextFrame,
          End,
          Child, BetterBalanceObject,
            MUIA_ObjectID, 7,
          End,
          Child, RectangleObject,
            TextFrame,
          End,
        End,
        Child, BetterBalanceObject,
          MUIA_ObjectID, 8,
        End,
        Child, HGroup,
          Child, RectangleObject,
            TextFrame,
          End,
          Child, BetterBalanceObject,
            MUIA_ObjectID, 9,
          End,
          Child, RectangleObject,
            TextFrame,
          End,
          Child, BetterBalanceObject,
            MUIA_ObjectID, 10,
          End,
          Child, RectangleObject,
            TextFrame,
          End,
          Child, BetterBalanceObject,
            MUIA_ObjectID, 11,
          End,
          Child, RectangleObject,
            TextFrame,
          End,
        End,
        Child, MUI_MakeObject(MUIO_HBar, 10),
        Child, btSave = SimpleButton("Save balance settings to ENV:"),
      End,
    End,

  End))
    goto bye;

  /* load all BetterBalance positions into application
     (this should be done before opening any window) */
  DoMethod(apMain, MUIM_Application_Load, MUIV_Application_Load_ENV);

  /* btSave lets the user save the balance positions into ENV:
     (NOTE: If you also want to save to ENVARC:, you have to use
     MUIM_Application_Save with MUIV_Application_Save_ENVARC, too!)
  */
  DoMethod(btSave, MUIM_Notify, MUIA_Pressed, FALSE, apMain, 2, MUIM_Application_Save, MUIV_Application_Save_ENV);

  DoMethod(wiMain, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, apMain, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

  {
    ULONG open;

    set(wiMain, MUIA_Window_Open, TRUE);
    get(wiMain, MUIA_Window_Open, &open);
    if (!open)
      goto bye;
  }

  {
    ULONG sigs = 0;

    while (DoMethod(apMain, MUIM_Application_NewInput, &sigs) != MUIV_Application_ReturnID_Quit)
    {
      if (sigs)
      {
        sigs = Wait(sigs | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_E | SIGBREAKF_CTRL_F);
        if (sigs & SIGBREAKF_CTRL_C)
          break;
        if (sigs & SIGBREAKF_CTRL_F)
          set(apMain, MUIA_Application_Iconified, FALSE);
        else if (sigs & SIGBREAKF_CTRL_E)
          set(apMain, MUIA_Application_Iconified, TRUE);
      }
    }
  }

  ret = RETURN_OK;

  bye:
  if (apMain)        MUI_DisposeObject(apMain);
  if (MUIMasterBase) CloseLibrary(MUIMasterBase);
  exit(ret);
}

