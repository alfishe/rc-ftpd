
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
#include <dos/dos.h>
#include <clib/alib_protos.h>
#include <string.h>
#include <stdlib.h>
#if defined __MAXON__ || defined __STORM__
#include <wbstartup.h>
#endif

#include <mui/Lamp_mcc.h>


/* copied from <blizzy/macros.h> */
#ifndef REG
#ifdef _DCC
#define REG(x) __ ## x
#else
#define REG(x) register __ ## x
#endif
#endif

#ifndef ASM
#if defined __MAXON__ || defined __STORM__ || defined _DCC
#define ASM
#else
#define ASM __asm
#endif
#endif

#ifndef SAVEDS
#ifdef __MAXON__
#define SAVEDS
#endif
#if defined __STORM__ || defined __SASC
#define SAVEDS __saveds
#endif
#if defined _GCC || defined _DCC
#define SAVEDS __geta4
#endif
#endif


#define DropableLampObject NewObject(clDropableLamp->mcc_Class, NULL

struct DropableLampData
{
  ULONG dummy;
};


#define BlinkingLampObject NewObject(clBlinkingLamp->mcc_Class, NULL

#define MUIM_BlinkingLamp_Blink 0x85b9ffff

struct BlinkingLampData
{
  struct MUI_InputHandlerNode ihn;
  BOOL                        black;
};


#define TrafficLightObject NewObject(clTrafficLight->mcc_Class, NULL

#define MUIM_TrafficLight_Change 0x85b9fffe

struct TrafficLightData
{
  Object                      *lpLampRed;
  Object                      *lpLampYellow;
  Object                      *lpLampGreen;
  struct MUI_InputHandlerNode  ihn;
  UBYTE                        state;
};


#define GUI_LAMP_COLORTYPE 1


Object                 *apMain         = NULL;
Object                 *  wiMain;
Object                 *    lpLamp;
Object                 *    caColor;
Object                 *    btTiny;
Object                 *    btSmall;
Object                 *    btMedium;
Object                 *    btBig;
Object                 *    btHuge;
Object                 *    btColorOff;
Object                 *    btColorOk;
Object                 *    btColorWarning;
Object                 *    btColorError;
Object                 *    btColorFatalError;
Object                 *    btColorProcessing;
Object                 *    btColorLookingUp;
Object                 *    btColorConnecting;
Object                 *    btColorSendingData;
Object                 *    btColorReceivingData;
Object                 *    btColorLoadingData;
Object                 *    btColorSavingData;
Object                 *    pdShine;
Object                 *    pdHalfShine;
Object                 *    pdBackground;
Object                 *    pdHalfShadow;
Object                 *    pdShadow;
Object                 *    pdText;
Object                 *    pdFill;
Object                 *    pdMark;
Object                 *    txColorType;

struct MUI_CustomClass *clDropableLamp = NULL;
struct MUI_CustomClass *clBlinkingLamp = NULL;
struct MUI_CustomClass *clTrafficLight = NULL;

struct Library         *MUIMasterBase  = NULL;


ULONG DoSuperNew(struct IClass *cl, Object *obj, ULONG tag1, ...)
{
  return(DoSuperMethod(cl, obj, OM_NEW, &tag1, NULL));
}

ULONG DropableLamp_DragDrop(Object *obj, struct MUIP_DragDrop *msg)
{
  struct MUI_PenSpec *penspec;

  get(msg->obj, MUIA_Pendisplay_Spec, &penspec);
  set(obj, MUIA_Lamp_PenSpec, penspec);

  return(0);
}
ULONG ASM SAVEDS DropableLamp_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
  switch (msg->MethodID)
  {
    case MUIM_DragQuery: return(MUIV_DragQuery_Accept);
    case MUIM_DragDrop:  return(DropableLamp_DragDrop(obj, (struct MUIP_DragDrop *) msg));
  }
  return(DoSuperMethodA(cl, obj, msg));
}

