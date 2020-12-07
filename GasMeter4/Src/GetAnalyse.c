#include "includes.h"
//#include "cJSON.h"
enum {WEINA=2,WEISHENG=1}sensor_type;

#define REC_COM_LEN 100  //GET数据的命令长度

EventGroupHandle_t xGetCmdEventGroup = NULL;

void GetCmdEventGroupCreat (void)
{
		/* 创建事件标志组 */
	xGetCmdEventGroup = xEventGroupCreate();
	
	if(xGetCmdEventGroup == NULL)
	{
			/* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
	}
}


//void StrToHex(byte *pbDest, char *pszSrc, int nLen)
//{
// char h1, h2;
// byte s1, s2;
// for (int i = 0; i < nLen; i++)
// {
//  h1 = pszSrc[2 * i];
//  h2 = pszSrc[2 * i + 1];
// 
//  s1 = toupper(h1) - 0x30;
//  if (s1 > 9)
//   s1 -= 7;
// 
//  s2 = toupper(h2) - 0x30;
//  if (s2 > 9)
//   s2 -= 7;
// 
//  pbDest[i] = s1 * 16 + s2;
// }
//}

//分析GET的数据内容
void GetAnalyse(uint8_t *ptRecData)
{
	char *ptFindResult ;
	char *ptJson;
	char *ptStrStart;
	cJSON *json;
	cJSON *item;
	cJSON* test_arr ;
	cJSON *object;
	char Value[REC_COM_LEN];
	float JsonValue;
	memset(Value,0,REC_COM_LEN);
//	int test_arr; 
	ptStrStart = (char *)ptRecData;
	ptFindResult = strstr(ptStrStart,"200");
	if(ptFindResult != NULL)
	{
//		printf("111\r\n");
		ptJson = strstr(ptStrStart,"{");
		
		ptFindResult = NULL;
		ptFindResult = strstr(ptStrStart,"STUP");
		if(ptFindResult != NULL)   //GetMeterSetting 命令解析
		{
			xEventGroupSetBits(xGetCmdEventGroup, GET_CMD_STUP_RESPONSE);
//			printf("xGetCmdEventGroup = %d \r\n",(int)xEventGroupGetBits(xGetCmdEventGroup));
			xEventGroupClearBits(xGetCmdEventGroup,GET_CMD_STUP_REQUIRE);
//			printf("xGetCmdEventGroup = %d \r\n",(int)xEventGroupGetBits(xGetCmdEventGroup));
			json=cJSON_Parse(ptJson); //获取整个Json大的句柄
			if(json != NULL)
			{
				test_arr = cJSON_GetObjectItem(json,"message");
				
				item = cJSON_GetObjectItem(test_arr,"serverIpAddress"); 					//IP地址
				memcpy(Value,item->valuestring,strlen(item->valuestring));
				strncpy(CONFIG_GPRS.Server_IP,Value,strlen(Value));
				memset(Value,0,REC_COM_LEN);
				
				item = cJSON_GetObjectItem(test_arr,"serverPort"); 					//端口号
				JsonValue = item->valueint;
				sprintf(CONFIG_GPRS.Socket_Port,"%d",(uint32_t)JsonValue);	
				IsSaveCONFIG_GPRS = true;		
				CONFIG_GPRS_Write();
//				CONFIG_GPRS_READ();			
				
				item = cJSON_GetObjectItem(test_arr,"dataUploadPeriod"); 					//上传周期
				JsonValue = item->valueint;
				CONFIG_Meter.UpDuty = (uint32_t) JsonValue;
				
				item = cJSON_GetObjectItem(test_arr,"warningLowBatteryVoltage");	//报警电压阈值
				memcpy(Value,item->valuestring,strlen(item->valuestring));
				sscanf(Value,"%f",&CONFIG_Meter.LowBattery);
				memset(Value,0,REC_COM_LEN);
				
				item = cJSON_GetObjectItem(test_arr,"warningLowCreditBalance"); 	//卡内余额报敬阈值
				JsonValue = item->valuedouble;
				CONFIG_Meter.LowCredit = JsonValue;
				
				item = cJSON_GetObjectItem(test_arr,"warningLowGasVolumeAlarm"); 	//罐内气体余量阈值
				memcpy(Value,item->valuestring,strlen(item->valuestring));
				sscanf(Value,"%f",&CONFIG_Meter.LowGasVolume);
				memset(Value,0,REC_COM_LEN);
				
				item = cJSON_GetObjectItem(test_arr,"meterCurrency"); 						//币种 单位
				memcpy(Value,item->valuestring,strlen(item->valuestring));
				strncpy(CONFIG_Meter.CURRENCY,Value,3);
				memset(Value,0,REC_COM_LEN);
				
				IsSaveCONFIG_Meter = true;
				CONFIG_Meter_Write();
//				CONFIG_Meter_Write(); //保存表信息 
				item = cJSON_GetObjectItem(test_arr,"sensorType"); 						//传感器类型
				JsonValue = item->valueint;
				if(JsonValue == WEINA)
				{
					item = cJSON_GetObjectItem(test_arr,"sensorSlope");	//斜率
					memcpy(Value,item->valuestring,strlen(item->valuestring));
					sscanf(Value,"%f",&REAL_DATA_CALIBRATION.slope);
					memset(Value,0,REC_COM_LEN);
					
					item = cJSON_GetObjectItem(test_arr,"sensorIntercept"); 	//原点
					JsonValue = item->valueint;
					REAL_DATA_CALIBRATION.zero = JsonValue;
					
					IsSaveCALIBRATION = true;
					CALIBRATION_Voltage_Write();
				}
				else if(JsonValue == WEISHENG)
				{
					item = cJSON_GetObjectItem(test_arr,"sensorSlope");	//斜率
					memcpy(Value,item->valuestring,strlen(item->valuestring));
					sscanf(Value,"%f",REAL_Flow_CALIBRATION.ABCDEF);
					memset(Value,0,REC_COM_LEN);
					
					item = cJSON_GetObjectItem(test_arr,"sensorIntercept"); 	//原点
					JsonValue = item->valueint;
					REAL_Flow_CALIBRATION.ParamNumber = JsonValue;		
					IsSaveFlowCALIBRATION = true;
					CALIBRATION_Flow_Write();					
				}
			
				
//				CALIBRATION_Voltage_Write();  //保存斜率与原点
				
				
				cJSON_Delete(json);
			}
		}	
/*************************************************************************************************************/
//{
//	"statusCode":"CommandFound-200",
//	"message":
//	{
//		"commandId":"R5C4V8qUZdxuU4a",
//		"credit":"ADD",
//		"meterID":"TZ00001911",
//		"amount":"1234.00",
//		"lpgDensity":2.525,
//		"paymentDate":"2020-11-03T10:04:00.000Z",
//		"unitPrice":"42226.00",
//		"command":"ADMO",
//		"cardId":201910150000000500,
//		"customerId":"7066738",
//		"isRead":false
//		}
//}		
/*************************************************************************************************************/		
		ptFindResult = strstr(ptStrStart,"ADMO");
		if(ptFindResult != NULL)   //GetMeterCommand 命令解析
		{
			xEventGroupSetBits(xGetCmdEventGroup, GET_CMD_CMD_RESPONSE);
			xEventGroupClearBits(xGetCmdEventGroup,GET_CMD_CMD_REQUIRE);
			json=cJSON_Parse(ptJson); //获取整个Json大的句柄
			if(json != NULL)
			{
				test_arr = cJSON_GetObjectItem(json,"message");
				
				item = cJSON_GetObjectItem(test_arr,"meterId"); 					//"meterID":"TZ00001911",, length should be 10
				memcpy(Value,item->valuestring,strlen(item->valuestring));
				strncpy(RechargePacket.meterNumer ,Value,strlen(Value));
				memset(Value,0,REC_COM_LEN);
				
				item = cJSON_GetObjectItem(test_arr,"customerId"); 					//"customerId":"7066738",,length should be 7
				memcpy(Value,item->valuestring,strlen(item->valuestring));
				strncpy(RechargePacket.CUSTOMER_ID ,Value,strlen(Value));
				memset(Value,0,REC_COM_LEN);
				
//				item = cJSON_GetObjectItem(test_arr,"command"); 					//"command":"ADMO",
//				memcpy(Value,item->valuestring,strlen(item->valuestring));
//				strncpy(RechargePacket. ,Value,strlen(Value));
//				memset(Value,0,REC_COM_LEN);
				
				item = cJSON_GetObjectItem(test_arr,"cardId");	//"cardId":201910150000000500,
				memcpy(Value,item->valuestring,strlen(item->valuestring));
				strncpy(RechargePacket.CARD_ID ,Value,strlen(Value));
				memset(Value,0,REC_COM_LEN);
				
				item = cJSON_GetObjectItem(test_arr,"rechargeAmount"); 	// "rechargeAmount":1234,
				JsonValue = item->valuedouble ;
				sprintf(RechargePacket.rechargeAmountIn ,"%.2f",JsonValue);
//				sprintf(RechargePacket.rechargeAmountIn,"%.2f",JsonValue);	
				
				
				item = cJSON_GetObjectItem(test_arr,"unitPrice"); 	//unitPrice":"42226.00",
				JsonValue = item->valuedouble ;
				sprintf(RechargePacket.unitPrice  ,"%.2f",JsonValue);
//				memset(Value,0,REC_COM_LEN);
//			sprintf(RechargePacket.unitPrice ,"%.2f",JsonValue);

				
				item = cJSON_GetObjectItem(test_arr,"lpgDensity"); 						//"lpgDensity":2.525,
				JsonValue = item->valuedouble ;
				sprintf(RechargePacket.LPGDensity  ,"%.3f",JsonValue);
				
				item = cJSON_GetObjectItem(test_arr,"addOrSub");	//"addOrSub":"ADD",
				memcpy(Value,item->valuestring,strlen(item->valuestring));
				strncpy(RechargePacket.ADDORSUB  ,Value,strlen(Value));
				memset(Value,0,REC_COM_LEN);
			
				cJSON_Delete(json);
			}
			else
			{
				printf("memory fail\r\n");
			}
		}	 
/*************************************************************************************************************/		
//{
//	"statusCode":"FirmwareFound-200",
//	"message":
//	{
//		"meterId":"TZ00001911",
//		"command":"UPDATE",
//		"md5":"96c588a959c4dcbb6b20161427ce7173",
//		"url":"3ekz5ha5ww6c4co0os.bin",
//		"dataTime":"2019-11-11"
//	}
//}		
/*************************************************************************************************************/	
		ptFindResult = strstr(ptStrStart,"UPDATE");
		if(ptFindResult != NULL)   //GetMeterFirmware 命令解析
		{ 
			printf("UPDATE\r\n");
			xEventGroupSetBits(xGetCmdEventGroup, GET_CMD_UPDATA_RESPONSE);
			xEventGroupClearBits(xGetCmdEventGroup,GET_CMD_UPDATA_REQUIRE);
			json=cJSON_Parse(ptJson); //获取整个Json大的句柄
			if(json != NULL)
			{
				test_arr = cJSON_GetObjectItem(json,"message");
				
				item = cJSON_GetObjectItem(test_arr,"md5"); 					//"md5":"96c588a959c4dcbb6b20161427ce7173",
				memcpy(Value,item->valuestring,strlen(item->valuestring));
				StrToHex((byte*)update.MD5CODE,Value,16);
				memset(Value,0,REC_COM_LEN);
				
				
				CONFIG_GPRS_READ();
				item = cJSON_GetObjectItem(test_arr,"url"); 					//"url":"3ekz5ha5ww6c4co0os.bin",
				memcpy(Value,item->valuestring,strlen(item->valuestring));
				memset(update.URL_ADDR,0,128);
				strcat(update.URL_ADDR,CONFIG_GPRS.Server_IP );
				strcat(update.URL_ADDR,":" );
				strcat(update.URL_ADDR,CONFIG_GPRS.Socket_Port );
				strcat(update.URL_ADDR,"/" );
				strcat(update.URL_ADDR,Value );
				memset(Value,0,REC_COM_LEN);
				
				update.BOOTFLAG=0xaa; 
				
				cJSON_Delete(json);
				IsSaveUpdate = true;
			}
			else
			{
				printf("memory fail\r\n");
			}
		}		
/*************************************************************************************************************/			
	}
	
}