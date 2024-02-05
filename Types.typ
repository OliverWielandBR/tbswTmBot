
TYPE
	tmConfiguration : 	STRUCT 
		Host : {REDUND_UNREPLICABLE} STRING[79] := 'api.telegram.org';
		HostPort : {REDUND_UNREPLICABLE} UINT := 443;
		Token : {REDUND_UNREPLICABLE} STRING[79];
		baseURI : {REDUND_UNREPLICABLE} STRING[79] := '/bot';
		ChatID : {REDUND_UNREPLICABLE} STRING[79];
	END_STRUCT;
	tmInternal : 	STRUCT 
		Step : tmStep;
		currentCmd : tmCmd;
		fbHttpsClient : httpsClient;
		httpRequestHeader : httpRequestHeader_t;
		httpResponseHeader : httpResponseHeader_t;
		httpStatistics : httpStatistics_t;
		Telegram : tmTelegram;
		Configuration : tmConfiguration;
		ResponseData : STRING[999];
		parser : jsmn_parser;
		tokens : ARRAY[0..TM_MAX_MESSAGES]OF jsmntok_t;
		cnt_tokens : DINT;
		str : STRING[999];
		strUtf8 : STRING[999];
		isUpdateID : BOOL;
		isID : BOOL;
		isMsg : BOOL;
		ChatId : UDINT;
		strToUtf8 : httpStringToUtf8;
		utf8ToStr : httpUtf8ToString;
		New_Member : USINT;
	END_STRUCT;
	tmCmd : 
		(
		tmNoCommand,
		tmSendMessage,
		tmGetUpdates
		);
	tmTelegram : 	STRUCT 
		Method : STRING[20];
		MessageObject : STRING[999];
		URI : STRING[99];
		UpdateId : UDINT;
	END_STRUCT;
	tmMessageList : 	STRUCT 
		Message : {REDUND_UNREPLICABLE} STRING[49];
		ChatId : {REDUND_UNREPLICABLE} DINT;
	END_STRUCT;
	tmStep : 
		(
		tmStep_INIT,
		tmStep_IDLE,
		tmStep_PREPAREREQUEST,
		tmStep_SENDREQUEST,
		tmStep_PARSE_UPDATE_RESPONSE,
		tmStep_COMMAND_DONE,
		tmStep_ERROR
		);
END_TYPE
