/*
** This demo shows the use of MUI's menus classes.
*/

#include "demo.h"


/*
** MUI still supports the use of a conventional gadtools NewMenu structure
** to define a menu strip. The address of this structure is passed to a
** MUI_MakeObject(MUIO_MenustripNM,newmenu,0) function which creates a complete
** object tree for this menu and returns a pointer to the menustrip object,
** This pointer should be placed in a windows MUIA_Window_Menustrip attribute
** or in an applications MUIA_Application_Menustrip attribute, in which case
** all windows will inherit the menu. To get direct access to the menu item
** objects of such an auto generated tree, you can use the MUIM_FindUserData
** method.
**
** You can of course create the menu object tree yourself by defining
** a menu strip object with some menu object children with some menu
** item object children with optionally some more menu item object
** children. In this case, use the MUI_MakeObject(MUIO_Menuitem,...)
** call to create the menu items if possible.
**
** The main program shows some different methods how to create items and
** how to react on menu items. Choose the one you like best, and don't
** forget the MUIM_CallHook method if you dislike switch/case in the
** main loop!
*/

#define RB CHECKIT
#define TG CHECKIT|MENUTOGGLE

enum { MEN_PROJECT=1,MEN_ABOUT,MEN_QUIT,MEN_EDIT,MEN_CUT,MEN_COPY,MEN_PASTE,MEN_SETTINGS,MEN_HARDWARE,MEN_A1000,MEN_A2000,MEN_A3000,MEN_A4000,MEN_SOFTWARE,MEN_MUI,MEN_MFR,MEN_MWB,MEN_DFA,ID_ADD,ID_REM,ID_RADIO,ID_TOG };

static struct NewMenu MenuData1[] =
{
	{ NM_TITLE, "Project"                  , 0 ,0 ,0             ,(APTR)MEN_PROJECT  },
	{ NM_ITEM ,  "About"                   ,"?",0 ,0             ,(APTR)MEN_ABOUT    },
	{ NM_ITEM ,  NM_BARLABEL               , 0 ,0 ,0             ,(APTR)0            },
	{ NM_ITEM ,  "Quit"                    ,"Q",0 ,0             ,(APTR)MEN_QUIT     },

	{ NM_TITLE, "Edit"                     , 0 ,0 ,0             ,(APTR)MEN_EDIT     },
	{ NM_ITEM ,  "Cut"                     ,"X",0 ,0             ,(APTR)MEN_CUT      },
	{ NM_ITEM ,  "Copy"                    ,"C",0 ,0             ,(APTR)MEN_COPY     },
	{ NM_ITEM ,  "Paste"                   ,"V",0 ,0             ,(APTR)MEN_PASTE    },

	{ NM_TITLE, "Settings"                 , 0 ,NM_MENUDISABLED,0,(APTR)MEN_SETTINGS },
	{ NM_ITEM ,  "Hardware"                , 0 ,NM_ITEMDISABLED,0,(APTR)MEN_HARDWARE },
	{ NM_SUB  ,   "A1000"                  ,"1",RB|CHECKED,2|4|8 ,(APTR)MEN_A1000    },
	{ NM_SUB  ,   "A2000"                  ,"2",RB,1|4|8         ,(APTR)MEN_A2000    },
	{ NM_SUB  ,   "A3000"                  ,"3",RB,1|2|8         ,(APTR)MEN_A3000    },
	{ NM_SUB  ,   "A4000"                  ,"4",RB,1|2|4         ,(APTR)MEN_A4000    },
	{ NM_ITEM ,  "Software"                , 0 ,NM_ITEMDISABLED,0,(APTR)MEN_SOFTWARE },
	{ NM_SUB  ,   "MUI"                    ,"M",TG,0             ,(APTR)MEN_MUI      },
	{ NM_SUB  ,   "MFR"                    ,"F",TG,0             ,(APTR)MEN_MFR      },
	{ NM_SUB  ,   "MagicWB"                ,"W",TG,0             ,(APTR)MEN_MWB      },
	{ NM_SUB  ,   "DFA"                    ,"D",TG,0             ,(APTR)MEN_DFA      },

	{ NM_END,NULL,0,0,0,(APTR)0 },
};