ULONG BlinkingLamp_Setup(struct IClass *cl, Object *obj, Msg msg)
{
  if (!DoSuperMethodA(cl, obj, msg))
    return(FALSE);
  {
    struct BlinkingLampData *data = INST_DATA(cl, obj);

    memset(data, 0, sizeof(struct BlinkingLampData));

    DoMethod(obj, MUIM_Lamp_SetRGB, 0, 0, 0);
    data->black = TRUE;

    data->ihn.ihn_Object = obj;
    data->ihn.ihn_Method = MUIM_BlinkingLamp_Blink;
    data->ihn.ihn_Flags  = MUIIHNF_TIMER;
    {
      static UWORD millis = 50;

      data->ihn.ihn_Millis = millis;
      millis += 32;
    }
    DoMethod(_app(obj), MUIM_Application_AddInputHandler, &data->ihn);
  }
  return(TRUE);
}
ULONG BlinkingLamp_Cleanup(struct IClass *cl, Object *obj, Msg msg)
{
  struct BlinkingLampData *data = INST_DATA(cl, obj);

  DoMethod(_app(obj), MUIM_Application_RemInputHandler, &data->ihn);

  return(DoSuperMethodA(cl, obj, msg));
}
ULONG BlinkingLamp_Blink(struct IClass *cl, Object *obj)
{
  struct BlinkingLampData *data = INST_DATA(cl, obj);

  if (data->black)
  {
    set(obj, MUIA_Lamp_Color, MUIV_Lamp_Color_FatalError);
    data->black = FALSE;
  }
  else
  {
    DoMethod(obj, MUIM_Lamp_SetRGB, 0, 0, 0);
    data->black = TRUE;
  }
  return(TRUE);
}
ULONG ASM SAVEDS BlinkingLamp_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
  switch (msg->MethodID)
  {
    case MUIM_Setup:              return(BlinkingLamp_Setup(cl, obj, msg));
    case MUIM_Cleanup:            return(BlinkingLamp_Cleanup(cl, obj, msg));

    case MUIM_BlinkingLamp_Blink: return(BlinkingLamp_Blink(cl, obj));
  }
  return(DoSuperMethodA(cl, obj, msg));
}

