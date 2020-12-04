#include "includes.h"
//#include "cJSON.h"
enum {WEINA=2,WEISHENG=1}sensor_type;

#define REC_COM_LEN 40  //GET数据的命令长度

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
		
		ptFindResult = strstr(ptStrStart,"ADMO");
		if(ptFindResult != NULL)   //GetMeterCommand 命令解析
		{
			xEventGroupSetBits(xGetCmdEventGroup, GET_CMD_CMD_RESPONSE);
//			printf("xGetCmdEventGroup = %d \r\n",(int)xEventGroupGetBits(xGetCmdEventGroup));
			xEventGroupClearBits(xGetCmdEventGroup,GET_CMD_STUP_REQUIRE);
//			printf("xGetCmdEventGroup = %d \r\n",(int)xEventGroupGetBits(xGetCmdEventGroup));
			json=cJSON_Parse(ptJson); //获取整个Json大的句柄
			if(json != NULL)
			{
				test_arr = cJSON_GetObjectItem(json,"message");
				
				item = cJSON_GetObjectItem(test_arr,"meterId"); 					//meterId": "TZ00000525", length should be 10
				memcpy(Value,item->valuestring,strlen(item->valuestring));
				strncpy(RechargePacket.meterNumer ,Value,strlen(Value));
				memset(Value,0,REC_COM_LEN);
				
				item = cJSON_GetObjectItem(test_arr,"customerId"); 					//"customerId": "212137868",length should be 7
				memcpy(Value,item->valuestring,strlen(item->valuestring));
				strncpy(RechargePacket.CUSTOMER_ID ,Value,strlen(Value));
				memset(Value,0,REC_COM_LEN);
				
//				item = cJSON_GetObjectItem(test_arr,"command"); 					//"command": "ADMO",
//				memcpy(Value,item->valuestring,strlen(item->valuestring));
//				strncpy(RechargePacket. ,Value,strlen(Value));
//				memset(Value,0,REC_COM_LEN);
				
				item = cJSON_GetObjectItem(test_arr,"cardID");	//"cardID": "test123",
				memcpy(Value,item->valuestring,strlen(item->valuestring));
				strncpy(RechargePacket.CARD_ID ,Value,strlen(Value));
				memset(Value,0,REC_COM_LEN);
				
				item = cJSON_GetObjectItem(test_arr,"rechargeAmount"); 	// "rechargeAmount": 1500,
				JsonValue = item->valuedouble;
				sprintf(RechargePacket.rechargeAmountIn,"%.2f",JsonValue);	
				
				
				item = cJSON_GetObjectItem(test_arr,"unitPrice"); 	//"unitPrice": 1000,
				JsonValue = item->valuedouble ;
				sprintf(RechargePacket.unitPrice ,"%.2f",JsonValue);

				
				item = cJSON_GetObjectItem(test_arr,"lpgDensity"); 						//"lpgDensity": 3.4,
				JsonValue = item->valuedouble ;
				sprintf(RechargePacket.LPGDensity  ,"%.2f",JsonValue);
				
				item = cJSON_GetObjectItem(test_arr,"addOrSub");	//"addOrSub": "ADD",
				memcpy(Value,item->valuestring,strlen(item->valuestring));
				strncpy(RechargePacket.ADDORSUB  ,Value,strlen(Value));
				memset(Value,0,REC_COM_LEN);
			
				cJSON_Delete(json);
			}
		}	 
		
	}
	
}