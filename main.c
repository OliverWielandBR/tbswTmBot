#include <bur/plctypes.h>
#include <string.h>
#include <stdlib.h>
#include <tbswJsmn.h>
#include <AsHttp.h>

#ifdef __cplusplus
	extern "C"
	{
#endif
#include "tbswTmBot.h"
#ifdef __cplusplus
	};
#endif

void tbswTmBot(struct tbswTmBot* inst)
{
	int ii;
	httpsClient_typ* pClient = &inst->internal.fbHttpsClient;
	tmTelegram* pTelegram = &inst->internal.Telegram;
	tmConfiguration* pConfig = &inst->internal.Configuration;
	httpRequestHeader_t* pReqHeader = &inst->internal.httpRequestHeader;
	
	if (!inst->Enable)
	{
		// ToDo: Clean up instance
		inst->Status = ERR_FUB_ENABLE_FALSE;
		inst->Error = 0;
		inst->internal.Step = tmStep_INIT;
		return;
	}
	
	switch (inst->internal.Step)
	{
		case tmStep_INIT:
			if (inst->Enable && !inst->Error)
			{
				if (!strlen(inst->Configuration->Token))
				{
					inst->Status = tmERR_NO_TOKEN;
					inst->Error = 1;
				}
				else if (!strlen(inst->Configuration->ChatID))
				{
					inst->Status = tmERR_NO_CHATID;
					inst->Error = 1;
				}
					else
					{
						memcpy(pConfig->baseURI, inst->Configuration->baseURI, sizeof(pConfig->baseURI));
						memcpy(pConfig->ChatID, inst->Configuration->ChatID, sizeof(pConfig->ChatID));
						memcpy(pConfig->Host, inst->Configuration->Host, sizeof(pConfig->Host));
						pConfig->HostPort = inst->Configuration->HostPort;
						memcpy(pConfig->Token, inst->Configuration->Token, sizeof(pConfig->Token));
						inst->Active = 1;
						inst->Status = ERR_OK;
						inst->internal.Step = tmStep_IDLE;
					}
			}
			break;
				
		case tmStep_IDLE:
			if (inst->CmdSendMessage)
			{
				inst->internal.currentCmd = tmSendMessage;
				strcpy(pTelegram->Method, "sendMessage");
				inst->Status = ERR_FUB_BUSY;
				
				inst->internal.strToUtf8.enable = 1;
				inst->internal.strToUtf8.pSrc = (UDINT)inst->Message;
				inst->internal.strToUtf8.pDest = (UDINT)inst->internal.strUtf8;
				inst->internal.strToUtf8.destSize = sizeof(inst->internal.strUtf8);
				httpStringToUtf8(&inst->internal.strToUtf8);
				
				strcpy(pTelegram->MessageObject, "{\"chat_id\":");
				strcat(pTelegram->MessageObject, pConfig->ChatID);
				strcat(pTelegram->MessageObject, ", \"text\":\"");
				strcat(pTelegram->MessageObject, inst->internal.strUtf8);
				strcat(pTelegram->MessageObject, "\"}");
				
				inst->internal.Step = tmStep_PREPAREREQUEST;
			}
			else if (inst->CmdGetUpdates)
			{
				inst->internal.currentCmd = tmGetUpdates;
				strcpy(pTelegram->Method, "getUpdates");
				inst->Status = ERR_FUB_BUSY;
					
				strcpy(pTelegram->MessageObject, "{\"offset\":");
				brsitoa(pTelegram->UpdateId, (UDINT)inst->internal.str);
				strcat(pTelegram->MessageObject, inst->internal.str);
				strcat(pTelegram->MessageObject, "}");
					
				inst->internal.Step = tmStep_PREPAREREQUEST;
			}
			break;
		
		case tmStep_PREPAREREQUEST:
			strcpy(pTelegram->URI, pConfig->baseURI);
			strcat(pTelegram->URI, pConfig->Token);
			strcat(pTelegram->URI, "/");
			strcat(pTelegram->URI, pTelegram->Method);
			
			strcpy(pReqHeader->contentType, "application/json");
			strcpy(pReqHeader->protocol, "HTTP/1.1");
			strcpy(pReqHeader->host, pConfig->Host);
			strcpy(pReqHeader->connection, "Close");

			pClient->enable = 1;
			pClient->send = 1;
			pClient->option = httpOPTION_HTTP_11;
			pClient->sslCfgIdent = 0;
			pClient->pHost =  (UDINT)pConfig->Host;
			pClient->hostPort = pConfig->HostPort;
			pClient->method = httpMETHOD_POST;
			pClient->pUri = (UDINT)pTelegram->URI;
			pClient->pRequestHeader = (UDINT)pReqHeader;
			pClient->pRequestData = (UDINT)pTelegram->MessageObject;
			pClient->requestDataLen = strlen(pTelegram->MessageObject);
			pClient->pResponseHeader = (UDINT)&inst->internal.httpResponseHeader;
			pClient->pResponseData = (UDINT)inst->internal.ResponseData;
			pClient->responseDataSize = sizeof(inst->internal.ResponseData);
			pClient->pStatistics = (UDINT)&inst->internal.httpStatistics;
			
			/////////////////////////////////////////////////////////////////////
			// httpsClient() will be called cyclically at the end of the function
			/////////////////////////////////////////////////////////////////////
			
			inst->internal.Step = tmStep_SENDREQUEST;
			break;
		
		case tmStep_SENDREQUEST:
			if (pClient->status == ERR_OK)
			{
				if(pClient->phase == httpPHASE_RECEIVED)
				{
					if(inst->internal.currentCmd == tmGetUpdates)
					{
						//					Step := tmStep_PARSE_UPDATE_RESPONSE;
						if(pClient->responseDataLen > 0)
						{
							tbsw_JsonInit(&inst->internal.parser);
							inst->internal.cnt_tokens = tbsw_JsonParse(&inst->internal.parser, pClient->pResponseData, pClient->responseDataLen, (UDINT)inst->internal.tokens, TM_MAX_MESSAGES);
							if(inst->internal.cnt_tokens > 0)
							{
								inst->internal.Step = tmStep_PARSE_UPDATE_RESPONSE;
							}
							else
							{
								inst->Status = inst->internal.cnt_tokens;
								inst->internal.Step = tmStep_ERROR;
							}
						}
						else
						{
							inst->Status = 1;
							inst->internal.Step = tmStep_ERROR;
						}
					}
					else
					{
						inst->Status = ERR_OK;
						inst->internal.Step = tmStep_COMMAND_DONE;
					}
				}
			}
			else if (pClient->status != ERR_FUB_BUSY)
			{
				inst->Status = pClient->status;
				inst->internal.Step = tmStep_ERROR;
			}
		
			break;
		
		case tmStep_PARSE_UPDATE_RESPONSE:
			inst->MsgCounter = 0;
			for(ii=0; ii < inst->internal.cnt_tokens; ii++)
			{
				switch(inst->internal.tokens[ii].type)
				{
					case JSMN_OBJECT:
						// do nothing
						break;
						
					case JSMN_ARRAY:
						// do nothing
						break;
						
					case JSMN_STRING:
						memset(inst->internal.str, 0, sizeof(inst->internal.str));
						memcpy(inst->internal.str, (char *)(pClient->pResponseData + inst->internal.tokens[ii].start), inst->internal.tokens[ii].end - inst->internal.tokens[ii].start);
						if (inst->internal.tokens[ii].size > 0)
						{
							// this is a name token
							inst->internal.isUpdateID = 0;
							inst->internal.isID = 0;
							inst->internal.isMsg = 0;
											
							if (!strcmp(inst->internal.str, "id"))
							{
								inst->internal.isID = 1;
							}
							else if (!strcmp(inst->internal.str, "update_id"))
							{
								inst->internal.isUpdateID = 1;
							}
								else if (!strcmp(inst->internal.str, "text"))
								{	
									inst->internal.isMsg = 1;
								}
						}
						else
						{
							if (inst->internal.isMsg)
							{
								strcpy(inst->pMessageList[inst->MsgCounter].Message, inst->internal.str);
								inst->pMessageList[inst->MsgCounter].ChatId = inst->internal.ChatId;
								inst->MsgCounter++;
							}
						}
							
						break;
						
					case JSMN_PRIMITIVE:
						memset(inst->internal.str, 0, sizeof(inst->internal.str));
						memcpy(inst->internal.str, (char *)(pClient->pResponseData + inst->internal.tokens[ii].start), inst->internal.tokens[ii].end - inst->internal.tokens[ii].start);
							
						if (inst->internal.isID)
						{
							inst->internal.ChatId = atoi(inst->internal.str);
						}
						else if (inst->internal.isUpdateID)
						{
							pTelegram->UpdateId = atoi(inst->internal.str) + 1;
						}
	
						break;
			
					case JSMN_UNDEFINED:
						break;
				}
			}
		
			inst->internal.Step = tmStep_COMMAND_DONE;
			inst->Status = ERR_OK;
			inst->CommandDone = 1;
		
			break;
		
		case tmStep_ERROR:
			inst->Error = 1;
			if (!inst->CmdGetUpdates && !inst->CmdSendMessage)
			{
				inst->Error = 0;
				inst->internal.Step = tmStep_IDLE;
			}
			break;
		case tmStep_COMMAND_DONE:
			inst->CommandBusy = 0;
			inst->CommandDone = 1;

			if (!inst->CmdGetUpdates && !inst->CmdSendMessage)
			{
				inst->CommandDone = 0;
				inst->internal.Step = tmStep_IDLE;
			}
			break;
	}

	httpsClient(pClient);
	pClient->send = 0;
	
	inst->CommandBusy = (inst->Status == ERR_FUB_BUSY);
	
		
	
}