ULONG TrafficLight_NEW(struct IClass *cl, Object *obj, struct opSet *msg)
{
  Object *lpLampRed;
  Object *lpLampYellow;
  Object *lpLampGreen;

  if (obj = (Object *) DoSuperNew(cl, obj,
    MUIA_Group_VertSpacing, 1,
    Child, lpLampRed    = LampObject, MUIA_Lamp_Type, MUIV_Lamp_Type_Huge, End,
    Child, lpLampYellow = LampObject, MUIA_Lamp_Type, MUIV_Lamp_Type_Huge, End,
    Child, lpLampGreen  = LampObject, MUIA_Lamp_Type, MUIV_Lamp_Type_Huge, End,
    TAG_MORE, msg->ops_AttrList))
  {
    struct TrafficLightData *data = INST_DATA(cl, obj);

    memset(data, 0, sizeof(struct TrafficLightData));

    data->lpLampRed    = lpLampRed;
    data->lpLampYellow = lpLampYellow;
    data->lpLampGreen  = lpLampGreen;

    return((ULONG) obj);
  }
  return(0);
}
ULONG TrafficLight_Setup(struct IClass *cl, Object *obj, Msg msg)
{
  if (!DoSuperMethodA(cl, obj, msg))
    return(FALSE);
  {
    struct TrafficLightData *data = INST_DATA(cl, obj);

    DoMethod(data->lpLampRed,    MUIM_Lamp_SetRGB, 0xffffffff, 0, 0);
    DoMethod(data->lpLampYellow, MUIM_Lamp_SetRGB, 0,          0, 0);
    DoMethod(data->lpLampGreen,  MUIM_Lamp_SetRGB, 0,          0, 0);
    data->state = 0;

    memset(&data->ihn, 0, sizeof(struct MUI_InputHandlerNode));
    data->ihn.ihn_Object = obj;
    data->ihn.ihn_Method = MUIM_TrafficLight_Change;
    data->ihn.ihn_Flags  = MUIIHNF_TIMER;
    data->ihn.ihn_Millis = 7500;
    DoMethod(_app(obj), MUIM_Application_AddInputHandler, &data->ihn);
  }
  return(TRUE);
}
ULONG TrafficLight_Cleanup(struct IClass *cl, Object *obj, Msg msg)
{
  struct TrafficLightData *data = INST_DATA(cl, obj);

  DoMethod(_app(obj), MUIM_Application_RemInputHandler, &data->ihn);

  return(DoSuperMethodA(cl, obj, msg));
}
ULONG TrafficLight_Change(struct IClass *cl, Object *obj)
{
  struct TrafficLightData *data = INST_DATA(cl, obj);
  Object                  *app;

  get(obj, MUIA_ApplicationObject, &app);

  DoMethod(app, MUIM_Application_RemInputHandler, &data->ihn);

  memset(&data->ihn, 0, sizeof(struct MUI_InputHandlerNode));
  data->ihn.ihn_Object = obj;
  data->ihn.ihn_Method = MUIM_TrafficLight_Change;
  data->ihn.ihn_Flags  = MUIIHNF_TIMER;

  switch (data->state)
  {
    case 0:
      DoMethod(data->lpLampYellow, MUIM_Lamp_SetRGB, 0xffffffff, 0xffffffff, 0);
      data->ihn.ihn_Millis = 1500;
      data->state = 1;
      break;
    case 1:
      DoMethod(data->lpLampRed,    MUIM_Lamp_SetRGB, 0, 0,          0);
      DoMethod(data->lpLampYellow, MUIM_Lamp_SetRGB, 0, 0,          0);
      DoMethod(data->lpLampGreen,  MUIM_Lamp_SetRGB, 0, 0xffffffff, 0);
      data->ihn.ihn_Millis = 7500;
      data->state = 2;
      break;
    case 2:
      DoMethod(data->lpLampGreen,  MUIM_Lamp_SetRGB, 0,          0,          0);
      DoMethod(data->lpLampYellow, MUIM_Lamp_SetRGB, 0xffffffff, 0xffffffff, 0);
      data->ihn.ihn_Millis = 1500;
      data->state = 3;
      break;
    case 3:
      DoMethod(data->lpLampYellow, MUIM_Lamp_SetRGB, 0,          0, 0);
      DoMethod(data->lpLampRed,    MUIM_Lamp_SetRGB, 0xffffffff, 0, 0);
      data->ihn.ihn_Millis = 7500;
      data->state = 0;
      break;
  }

  DoMethod(_app(obj), MUIM_Application_AddInputHandler, &data->ihn);

  return(TRUE);
}
ULONG ASM SAVEDS TrafficLight_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
  switch (msg->MethodID)
  {
    case OM_NEW:                   return(TrafficLight_NEW(cl, obj, (struct opSet *) msg));

    case MUIM_Setup:               return(TrafficLight_Setup(cl, obj, msg));
    case MUIM_Cleanup:             return(TrafficLight_Cleanup(cl, obj, msg));

    case MUIM_TrafficLight_Change: return(TrafficLight_Change(cl, obj));
  }
  return(DoSuperMethodA(cl, obj, msg));
}

