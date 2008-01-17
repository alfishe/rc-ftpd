struct ApplicationData
{
	struct MsgPort *ARexxPort;
	BPTR RxInOut;
	ULONG RunningScripts;
	struct MUI_InputHandlerNode ARexxINode;
};

struct MUIP_Application_RunScript
{
	ULONG MethodID;
	STRPTR Script;
	STRPTR Args;
};

ULONG ApplicationDispatcher (REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	ULONG result = NULL;
	struct ApplicationData *data = (ApplicationData *)INST_DATA(cl, obj);
	switch(msg->MethodID)
	{
		case OM_DISPOSE:
		{
			while(data->RunningScripts)
			{
				if(!DoMethod(obj, MUIM_Application_ScriptDone) && data->RunningScripts)
					WaitPort(data->ARexxPort);
			}

			if(data->ARexxPort)
				DeleteMsgPort(data->ARexxPort);
			if(data->RxInOut)
				Close(data->RxInOut);

			result = DoSuperMethodA(cl, obj, msg);
		}
		break;

		case MUIM_Application_RunScript:
		{
			struct MUIP_Application_RunScript *rmsg = (MUIP_Application_RunScript *)msg;

			if(!data->ARexxPort)
			{
				data->ARexxPort = CreateMsgPort();
				data->ARexxINode.ihn_Object	= obj;
				data->ARexxINode.ihn_Signals	= 1 << data->ARexxPort->mp_SigBit;
				data->ARexxINode.ihn_Flags		= 0L;
				data->ARexxINode.ihn_Method	= MUIM_Application_ScriptDone;
			}

			if(!data->RxInOut)
				data->RxInOut = Open("Con://640/100/" PROGRAM " ARexx output:/Close/Wait/Auto/InActive", MODE_READWRITE);

			BPTR lock;
			if(data->ARexxPort && (lock = Lock(rmsg->Script, ACCESS_READ)))
			{
				STRPTR script;
				if(script = new UBYTE [512 + strlen(rmsg->Args) + 1])
				{
					NameFromLock(lock, script, 512);
					strcat(script, " ");
					strcat(script, rmsg->Args);

					struct MsgPort *rexxport;
					if(rexxport = FindPort("REXX"))
					{
						STRPTR myarexxport;
						get(obj, MUIA_Application_Base, &myarexxport);

						struct RexxMsg *rxmsg;
						rxmsg = CreateRexxMsg(data->ARexxPort, NULL, myarexxport);
						rxmsg->rm_Action = RXCOMM;
						rxmsg->rm_Stdin = rxmsg->rm_Stdout = data->RxInOut;
						rxmsg->rm_Args[0] = CreateArgstring(script, strlen(script));
						PutMsg(rexxport, &rxmsg->rm_Node);

						if(data->RunningScripts++ == 0)
							DoMethod(obj, MUIM_Application_AddInputHandler, &data->ARexxINode);
					}
					delete script;
				}
				UnLock(lock);
			}
		}
		break;

		case MUIM_Application_ScriptDone:
		{
			struct RexxMsg *rxmsg;
			while(rxmsg = (struct RexxMsg *)GetMsg(data->ARexxPort))
			{
				if(!rxmsg->rm_Result1 && rxmsg->rm_Result2)
					DeleteArgstring((STRPTR)rxmsg->rm_Result2);
				DeleteArgstring(rxmsg->rm_Args[0]);
				DeleteRexxMsg(rxmsg);

				if(--data->RunningScripts == 0)
					DoMethod(obj, MUIM_Application_RemInputHandler, &data->ARexxINode);

				result = TRUE;
			}
		}
		break;

		default:
			result = DoSuperMethodA(cl, obj, msg);
		break;
	}
	return(result);
}