int main(int argc,char *argv[])
{
	APTR app,win1,cm1,cm2,cm3,cm4,radio,cy1,cy2,cy3,menuset,menuedit,itemhw,itemsw,strip,menumisc,btadd,btrem,bttog,possi;
	ULONG signals,x;
	BOOL running = TRUE;
	BOOL addedmisc = FALSE;
	static char *hardware[] = { "Amiga 1000", "Amiga 2000", "Amiga 3000", "Amiga 4000", NULL };
	static char *endis1[]   = { "Settings disabled", "Settings enabled", NULL };
	static char *endis2[]   = { "Hardware disabled", "Hardware enabled",  NULL };
	static char *endis3[]   = { "Software disabled", "Software enabled",  NULL };

	init();

	app = ApplicationObject,
/*
		MUIA_Application_Title      , "Menus",
		MUIA_Application_Version    , "$VER: Menus 19.5 (12.02.97)",
		MUIA_Application_Copyright  , "©1994 by Stefan Stuntz",
		MUIA_Application_Author     , "Stefan Stuntz",
		MUIA_Application_Description, "Demonstrate MUI's menu classes.",
		MUIA_Application_Base       , "MENU",
*/
		MUIA_Application_Window, win1 = WindowObject,
			MUIA_Window_Title, "Menus",
			MUIA_Window_ID   , MAKE_ID('M','E','N','1'),
			MUIA_Window_Menustrip, strip = MUI_MakeObject(MUIO_MenustripNM,MenuData1,0),
			WindowContents, VGroup,
				Child, cy1 = MUI_MakeObject(MUIO_Cycle,NULL,endis1),
				Child, HGroup, MUIA_Group_SameSize, TRUE,
					Child, cy2 = MUI_MakeObject(MUIO_Cycle,NULL,endis2),
					Child, cy3 = MUI_MakeObject(MUIO_Cycle,NULL,endis3),
					End,
				Child, MUI_MakeObject(MUIO_HBar,4),
				Child, HGroup,
					Child, HSpace(0),
					Child, radio = MUI_MakeObject(MUIO_Radio,NULL,hardware),
					Child, HSpace(0),
					Child, MUI_MakeObject(MUIO_VBar,1),
					Child, HSpace(0),
					Child, ColGroup(2),
						Child, Label("_MUI"    ), Child, cm1 = MUI_MakeObject(MUIO_Checkmark,"_MUI"    ),
						Child, Label("M_FR"    ), Child, cm2 = MUI_MakeObject(MUIO_Checkmark,"M_FR"    ),
						Child, Label("Magic_WB"), Child, cm3 = MUI_MakeObject(MUIO_Checkmark,"Magic_WB"),
						Child, Label("_DFA"    ), Child, cm4 = MUI_MakeObject(MUIO_Checkmark,"_DFA"    ),
						End,
					Child, HSpace(0),
					End,
				Child, MUI_MakeObject(MUIO_HBar,4),
				Child, HGroup, MUIA_Group_SameSize, TRUE,
					Child, btadd = MUI_MakeObject(MUIO_Button,"_Add Misc Menu"),
					Child, btrem = MUI_MakeObject(MUIO_Button,"_Remove Misc Menu"),
					Child, bttog = MUI_MakeObject(MUIO_Button,"_Toggle Misc Titles"),
					End,
				End,
			End,
		End;

	menumisc = MenuObject, MUIA_Menu_Title, "Misc",
		MUIA_Family_Child, MUI_MakeObject(MUIO_Menuitem,"Dynamic" ,0,0,0),
		MUIA_Family_Child, MUI_MakeObject(MUIO_Menuitem,"Adding"  ,0,0,0),
		MUIA_Family_Child, MUI_MakeObject(MUIO_Menuitem,"Of"      ,0,0,0),
		MUIA_Family_Child, MUI_MakeObject(MUIO_Menuitem,"Items"   ,0,0,0),
		MUIA_Family_Child, MUI_MakeObject(MUIO_Menuitem,"Is"      ,0,0,0),
		MUIA_Family_Child, possi = MUI_MakeObject(MUIO_Menuitem,"Possible",0,CHECKIT|CHECKED|MENUTOGGLE,0),
		End;

	if (!app || !menumisc)
		fail(app,"Failed to create Application.");

	menuset  = (APTR)DoMethod(strip,MUIM_FindUData,MEN_SETTINGS);
	menuedit = (APTR)DoMethod(strip,MUIM_FindUData,MEN_EDIT);
	itemhw   = (APTR)DoMethod(strip,MUIM_FindUData,MEN_HARDWARE);
	itemsw   = (APTR)DoMethod(strip,MUIM_FindUData,MEN_SOFTWARE);

	DoMethod(cy1,MUIM_Notify,MUIA_Cycle_Active,MUIV_EveryTime,menuset,3,MUIM_Set,MUIA_Menu_Enabled    ,MUIV_TriggerValue);
	DoMethod(cy2,MUIM_Notify,MUIA_Cycle_Active,MUIV_EveryTime,itemhw ,3,MUIM_Set,MUIA_Menuitem_Enabled,MUIV_TriggerValue);
	DoMethod(cy3,MUIM_Notify,MUIA_Cycle_Active,MUIV_EveryTime,itemsw ,3,MUIM_Set,MUIA_Menuitem_Enabled,MUIV_TriggerValue);

	DoMethod(btadd,MUIM_Notify,MUIA_Pressed,FALSE,app,2,MUIM_Application_ReturnID,ID_ADD);
	DoMethod(btrem,MUIM_Notify,MUIA_Pressed,FALSE,app,2,MUIM_Application_ReturnID,ID_REM);
	DoMethod(bttog,MUIM_Notify,MUIA_Pressed,FALSE,app,2,MUIM_Application_ReturnID,ID_TOG);

	DoMethod(app,MUIM_Notify,MUIA_Application_MenuAction,MEN_A1000,radio,3,MUIM_Set,MUIA_Radio_Active,0);
	DoMethod(app,MUIM_Notify,MUIA_Application_MenuAction,MEN_A2000,radio,3,MUIM_Set,MUIA_Radio_Active,1);
	DoMethod(app,MUIM_Notify,MUIA_Application_MenuAction,MEN_A3000,radio,3,MUIM_Set,MUIA_Radio_Active,2);
	DoMethod(app,MUIM_Notify,MUIA_Application_MenuAction,MEN_A4000,radio,3,MUIM_Set,MUIA_Radio_Active,3);

	DoMethod(cm1,MUIM_Notify,MUIA_Selected,MUIV_EveryTime,strip,4,MUIM_SetUData,MEN_MUI,MUIA_Menuitem_Checked,MUIV_TriggerValue);
	DoMethod(cm2,MUIM_Notify,MUIA_Selected,MUIV_EveryTime,strip,4,MUIM_SetUData,MEN_MFR,MUIA_Menuitem_Checked,MUIV_TriggerValue);
	DoMethod(cm3,MUIM_Notify,MUIA_Selected,MUIV_EveryTime,strip,4,MUIM_SetUData,MEN_MWB,MUIA_Menuitem_Checked,MUIV_TriggerValue);
	DoMethod(cm4,MUIM_Notify,MUIA_Selected,MUIV_EveryTime,strip,4,MUIM_SetUData,MEN_DFA,MUIA_Menuitem_Checked,MUIV_TriggerValue);

	DoMethod(radio,MUIM_Notify,MUIA_Radio_Active,MUIV_EveryTime,app,2,MUIM_Application_ReturnID,ID_RADIO);

	DoMethod(win1,MUIM_Notify,MUIA_Window_CloseRequest,TRUE,app,2,MUIM_Application_ReturnID,MUIV_Application_ReturnID_Quit);

	set(btrem,MUIA_Disabled,TRUE);

	set(cm1,MUIA_UserData,MEN_MUI);
	set(cm2,MUIA_UserData,MEN_MFR);
	set(cm3,MUIA_UserData,MEN_MWB);
	set(cm4,MUIA_UserData,MEN_DFA);


/*
** Input loop...
*/

	set(win1,MUIA_Window_Open,TRUE);

	while (running)
	{
		ULONG id = DoMethod(app,MUIM_Application_Input,&signals);

		switch (id)
		{
			case MEN_QUIT:
			case MUIV_Application_ReturnID_Quit:
				running = FALSE;
				break;

			case MEN_ABOUT:
				MUI_Request(app,win1,0,NULL,"OK","Some little about window.");
				break;

			case ID_ADD:
				set(btadd,MUIA_Disabled,TRUE);
				set(btrem,MUIA_Disabled,FALSE);
				DoMethod(strip,MUIM_Family_Insert,menumisc,menuedit);
				addedmisc = TRUE;
				break;

			case ID_REM:
				set(btadd,MUIA_Disabled,FALSE);
				set(btrem,MUIA_Disabled,TRUE);
				DoMethod(strip,MUIM_Family_Remove,menumisc);
				addedmisc = FALSE;
				break;

			case ID_TOG:
			{
				char *x;

				get(menumisc,MUIA_Menu_Title,&x);
				if (strcmp(x,"Misc"))
					set(menumisc,MUIA_Menu_Title,"Misc");
				else
					set(menumisc,MUIA_Menu_Title,"Miscodil");

				get(possi,MUIA_Menuitem_Title,&x);
				if (strcmp(x,"Possible"))
					set(possi,MUIA_Menuitem_Title,"Possible");
				else
					set(possi,MUIA_Menuitem_Title,"elbissoP");

				break;
			}

			case MEN_MUI:
			case MEN_MFR:
			case MEN_MWB:
			case MEN_DFA:
				DoMethod(strip,MUIM_GetUData,id,MUIA_Menuitem_Checked,&x);
				DoMethod(win1,MUIM_SetUData,id,MUIA_Selected,x);
				break;

			case ID_RADIO:
				get(radio,MUIA_Radio_Active,&x);
				DoMethod(itemhw,MUIM_SetUData,MEN_A1000,MUIA_Menuitem_Checked,x==0);
				DoMethod(itemhw,MUIM_SetUData,MEN_A2000,MUIA_Menuitem_Checked,x==1);
				DoMethod(itemhw,MUIM_SetUData,MEN_A3000,MUIA_Menuitem_Checked,x==2);
				DoMethod(itemhw,MUIM_SetUData,MEN_A4000,MUIA_Menuitem_Checked,x==3);
				break;
		}
		if (running && signals) Wait(signals);
	}

	set(win1,MUIA_Window_Open,FALSE);

/*
** Shut down...
*/

	if (!addedmisc)
		MUI_DisposeObject(menumisc);

	fail(app,NULL);
}