Object *lampbutton(ULONG id)
{
  char *label;
  char  key;

  switch (id)
  {
    case MUIV_Lamp_Color_Ok:
      label = "Ok";
      key   = 'k';
      break;
    case MUIV_Lamp_Color_Warning:
      label = "Warning";
      key   = 'w';
      break;
    case MUIV_Lamp_Color_Error:
      label = "Error";
      key   = 'e';
      break;
    case MUIV_Lamp_Color_FatalError:
      label = "Fatal error";
      key   = 'f';
      break;
    case MUIV_Lamp_Color_Processing:
      label = "Processing";
      key   = 'p';
      break;
    case MUIV_Lamp_Color_LookingUp:
      label = "Looking up";
      key   = 'i';
      break;
    case MUIV_Lamp_Color_Connecting:
      label = "Connecting";
      key   = 'c';
      break;
    case MUIV_Lamp_Color_SendingData:
      label = "Sending data";
      key   = 'n';
      break;
    case MUIV_Lamp_Color_ReceivingData:
      label = "Receiving data";
      key   = 'r';
      break;
    case MUIV_Lamp_Color_LoadingData:
      label = "Loading data";
      key   = 'l';
      break;
    case MUIV_Lamp_Color_SavingData:
      label = "Saving data";
      key   = 's';
      break;
    default:   /* case MUIV_Lamp_Color_Off: */
      label = "Off";
      key   = 'o';
      break;
  }

  return(HGroup,
    ButtonFrame,
    MUIA_Background, MUII_ButtonBack,
    MUIA_Font, MUIV_Font_Button,
    MUIA_HorizWeight, 0,
    MUIA_ControlChar, key,
    MUIA_InputMode, MUIV_InputMode_RelVerify,
    MUIA_CycleChain, TRUE,
    MUIA_Group_VertSpacing, 0,
    Child, LampObject,
      MUIA_Lamp_Type, MUIV_Lamp_Type_Huge,
      MUIA_Lamp_Color, id,
    End,
    Child, MUI_MakeObject(MUIO_VBar, 2),
    Child, RectangleObject, End,
    Child, TextObject,
      MUIA_Text_SetMax, TRUE,
      MUIA_Text_Contents, label,
      MUIA_Text_HiChar, key,
    End,
    Child, RectangleObject, End,
  End);
}
Object *button(char *label, char key)
{
  return(TextObject,
    ButtonFrame,
    MUIA_Background, MUII_ButtonBack,
    MUIA_Font, MUIV_Font_Button,
    MUIA_InputMode, MUIV_InputMode_RelVerify,
    MUIA_Text_PreParse, "\033c",
    MUIA_Text_Contents, label,
    MUIA_Text_HiChar, key,
    MUIA_ControlChar, key,
    MUIA_CycleChain, TRUE,
  End);
}

