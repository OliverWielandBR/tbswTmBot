
FUNCTION_BLOCK tbswTmBot
	VAR_INPUT
		Enable : BOOL;
		Configuration : REFERENCE TO tmConfiguration;
		CmdSendMessage : BOOL;
		CmdGetUpdates : BOOL;
		Message : STRING[799];
		pMessageList : REFERENCE TO tmMessageList;
	END_VAR
	VAR_OUTPUT
		Active : BOOL;
		Error : BOOL;
		Status : UINT;
		CommandBusy : BOOL;
		CommandDone : BOOL;
		MsgCounter : UDINT;
	END_VAR
	VAR
		internal : tmInternal;
	END_VAR
END_FUNCTION_BLOCK