int main(void)
{
  int                ret           = RETURN_FAIL;
  static const char *txt_rgMain [] =
  {
    "Setting attributes",
    "Just kidding",
    NULL
  };

  if (!(MUIMasterBase = OpenLibrary(MUIMASTER_NAME, MUIMASTER_VMIN)))
    goto bye;

  if (!(clDropableLamp = MUI_CreateCustomClass(NULL, MUIC_Lamp, NULL, sizeof(struct DropableLampData), DropableLamp_Dispatcher)))
    goto bye;
  if (!(clBlinkingLamp = MUI_CreateCustomClass(NULL, MUIC_Lamp, NULL, sizeof(struct BlinkingLampData), BlinkingLamp_Dispatcher)))
    goto bye;
  if (!(clTrafficLight = MUI_CreateCustomClass(NULL, MUIC_Group, NULL, sizeof(struct TrafficLightData), TrafficLight_Dispatcher)))
    goto bye;

  if (!(apMain = ApplicationObject,
    MUIA_Application_Title,       "Lamp-Demo",
    MUIA_Application_Version,     "$VER: Lamp-Demo 1.1 (16.3.97) © by Maik \"BLiZZeR\" Schreiber",
    MUIA_Application_Copyright,   "Copyright © 16-Mar-1997 by Maik Schreiber <BLiZZeR@dame.de>",
    MUIA_Application_Author,      "Maik Schreiber <BLiZZeR@dame.de>",
    MUIA_Application_Description, "Demonstrates Lamp.mcc's features",
    MUIA_Application_Base,        "LAMPDEMO",

    SubWindow, wiMain = WindowObject,
      MUIA_Window_Title, "Lamp-Demo 1.1",
      MUIA_Window_Width, MUIV_Window_Width_Visible(100),
      MUIA_Window_Height, MUIV_Window_Height_Visible(100),
      WindowContents, VGroup,
        Child, RegisterGroup(txt_rgMain),
          MUIA_Register_Frame, TRUE,
          MUIA_CycleChain, TRUE,
          Child, VGroup,
            GroupFrame,
            MUIA_Background, MUII_PageBack,
            Child, HGroup,
              Child, VGroup,
                GroupFrameT("Lamp"),
                Child, RectangleObject, End,
                Child, HGroup,
                  Child, HSpace(25),
                  Child, lpLamp = DropableLampObject,
                    MUIA_Lamp_Type, MUIV_Lamp_Type_Huge,
                    MUIA_Lamp_Red,   0xffffffff,
                    MUIA_Lamp_Green, 0,
                    MUIA_Lamp_Blue,  0,
                  End,
                  Child, HSpace(25),
                End,
                Child, RectangleObject, End,
              End,
              Child, HSpace(8),
              Child, VGroup,
                Child, HGroup,
                  GroupFrameT("Color"),
                  Child, VGroup,
                    Child, MUI_MakeObject(MUIO_BarTitle, "Custom"),
                    Child, caColor = ColoradjustObject,
                      MUIA_Coloradjust_Red,   0xffffffff,
                      MUIA_Coloradjust_Green, 0,
                      MUIA_Coloradjust_Blue,  0,
                    End,
                  End,
                  Child, MUI_MakeObject(MUIO_VBar, 6),
                  Child, VGroup,
                    Child, MUI_MakeObject(MUIO_BarTitle, "Preferences"),
                    Child, RectangleObject, End,
                    Child, ColGroup(2),
                      MUIA_Group_VertSpacing, 1,
                      MUIA_Group_SameWidth, TRUE,
                      Child, btColorOff           = lampbutton(MUIV_Lamp_Color_Off),
                      Child, btColorLookingUp     = lampbutton(MUIV_Lamp_Color_LookingUp),
                      Child, btColorOk            = lampbutton(MUIV_Lamp_Color_Ok),
                      Child, btColorConnecting    = lampbutton(MUIV_Lamp_Color_Connecting),
                      Child, btColorWarning       = lampbutton(MUIV_Lamp_Color_Warning),
                      Child, btColorSendingData   = lampbutton(MUIV_Lamp_Color_SendingData),
                      Child, btColorError         = lampbutton(MUIV_Lamp_Color_Error),
                      Child, btColorReceivingData = lampbutton(MUIV_Lamp_Color_ReceivingData),
                      Child, btColorFatalError    = lampbutton(MUIV_Lamp_Color_FatalError),
                      Child, btColorLoadingData   = lampbutton(MUIV_Lamp_Color_LoadingData),
                      Child, btColorProcessing    = lampbutton(MUIV_Lamp_Color_Processing),
                      Child, btColorSavingData    = lampbutton(MUIV_Lamp_Color_SavingData),
                    End,
                    Child, RectangleObject, End,
                  End,
                End,
                Child, VSpace(8),
                Child, HGroup,
                  Child, VGroup,
                    GroupFrameT("Type"),
                    GroupSpacing(1),
                    Child, RectangleObject, End,
                    Child, ColGroup(2),
                      GroupSpacing(1),
                      Child, btTiny   = button("Tiny",   't'),
                      Child, btMedium = button("Medium", 'd'),
                      Child, btSmall  = button("Small",  'm'),
                      Child, btBig    = button("Big",    'b'),
                    End,
                    Child, btHuge = button("Huge", 'h'),
                    Child, RectangleObject, End,
                  End,
                  Child, HSpace(8),
                  Child, ColGroup(5),
                    GroupFrameT("PenSpec (drag'n'drop)"),
                    Child, FreeLabel("Shine:"),
                    Child, pdShine      = PendisplayObject, TextFrame, MUIA_Background, MUII_TextBack, MUIA_Draggable, TRUE, End,
                    Child, HSpace(8),
                    Child, FreeLabel("Shadow:"),
                    Child, pdShadow     = PendisplayObject, TextFrame, MUIA_Background, MUII_TextBack, MUIA_Draggable, TRUE, End,
                    Child, FreeLabel("Half shine:"),
                    Child, pdHalfShine  = PendisplayObject, TextFrame, MUIA_Background, MUII_TextBack, MUIA_Draggable, TRUE, End,
                    Child, HSpace(8),
                    Child, FreeLabel("Text:"),
                    Child, pdText       = PendisplayObject, TextFrame, MUIA_Background, MUII_TextBack, MUIA_Draggable, TRUE, End,
                    Child, FreeLabel("Background:"),
                    Child, pdBackground = PendisplayObject, TextFrame, MUIA_Background, MUII_TextBack, MUIA_Draggable, TRUE, End,
                    Child, HSpace(8),
                    Child, FreeLabel("Fill:"),
                    Child, pdFill       = PendisplayObject, TextFrame, MUIA_Background, MUII_TextBack, MUIA_Draggable, TRUE, End,
                    Child, FreeLabel("Half shadow:"),
                    Child, pdHalfShadow = PendisplayObject, TextFrame, MUIA_Background, MUII_TextBack, MUIA_Draggable, TRUE, End,
                    Child, HSpace(8),
                    Child, FreeLabel("Mark:"),
                    Child, pdMark       = PendisplayObject, TextFrame, MUIA_Background, MUII_TextBack, MUIA_Draggable, TRUE, End,
                  End,
                End,
              End,
            End,
            Child, VSpace(8),
            Child, HGroup,
              Child, Label("Current lamp color type:"),
              Child, txColorType = TextObject,
                TextFrame,
                MUIA_Background, MUII_TextBack,
                MUIA_Text_Contents, "direct color value",
              End,
            End,
          End,
          Child, HGroup,
            GroupFrame,
            MUIA_Background, MUII_PageBack,
            Child, RectangleObject, End,
            Child, VGroup,
              Child, RectangleObject, End,
              Child, MUI_MakeObject(MUIO_BarTitle, "Blinking lamps"),
              Child, HGroup,
                Child, RectangleObject, End,
                Child, ColGroup(3),
                  GroupSpacing(1),
                  Child, BlinkingLampObject, MUIA_Lamp_Type, MUIV_Lamp_Type_Big, End,
                  Child, BlinkingLampObject, MUIA_Lamp_Type, MUIV_Lamp_Type_Big, End,
                  Child, BlinkingLampObject, MUIA_Lamp_Type, MUIV_Lamp_Type_Big, End,
                  Child, BlinkingLampObject, MUIA_Lamp_Type, MUIV_Lamp_Type_Big, End,
                  Child, BlinkingLampObject, MUIA_Lamp_Type, MUIV_Lamp_Type_Big, End,
                  Child, BlinkingLampObject, MUIA_Lamp_Type, MUIV_Lamp_Type_Big, End,
                  Child, BlinkingLampObject, MUIA_Lamp_Type, MUIV_Lamp_Type_Big, End,
                  Child, BlinkingLampObject, MUIA_Lamp_Type, MUIV_Lamp_Type_Big, End,
                  Child, BlinkingLampObject, MUIA_Lamp_Type, MUIV_Lamp_Type_Big, End,
                End,
                Child, RectangleObject, End,
              End,
              Child, VSpace(8),
              Child, MUI_MakeObject(MUIO_BarTitle, "Traffic light"),
              Child, HGroup,
                Child, RectangleObject, End,
                Child, TrafficLightObject, End,
                Child, RectangleObject, End,
              End,
              Child, RectangleObject, End,
            End,
            Child, RectangleObject, End,
          End,
        End,
      End,
    End,
  End))
    goto bye;

  /* Damn *NASTY* MUI bug! */
  set(pdShine,      MUIA_Dropable, FALSE);
  set(pdHalfShine,  MUIA_Dropable, FALSE);
  set(pdBackground, MUIA_Dropable, FALSE);
  set(pdHalfShadow, MUIA_Dropable, FALSE);
  set(pdShadow,     MUIA_Dropable, FALSE);
  set(pdText,       MUIA_Dropable, FALSE);
  set(pdFill,       MUIA_Dropable, FALSE);
  set(pdMark,       MUIA_Dropable, FALSE);
  /* end of annoying bug handling */

  DoMethod(pdShine,      MUIM_Pendisplay_SetMUIPen, MPEN_SHINE);
  DoMethod(pdHalfShine,  MUIM_Pendisplay_SetMUIPen, MPEN_HALFSHINE);
  DoMethod(pdBackground, MUIM_Pendisplay_SetMUIPen, MPEN_BACKGROUND);
  DoMethod(pdHalfShadow, MUIM_Pendisplay_SetMUIPen, MPEN_HALFSHADOW);
  DoMethod(pdShadow,     MUIM_Pendisplay_SetMUIPen, MPEN_SHADOW);
  DoMethod(pdText,       MUIM_Pendisplay_SetMUIPen, MPEN_TEXT);
  DoMethod(pdFill,       MUIM_Pendisplay_SetMUIPen, MPEN_FILL);
  DoMethod(pdMark,       MUIM_Pendisplay_SetMUIPen, MPEN_MARK);

  DoMethod(lpLamp, MUIM_Notify, MUIA_Lamp_ColorType, MUIV_EveryTime, apMain, 2, MUIM_Application_ReturnID, GUI_LAMP_COLORTYPE);

  DoMethod(caColor, MUIM_Notify, MUIA_Coloradjust_RGB, MUIV_EveryTime, lpLamp, 3, MUIM_Set, MUIA_Lamp_Color, MUIV_TriggerValue);

  DoMethod(btColorOff,           MUIM_Notify, MUIA_Pressed, FALSE, lpLamp, 3, MUIM_Set, MUIA_Lamp_Color, MUIV_Lamp_Color_Off);
  DoMethod(btColorOk,            MUIM_Notify, MUIA_Pressed, FALSE, lpLamp, 3, MUIM_Set, MUIA_Lamp_Color, MUIV_Lamp_Color_Ok);
  DoMethod(btColorWarning,       MUIM_Notify, MUIA_Pressed, FALSE, lpLamp, 3, MUIM_Set, MUIA_Lamp_Color, MUIV_Lamp_Color_Warning);
  DoMethod(btColorError,         MUIM_Notify, MUIA_Pressed, FALSE, lpLamp, 3, MUIM_Set, MUIA_Lamp_Color, MUIV_Lamp_Color_Error);
  DoMethod(btColorFatalError,    MUIM_Notify, MUIA_Pressed, FALSE, lpLamp, 3, MUIM_Set, MUIA_Lamp_Color, MUIV_Lamp_Color_FatalError);
  DoMethod(btColorProcessing,    MUIM_Notify, MUIA_Pressed, FALSE, lpLamp, 3, MUIM_Set, MUIA_Lamp_Color, MUIV_Lamp_Color_Processing);
  DoMethod(btColorLookingUp,     MUIM_Notify, MUIA_Pressed, FALSE, lpLamp, 3, MUIM_Set, MUIA_Lamp_Color, MUIV_Lamp_Color_LookingUp);
  DoMethod(btColorConnecting,    MUIM_Notify, MUIA_Pressed, FALSE, lpLamp, 3, MUIM_Set, MUIA_Lamp_Color, MUIV_Lamp_Color_Connecting);
  DoMethod(btColorSendingData,   MUIM_Notify, MUIA_Pressed, FALSE, lpLamp, 3, MUIM_Set, MUIA_Lamp_Color, MUIV_Lamp_Color_SendingData);
  DoMethod(btColorReceivingData, MUIM_Notify, MUIA_Pressed, FALSE, lpLamp, 3, MUIM_Set, MUIA_Lamp_Color, MUIV_Lamp_Color_ReceivingData);
  DoMethod(btColorLoadingData,   MUIM_Notify, MUIA_Pressed, FALSE, lpLamp, 3, MUIM_Set, MUIA_Lamp_Color, MUIV_Lamp_Color_LoadingData);
  DoMethod(btColorSavingData,    MUIM_Notify, MUIA_Pressed, FALSE, lpLamp, 3, MUIM_Set, MUIA_Lamp_Color, MUIV_Lamp_Color_SavingData);

  DoMethod(btTiny,   MUIM_Notify, MUIA_Pressed, FALSE, lpLamp, 3, MUIM_Set, MUIA_Lamp_Type, MUIV_Lamp_Type_Tiny);
  DoMethod(btSmall,  MUIM_Notify, MUIA_Pressed, FALSE, lpLamp, 3, MUIM_Set, MUIA_Lamp_Type, MUIV_Lamp_Type_Small);
  DoMethod(btMedium, MUIM_Notify, MUIA_Pressed, FALSE, lpLamp, 3, MUIM_Set, MUIA_Lamp_Type, MUIV_Lamp_Type_Medium);
  DoMethod(btBig,    MUIM_Notify, MUIA_Pressed, FALSE, lpLamp, 3, MUIM_Set, MUIA_Lamp_Type, MUIV_Lamp_Type_Big);
  DoMethod(btHuge,   MUIM_Notify, MUIA_Pressed, FALSE, lpLamp, 3, MUIM_Set, MUIA_Lamp_Type, MUIV_Lamp_Type_Huge);

  DoMethod(wiMain, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, apMain, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

  ret = RETURN_ERROR;

  {
    ULONG open;

    set(wiMain, MUIA_Window_Open, TRUE);
    get(wiMain, MUIA_Window_Open, &open);
    if (!open)
      goto bye;
  }

  {
    ULONG sigs    = 0UL;
    BOOL  running = TRUE;

    while (running)
    {
      switch (DoMethod(apMain, MUIM_Application_NewInput, &sigs))
      {
        case MUIV_Application_ReturnID_Quit:
          running = FALSE;
          break;
        case GUI_LAMP_COLORTYPE:
          {
            ULONG type;

            get(lpLamp, MUIA_Lamp_ColorType, &type);
            switch (type)
            {
              case MUIV_Lamp_ColorType_UserDefined:
                set(txColorType, MUIA_Text_Contents, "user preferences");
                break;
              case MUIV_Lamp_ColorType_Color:
                set(txColorType, MUIA_Text_Contents, "direct color value");
                break;
              case MUIV_Lamp_ColorType_PenSpec:
                set(txColorType, MUIA_Text_Contents, "direct PenSpec definition");
                break;
            }
          }
          break;
      }

      if (running && sigs)
      {
        sigs = Wait(sigs | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_E | SIGBREAKF_CTRL_F);

        if (sigs & SIGBREAKF_CTRL_C)
          running = FALSE;
        if (sigs & SIGBREAKF_CTRL_F)
          set(apMain, MUIA_Application_Iconified, FALSE);
        else if (sigs & SIGBREAKF_CTRL_E)
          set(apMain, MUIA_Application_Iconified, TRUE);
      }
    }
  }

  ret = RETURN_OK;

  bye:
  if (apMain)         MUI_DisposeObject(apMain);
  if (clTrafficLight) MUI_DeleteCustomClass(clTrafficLight);
  if (clBlinkingLamp) MUI_DeleteCustomClass(clBlinkingLamp);
  if (clDropableLamp) MUI_DeleteCustomClass(clDropableLamp);
  if (MUIMasterBase)  CloseLibrary(MUIMasterBase);
  exit(ret);
}

