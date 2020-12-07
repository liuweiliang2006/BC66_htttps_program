#include "includes.h"

extern bool IsNeedRestart;
QueueHandle_t SendATQueue = NULL; 			//���ڱ�ʾ��ǰ����������ͨѶģ��
SemaphoreHandle_t  Semaphore_Uart_Rec = NULL; //���ź�������ָʾ�����Ƿ���յ���������֡
SemaphoreHandle_t  Semaphore_AT_Response = NULL;
EventGroupHandle_t xCreatedEventGroup = NULL;

EventGroupHandle_t BC66_AT_EventGroup = NULL;
EventGroupHandle_t BC66_AT_Deal = NULL;

uint8_t AT_work_Flag = 0; //�� �����ʾ�������ڽ���ATָ��Ľ�����������λΪ1ʱ���ڿ�������Ĺ��Ӻ����пɽ���ι��������HTTPS��ATָ����᳤ܻ�ڲ����ظ��������ڼ�Ӧ����ι��
//#define SEVER_URL "https://ateei9d448.execute-api.eu-west-1.amazonaws.com/"
//#define SEVER_VERSION "testing/"
#define SEVER_URL "https://test.kop4.com/"
#define SEVER_VERSION "meter/"
#define X_LOCALE "X-Locale:dev"
#define X_API_KEY "X-Api-Key: Qg4EIGj5piiDNVb4P7Th8cop60TJU2xmtIKwxo20"
#define HOST "Host:test.kop4.com"


#define URL_LENGTH_ARRAY 24 //at+qhttpurl=26 ָ��������±�
#define URL_ADDR_ARRAY 25 	//URL�ĵ�ַ ָ��������±�
#define POST_LENGTH_ARRAY 26 //at+qhttppost=152 ָ��������±�
#define POST_DATA_ARRAY 27 //POST���ݵĳ��� ָ��������±�

#define M26GETCOMMANDLEN 4
#define M26POSTCOMMANDLEN 5
#define BC66POSTCOMMANDLEN 5
#define ANALYSIS_PRINT 1


stru_P4_command_t Send_AT_cmd[]={
/***************************************************M26 https ATָ�����������test.kop4.com,�������Զ���ͷ��************************************/
	          //  u8CmdNum  	SendCommand																													pFun
/*0*/			{     	1,			 "AT\r\n",																												Analysis_AT_Cmd					},
/*1*/			{     	2,			 "AT+CSQ\r\n",																										Analysis_CSQ_Cmd				},
/*2*/			{     	3,			 "AT+QIREGAPP\r\n",																								Analysis_QIREGAPP_Cmd		},//����APN �û������� ��ʼ��TCP/IP����
/*3*/			{     	4,			 "AT+QIACT\r\n",																									Analysis_QIACT_Cmd			},//����GPRS PDP����
/*4*/			{     	5,			 "AT+QILOCIP\r\n",																								Analysis_QILOCIP_Cmd		},//��ȡ����IP��ַ
/*5*/			{     	6,			 "AT+QSSLCFG=\"sni\",0,1\r\n",																		Analysis_SNI_Cmd				},
/*6*/			{     	7,			 "AT+QSSLCFG=\"https\",1\r\n",																		Analysis_QSSLCFG_Cmd		},//����HTTPS���� 
/*7*/			{     	8,			 "AT+QSSLCFG=\"httpsctxi\",0\r\n",																Analysis_QSSLCFG_Cmd		},//ΪHTPS����SSL����������
/*8*/			{     	9,			 NULL											,																				Analysis_QHTTPURL_Cmd		}, //AT+QHTTPURL=88,60
/*9*/			{     	10,			 NULL,																														Analysis_SEVER_Addr_Cmd	},
/*10*/		{     	11,			 "AT+QHTTPGET=60,120\r\n",																				Analysis_QHTTPGET_Cmd		},  //GET����
/*11*/		{     	12,			 "AT+QHTTPREAD\r\n",																							Analysis_QHTTPREAD_Cmd	},//��ȡHTTPS����������Ӧ
/*12*/		{     	13,			 "AT+QIDEACT\r\n",																								Analysis_QIDEACT_Cmd		},//�رյ�ǰGPRS/CSD����
/*13*/		{     	14,			 "at+qhttpcfg=\"CONTENT-TYPE\",\"application/json\"\r\n",					Analysis_QSSLCFG_Cmd		},//����JSON��ʽ
/*14*/		{     	15,			 NULL,																														Analysis_QHTTPPOST_Cmd	},//AT+QHTTPPOST=272\r\n
/*15*/		{     	16,			 NULL,																														Analysis_POSTDATA_Cmd		}, //POST ָ��Я��������
/*16*/		{     	17,			 "AT+QHTTPCFG=\"requestheader\",1",															Analysis_QHTTPCFG_Cmd			}, //�����Զ���ͷ������
/*17*/		{     	18,			 "AT+QHTTPCFG=\"requestheader\",0",															Analysis_QHTTPCFG_Cmd			}, //�ر��Զ���ͷ������
/************************************************BC66 https ATָ���������Ϊtest.kop4.com,����Ϊ�Զ���ͷ��***********************************************/
/*18*/		{     	19,			 "AT+QSCLK=0\r\n",																								BC66_Analysis_QSCLK_Cmd					},
/*19*/		{     	20,			 "AT+QSSLCFG=1,5,\"seclevel\",0\r\n",															BC66_Analysis_QSSLCFG_Cmd				},
/*20*/		{     	21,			 "AT+QSSLCFG=1,5,\"sni\",1\r\n",																	BC66_Analysis_QSSLCFG_Cmd				},
/*21*/		{     	22,			 "AT+QSSLCFG=1,5,\"debug\",4\r\n",																BC66_Analysis_QSSLCFG_Cmd				},
/*22*/		{     	23,			 "AT+QHTTPCFG=\"ssl\",1,5\r\n",																		BC66_Analysis_QHTTPCFG_Cmd			},
/*23*/		{     	24,			 "AT+QHTTPCFG=\"requestheader\",1\r\n",														BC66_Analysis_QHTTPCFG_Cmd			},
/*24*/		{     	25,			 NULL,																														BC66_Analysis_QHTTPURL_Cmd			}, //at+qhttpurl=46
/*25*/		{     	26,			 NULL,																														BC66_Analysis_SEVER_Addr_Cmd		},  //URL address
/*26*/		{     	27,			 NULL,																														BC66_Analysis_QHTTPPOST_Cmd			},//at+qhttppost=152
/*27*/		{     	28,			 NULL,																														BC66_Analysis_POSTDATA_Cmd			}, //Postdata
/*28*/		{     	29,			 "AT+QHTTPREAD=1024\r\n",																					BC66_Analysis_QHTTPREAD_Cmd			}, //AT+QHTTPREAD=1024
/*29*/		{     	30,			 "AT+QSCLK=1\r\n",																								BC66_Analysis_QSCLK_Cmd					}, //Postdata
};


uint8_t u8GetNum[M26GETCOMMANDLEN]= {8,9,10,11};
uint8_t u8PostNum[M26POSTCOMMANDLEN] = {8,9,14,15,11};
uint8_t u8HTTPNum[3] = {6,7,13};
uint8_t u8BC66PostNum[BC66POSTCOMMANDLEN] = {24,25,26,27,28};
uint8_t u8BC66HTTPNum[5] = {19,20,21,22,23}; //����HTTPS�ĳ�ʼ��ATָ�������±�

/* ʵ��itoa������Դ�� */ 
static char *myitoa(int num,char *str,int radix) 
{  
	/* ������ */ 
	char index[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"; 
	unsigned unum; /* �м���� */ 
	int i=0,j,k; 
	char temp;
	/* ȷ��unum��ֵ */ 
	if(radix==10&&num<0) /* ʮ���Ƹ��� */ 
	{ 
		unum=(unsigned)-num; 
		str[i++]='-'; 
	} 
	else unum=(unsigned)num; /* ������� */ 
	/* ���� */ 
	do  
	{ 
		str[i++]=index[unum%(unsigned)radix]; 
		unum/=radix; 
	}while(unum); 
	str[i]='\0'; 
	/* ת�� */ 
	if(str[0]=='-') k=1; /* ʮ���Ƹ��� */ 
	else k=0; 
	/* ��ԭ���ġ�/2����Ϊ��/2.0������֤��num��16~255֮�䣬radix����16ʱ��Ҳ�ܵõ���ȷ��� */ 
	 
	for(j=k;j<=(i-k-1)/2.0;j++) 
	{ 
		temp=str[j]; 
		str[j]=str[i-j-1]; 
		str[i-j-1]=temp; 
	} 
	return str; 
}

void AppObjCreate (void)
{
	/* ������ֵ�ź������״δ����ź�������ֵ��0 */
	Semaphore_Uart_Rec = xSemaphoreCreateBinary();
	
	if(Semaphore_Uart_Rec == NULL)
	{
		printf("Semaphore creat failed!\r\n");
			/* û�д����ɹ����û�������������봴��ʧ�ܵĴ������ */
	}
	Semaphore_AT_Response  = xSemaphoreCreateBinary();
	if(Semaphore_AT_Response == NULL)
	{
		printf("Semaphore creat failed!\r\n");
			/* û�д����ɹ����û�������������봴��ʧ�ܵĴ������ */
	}	
	SendATQueue = xQueueCreate(1, sizeof(uint8_t));
	if( SendATQueue == 0 )
	{
			printf("create failed\r\n");
	}
	
	/* �����¼���־�� */
	xCreatedEventGroup = xEventGroupCreate();
	
	if(xCreatedEventGroup == NULL)
	{
			/* û�д����ɹ����û�������������봴��ʧ�ܵĴ������ */
	}
	BC66_AT_Deal = xEventGroupCreate();
	if(BC66_AT_Deal == NULL)
	{
		printf("create BC66_AT_Deal failed");
			/* û�д����ɹ����û�������������봴��ʧ�ܵĴ������ */
	}
	
	BC66_AT_EventGroup = xEventGroupCreate();
	if(BC66_AT_EventGroup == NULL)
	{
		printf("create BC66_AT_EventGroup failed");
			/* û�д����ɹ����û�������������봴��ʧ�ܵĴ������ */
	}	
}

//type:command,parameters,cookingSession,hardware,info,warning
static char * Sever_Address_GET(const Stru_Sever_Info_t* SeverInfo,char* type)
{
	uint8_t u8Lenth = 0;
	char* ptUrlInfo;
//	u8Lenth = strlen(type);

	u8Lenth = strlen(SeverInfo->Sendsever)+strlen(SeverInfo->SeverVer)+strlen(SeverInfo->MeterId)+strlen(SeverInfo->CardID)+strlen(type)+10;

	ptUrlInfo = (char *) malloc(u8Lenth);
	memset(ptUrlInfo,0,u8Lenth);
//	printf("malloc end\r\n");
	u8Lenth = 0;
	u8Lenth = strlen(SeverInfo->Sendsever);
	if(u8Lenth != 0)
	{
		strcat(ptUrlInfo,SeverInfo->Sendsever);
	}
	//�汾��ƴ��
	u8Lenth = 0;
	u8Lenth = strlen(SeverInfo->SeverVer);
	if(u8Lenth != 0)
	{
		strcat(ptUrlInfo,SeverInfo->SeverVer);
	}
	
	u8Lenth = 0;
	u8Lenth = strlen(SeverInfo->MeterId);
	if(u8Lenth != 0)
	{
		strcat(ptUrlInfo,SeverInfo->MeterId);
	}
	
	u8Lenth = 0;
	u8Lenth = strlen(SeverInfo->CardID);
	if(u8Lenth != 0)
	{
		strcat(ptUrlInfo,"/");
		strcat(ptUrlInfo,SeverInfo->CardID);
	}
	
	u8Lenth = 0;
	u8Lenth = strlen(type);
	if(u8Lenth != 0)
	{
		strcat(ptUrlInfo,type);
	}
	
	strcat(ptUrlInfo,"\r\n");
	
	return ptUrlInfo;
}
static char * Post_Data_Cmd(char *postdata)
{
	char* ptPostInfo;
	uint16_t u16Lenth = 0;
	u16Lenth = strlen(postdata)+10;
	ptPostInfo = (char *) malloc(u16Lenth);
	
	u16Lenth = 0;
	u16Lenth = strlen(postdata);
	if(u16Lenth != 0)
	{
		strcat(ptPostInfo,postdata);
	}
	strcat(ptPostInfo,"\r\n");
}

//�ú�������ʵ�ֶԷ���URL��sever�����ȵļ��㣬������������䵽Send_AT_cmd[7].SendCommand
static void UrlLength(uint16_t length)
{
	char *ptUrlCharLength,chUrl[20]="AT+QHTTPURL=";
	char str;
	uint16_t i ;
	ptUrlCharLength = (char *) malloc(length+strlen(chUrl));
	myitoa(length,ptUrlCharLength,10); 
	strcat(chUrl,ptUrlCharLength);
	strcat(chUrl,",60\r\n");
	for(i =0;i<strlen(chUrl);i++)
	{
		str = chUrl[i];
		Send_AT_cmd[8].SendCommand[i] = str;
	}
}


//AT+QHTTPURL   25
//AT+QHTTPPOST  27
static void CmdLength(uint16_t urllength,uint8_t cmd_num)
{
	char ptUrlCharLength[4],chUrl[20];
	char str;
	uint16_t i ;
//	ptUrlCharLength = (char *) malloc(urllength+strlen(chUrl));
	myitoa(urllength,ptUrlCharLength,10); 
	if(cmd_num == Send_AT_cmd[URL_LENGTH_ARRAY].u8CmdNum)  //�����Ҫ���AT+QHTTPURLָ��
	{
		strcpy(chUrl,"AT+QHTTPURL=");
		strcat(chUrl,ptUrlCharLength);
//		strcat(chUrl,",60\r\n");
		strcat(chUrl,"\r\n");
		for(i =0;i<strlen(chUrl);i++)
		{
			str = chUrl[i];
			Send_AT_cmd[URL_LENGTH_ARRAY].SendCommand[i] = str;
		}
	}

	if(cmd_num == Send_AT_cmd[POST_LENGTH_ARRAY].u8CmdNum)  //�����Ҫ���AT+QHTTPPOST=272
	{
		strcpy(chUrl,"AT+QHTTPPOST=");
		strcat(chUrl,ptUrlCharLength);
		strcat(chUrl,"\r\n");
		for(i =0;i<strlen(chUrl);i++)
		{
			str = chUrl[i];
			Send_AT_cmd[POST_LENGTH_ARRAY].SendCommand[i] = str;
		}
	}	
}

//��AWS����GET����
static ErrorStatus SendGetCommand()
{
	uint8_t i = 0;
	EventBits_t event_value = 0;
	uint8_t u8QIDEACTSendcnt = 0; //���ڼ�¼AT+QIDEACT�Ĵ���
	uint8_t u8ErrorFlag = 0;
	
	xEventGroupClearBits( xCreatedEventGroup,0xffffff );
	for(i=0;i< M26GETCOMMANDLEN;i++)
	{
			Sim80x.AtCommand.FindAnswer = 0;
			printf("send:%d %s\r\n\r\n",Send_AT_cmd[u8GetNum[i]].u8CmdNum,Send_AT_cmd[u8PostNum[i]].SendCommand); 
			while(!Sim80x.AtCommand.FindAnswer)
			{				
				event_value = xEventGroupGetBits(xCreatedEventGroup);
				if(event_value != 0)
				{
					printf("GET event_value %d\r\n",event_value);
					i = M26GETCOMMANDLEN -1; //����ATָ��Ĵ�������ʹ������һ��ѭ����������Ͽ�
					xEventGroupClearBits( xCreatedEventGroup,0xffffff );
					event_value = 0;
					u8ErrorFlag = 1;
					break;
				}
//				if(CONFIG_Meter.NotHaveDog == false && IsNeedRestart == false)
//				{
						HAL_IWDG_Refresh(&hiwdg);
//				}
				memset(Sim80x.UsartRxBuffer,0,_SIM80X_BUFFER_SIZE);
				xQueueSend(SendATQueue,(void *) &Send_AT_cmd[u8GetNum[i]].u8CmdNum,(TickType_t)10);
				Sim80x_SendAtCommand(Send_AT_cmd[u8GetNum[i]].SendCommand,1000,1,"OK\r\n");
			}
	}
	if (u8ErrorFlag !=0) 
	{
		u8ErrorFlag = 0;
		return ERROR;
	}
	return SUCCESS;
}

void vApplicationIdleHook( void )
{
	if(AT_work_Flag ==1)
	{
		HAL_IWDG_Refresh(&hiwdg);
	}	
}

//��AWS����POST����
static ErrorStatus SendPostCommand()
{
	uint8_t i = 0;
	uint8_t u8Lenth = 0;
	BaseType_t xResult;
	EventBits_t event_value = 0;
	uint8_t u8ErrorFlag = 0;
	uint8_t u8QIDEACTSendcnt = 0; //���ڼ�¼AT+QIDEACT�Ĵ���
	xEventGroupClearBits( BC66_AT_EventGroup,0xffffff );

	AT_work_Flag =1;
	for(i=0;i< BC66POSTCOMMANDLEN;i++)
	{		
		Sim80x.AtCommand.FindAnswer = 0;
		printf("send:%d %s\r\n\r\n",Send_AT_cmd[u8BC66PostNum[i]].u8CmdNum,Send_AT_cmd[u8BC66PostNum[i]].SendCommand);
		while(!Sim80x.AtCommand.FindAnswer)
		{				 
			HAL_IWDG_Refresh(&hiwdg);

			memset(Sim80x.UsartRxBuffer,0,_SIM80X_BUFFER_SIZE);
			xQueueSend(SendATQueue,(void *) &Send_AT_cmd[u8BC66PostNum[i]].u8CmdNum,(TickType_t)10);
			xEventGroupSetBits(BC66_AT_Deal, 1);//��֪�������� ���µ�ATָ�����
			Sim80x_SendAtCommand(Send_AT_cmd[u8BC66PostNum[i]].SendCommand,1000,1,"OK\r\n");
			
			while(!(xSemaphoreTake(Semaphore_AT_Response, (TickType_t)portMAX_DELAY)))
			{
			}
		}
		
		event_value = xEventGroupGetBits(BC66_AT_EventGroup);
		if(event_value & BC66_POSTDATA_BIT ) //if is true ,means postdata command is wrong,and can't send other cmd;
		{
			u8ErrorFlag = 1;
			break;
		}
	}
	AT_work_Flag = 0;
//	printf("send post end!\r\n");
	if (u8ErrorFlag!=0) 
	{
		u8ErrorFlag = 0;
		return ERROR;
	}
	return SUCCESS;
}


//M26 SNI���ܲ��ԣ���GET��POSTǰҪ��SNI���� ����ģ����˵ֻ����һ�γ�ʼ�����ɡ�
void M26_HTTP_Init(void )
{
	uint8_t i = 0;
	for(i = 0;i < 3;i++)
	{
		{
			printf("send %s\r\n\r\n",Send_AT_cmd[u8HTTPNum[i]].SendCommand);
			Sim80x.AtCommand.FindAnswer = 0;
			xQueueSend(SendATQueue,(void *) &Send_AT_cmd[u8HTTPNum[i]].u8CmdNum,(TickType_t)10);	 
//			Sim80x_SendAtCommand(Send_AT_cmd[u8SniNum[i]].SendCommand,1000,1,"AT\r\r\nOK\r\n");
//			osDelay(2000);
			while(!Sim80x.AtCommand.FindAnswer)
			{
//				if(CONFIG_Meter.NotHaveDog == false && IsNeedRestart == false)
//				{
						HAL_IWDG_Refresh(&hiwdg);
//				}
				{
					Sim80x_SendAtCommand(Send_AT_cmd[u8HTTPNum[i]].SendCommand,1000,1,"OK\r\n");
					osDelay(2000);
				}				
			}
		}		
	}
}

void BC66_HTTP_Init(void )
{
	uint8_t i = 0;
	BaseType_t xResult;
	for(i = 0;i < 5;i++)
	{
		{
			printf("send %s\r\n\r\n",Send_AT_cmd[u8BC66HTTPNum[i]].SendCommand);
			Sim80x.AtCommand.FindAnswer = 0;
			xQueueSend(SendATQueue,(void *) &Send_AT_cmd[u8BC66HTTPNum[i]].u8CmdNum,(TickType_t)10);	 
//			Sim80x_SendAtCommand(Send_AT_cmd[u8SniNum[i]].SendCommand,1000,1,"AT\r\r\nOK\r\n");
//			osDelay(2000);
			while(!Sim80x.AtCommand.FindAnswer)
			{
//				if(CONFIG_Meter.NotHaveDog == false && IsNeedRestart == false)
//				{
//						HAL_IWDG_Refresh(&hiwdg);
//				}
				{
					xEventGroupSetBits(BC66_AT_Deal, 1);//��֪�������� ���µ�ATָ�����
					Sim80x_SendAtCommand(Send_AT_cmd[u8BC66HTTPNum[i]].SendCommand,1000,1,"OK\r\n");
					
					while(!(xSemaphoreTake(Semaphore_AT_Response, (TickType_t)portMAX_DELAY)))
					{
//						osDelay(2000);
					}
//					xResult = xSemaphoreTake(Semaphore_AT_Response, (TickType_t)portMAX_DELAY);
//					osDelay(2000);
				}				
			}
		}		
	}	
}
//######################################################################################################################
//AT ָ�������
//��ȷ����1�����󷵻�0��
uint8_t Analysis_AT_Cmd(char *pdata)
{
	char *ptStrStart ;
	char *ptFindResult ;
	static uint8_t u8ErrorCnt = 0;
	
	ptStrStart = (char*)Sim80x.UsartRxBuffer;
	ptFindResult = strstr(ptStrStart,"OK");
	if(ptFindResult != NULL)
	{
		#ifdef ANALYSIS_PRINT
		printf("AT OK find!\r\n");
		#endif
		u8ErrorCnt =0;
		return 1;
	}	
	
	u8ErrorCnt++;
	if(u8ErrorCnt == 10)
	{
		u8ErrorCnt = 0;
		xEventGroupSetBits(xCreatedEventGroup, ALL_AT_BIT | AT_BIT);
		#ifdef ANALYSIS_PRINT
		printf("AT ERR\r\n");
		#endif
	}
	return 0;
}
//POST ָ��Я�������ݴ�����
//��ȷ����1�����󷵻�0��
uint8_t Analysis_POSTDATA_Cmd(char *pdata)
{
	char *ptStrStart ;
	char *ptFindResult ;
	ptStrStart = (char*)Sim80x.UsartRxBuffer;
	ptFindResult = strstr(ptStrStart,"OK");
	if(ptFindResult != NULL)
	{
		return 1;
	}	
	ptFindResult = strstr(ptStrStart,"ERROR");
	if(ptFindResult != NULL)
	{
		xEventGroupSetBits(xCreatedEventGroup, ALL_AT_BIT | POSTDATA_BIT);
		return 1;
	}
	return 0;
}

//AT+CSQָ�������
//��ȷ����1�����󷵻�0��
uint8_t Analysis_CSQ_Cmd(char *pdata)
{
	char *ptStrStart ;
	char *ptFindResult ;
	static uint8_t u8ErrorCnt = 0;
	
	ptStrStart = (char*)Sim80x.UsartRxBuffer;
	ptFindResult = strstr(ptStrStart,"OK");
	if(ptFindResult != NULL)
	{
		ptFindResult = strstr(ptStrStart,"CSQ");
		if(ptFindResult != NULL)
		{
			if((strstr(ptStrStart,"99") || strstr(ptStrStart,"0,0")))
			{
				u8ErrorCnt++;
				if(u8ErrorCnt == 15)
				{
					u8ErrorCnt = 0;
					xEventGroupSetBits(xCreatedEventGroup, ALL_AT_BIT | CSQ_BIT);
		#ifdef ANALYSIS_PRINT
		printf("CSQ ERR !\r\n");
		#endif					
				}				
			}
			else
			{
		#ifdef ANALYSIS_PRINT
		printf("CSQ OK find!\r\n");
		#endif				
				u8ErrorCnt=0;
				return 1;
			}
		}
	}	
	return 0;
}

//AT+QIREGAPPָ�������
//��ȷ����1�����󷵻�0��
uint8_t Analysis_QIREGAPP_Cmd(char *pdata)
{
	char *ptStrStart ;
	char *ptFindResult ;
	static uint8_t u8ErrorCnt = 0;
	
	ptStrStart = (char*)Sim80x.UsartRxBuffer;
	ptFindResult = strstr(ptStrStart,"OK");
	if(ptFindResult != NULL)
	{
		u8ErrorCnt = 0;
		return 1;
	}	
	ptFindResult = strstr(ptStrStart,"ERROR");
	if(ptFindResult != NULL)
	{
		u8ErrorCnt++;
		if(u8ErrorCnt == 10)
		{
			u8ErrorCnt = 0;
			xEventGroupSetBits(xCreatedEventGroup, ALL_AT_BIT | QIREGAPP_BIT);
		}
		return 1;
	}	
	return 0;
}

//AT+QIACTָ�������
//��ȷ����1�����󷵻�0��
uint8_t Analysis_QIACT_Cmd(char *pdata)
{
	char *ptStrStart ;
	char *ptFindResult ;
	static uint8_t u8ErrorCnt = 0;

	ptStrStart = (char*)Sim80x.UsartRxBuffer;
	ptFindResult = strstr(ptStrStart,"OK");
	if(ptFindResult != NULL)
	{
		return 1;
	}	
//	
	ptFindResult = strstr(ptStrStart,"ERROR");
	if(ptFindResult != NULL)
	{
		u8ErrorCnt++;
		if(u8ErrorCnt == 5)
		{
			u8ErrorCnt=0;
			xEventGroupSetBits(xCreatedEventGroup, ALL_AT_BIT | QIACT_BIT);
		}
		
		return 0;
	}	
	return 0;
}

//AT+QIACTָ���ȡ����IP
//��ȷ����1�����󷵻�0��
uint8_t Analysis_QILOCIP_Cmd(char *pdata)
{
	char *ptStrStart ;
	char *ptFindResult ;
	static uint8_t u8ErrorCnt = 0;
	
	ptStrStart = (char*)Sim80x.UsartRxBuffer;
	ptFindResult = strstr(ptStrStart,".");
	if(ptFindResult != NULL)
	{
		return 1;
	}
	ptFindResult = strstr(ptStrStart,"ERROR");
	if(ptFindResult != NULL)
	{
		u8ErrorCnt++;
		if(u8ErrorCnt == 5)
		{
			u8ErrorCnt = 0;
			xEventGroupSetBits(xCreatedEventGroup, ALL_AT_BIT | QILOCIP_BIT);
		}
		return 0;
	}
	return 0;
}

//URL��ַ
//��ȷ����1�����󷵻�0��
uint8_t Analysis_SEVER_Addr_Cmd(char *pdata)
{
	char *ptStrStart ;
	char *ptFindResult ;
	
	ptStrStart = (char*)Sim80x.UsartRxBuffer;
	ptFindResult = strstr(ptStrStart,"OK");
	if(ptFindResult != NULL)
	{
		return 1;
	}
	
	ptFindResult = strstr(ptStrStart,"ERROR");
	if(ptFindResult != NULL)
	{
		xEventGroupSetBits(xCreatedEventGroup, ALL_AT_BIT | SEVERADDR_BIT);
		return 0;
	}		
	return 0;
}

uint8_t Analysis_SNI_Cmd(char *pdata)
{
	return 1;
}

//AT+QHTTPPOSTָ��
//��ȷ����1�����󷵻�0��
uint8_t Analysis_QHTTPPOST_Cmd(char *pdata)
{
	char *ptStrStart ;
	char *ptFindResult ;
	
	ptStrStart = (char*)Sim80x.UsartRxBuffer;
	ptFindResult = strstr(ptStrStart,"CONNECT");
	if(ptFindResult != NULL)
	{
		return 1;
	}	
	ptFindResult = strstr(ptStrStart,"ERROR");
	if(ptFindResult != NULL)
	{
		xEventGroupSetBits(xCreatedEventGroup, ALL_AT_BIT | QHTTPPOST_BIT);
		return 1;
	}	
	return 0;
}

//AT+QSSLCFGָ��
//��ȷ����1�����󷵻�0��
uint8_t Analysis_QSSLCFG_Cmd(char *pdata)
{
	char *ptStrStart ;
	char *ptFindResult ;
	
	ptStrStart = (char*)Sim80x.UsartRxBuffer;
	ptFindResult = strstr(ptStrStart,"OK");
	if(ptFindResult != NULL)
	{
		return 1;
	}	
	
	ptFindResult = strstr(ptStrStart,"ERROR");
	if(ptFindResult != NULL)
	{
		xEventGroupSetBits(xCreatedEventGroup, ALL_AT_BIT | QSSLCFG_BIT);
		return 0;
	}
	return 0;
}

//AT+QHTTPURLָ��
//��ȷ����1�����󷵻�0��
uint8_t Analysis_QHTTPURL_Cmd(char *pdata)
{
	char *ptStrStart ;
	char *ptFindResult ;
	
	ptStrStart = (char*)Sim80x.UsartRxBuffer;
	ptFindResult = strstr(ptStrStart,"CONNECT");
	if(ptFindResult != NULL)
	{
		return 1;
	}	
	ptFindResult = strstr(ptStrStart,"ERROR");
	if(ptFindResult != NULL)
	{
		xEventGroupSetBits(xCreatedEventGroup, ALL_AT_BIT | QHTTPURL_BIT);
		return 1;
	}	
	return 0;
}


uint8_t Analysis_QHTTPGET_Cmd(char *pdata)
{

	return 0;
}

//AT+QHTTPURLָ��
//��ȷ����1�����󷵻�0��
uint8_t Analysis_QHTTPREAD_Cmd(char *pdata)
{
	
	return 0;
}

uint8_t Analysis_QIDEACT_Cmd(char *pdata)
{

	return 0;
}

uint8_t Analysis_QHTTPCFG_Cmd(char *pdata)
{

	return 0;
}



/**************follow is bc66 AT command analysis function******************************/
//AT+QSCLK=0 �͹��Ľ�����������
#define BC66_ANALYSIS_DEBUG 0
uint8_t BC66_Analysis_QSCLK_Cmd(char *pdata)
{
	return 0;
}

uint8_t BC66_Analysis_QSSLCFG_Cmd(char *pdata)
{
	char *ptStrStart ;
	char *ptFindResult ;
	#if 	BC66_ANALYSIS_DEBUG
	printf("QSSLCFG analysis\r\n");
	#endif
	
	ptStrStart = (char*)Sim80x.UsartRxBuffer;
	ptFindResult = strstr(ptStrStart,"OK");
	if(ptFindResult != NULL)
	{
		#if 	BC66_ANALYSIS_DEBUG
		printf("return 1\r\n");
		#endif
		return 1;
	}	
	
	ptFindResult = strstr(ptStrStart,"ERROR");
	if(ptFindResult != NULL)
	{
		xEventGroupSetBits(xCreatedEventGroup, ALL_AT_BIT | QSSLCFG_BIT);
		#if 	BC66_ANALYSIS_DEBUG
		printf("return 0\r\n");
		#endif
		return 0;
	}	
	return 0;
}

uint8_t BC66_Analysis_QHTTPCFG_Cmd(char *pdata)
{
	char *ptStrStart ;
	char *ptFindResult ;
	#if 	BC66_ANALYSIS_DEBUG
	printf("qhttpcfg analysis\r\n");
	#endif
	ptStrStart = (char*)Sim80x.UsartRxBuffer;
	ptFindResult = strstr(ptStrStart,"OK");
	if(ptFindResult != NULL)
	{
		#if 	BC66_ANALYSIS_DEBUG
		printf("return 1\r\n");
		#endif
		return 1;
	}	
	
	ptFindResult = strstr(ptStrStart,"ERROR");
	if(ptFindResult != NULL)
	{
		xEventGroupSetBits(xCreatedEventGroup, ALL_AT_BIT | QSSLCFG_BIT);
		#if 	BC66_ANALYSIS_DEBUG
		printf("return 0\r\n");
		#endif
		return 0;
	}	
	return 0;
}

uint8_t BC66_Analysis_QHTTPURL_Cmd(char *pdata)
{
	char *ptStrStart ;
	char *ptFindResult ;
	
	ptStrStart = (char*)Sim80x.UsartRxBuffer;
	#if 	BC66_ANALYSIS_DEBUG
	printf("QHTTPURL analysis\r\n");
	printf("%s\r\n",ptStrStart);
	#endif
	ptFindResult = strstr(ptStrStart,">");
	if(ptFindResult != NULL)
	{
		#if 	BC66_ANALYSIS_DEBUG
		printf("has find >\r\n");
		#endif
//		xSemaphoreGive(Semaphore_AT_Response);
		return 1;
	}	
	
	ptFindResult = strstr(ptStrStart,"ERROR");
	if(ptFindResult != NULL)
	{
		#if 	BC66_ANALYSIS_DEBUG
		printf("return 0\r\n");
		#endif
		xEventGroupSetBits(xCreatedEventGroup, ALL_AT_BIT | QSSLCFG_BIT);
		return 0;
	}		
	return 0;
}

uint8_t BC66_Analysis_POSTDATA_Cmd(char *pdata)
{
	char *ptStrStart ;
	char *ptFindResult ;
	ptStrStart = (char*)Sim80x.UsartRxBuffer;
	ptFindResult = strstr(ptStrStart,"+QHTTPPOST:");
	if(ptFindResult != NULL)
	{
		ptFindResult = strstr(ptStrStart,",");
		if(ptFindResult != NULL)
		{
			#if 	BC66_ANALYSIS_DEBUG
			printf("return 1\r\n");
			#endif
//			return 1;
		}	
		else
		{
			xEventGroupSetBits(BC66_AT_EventGroup,BC66_POSTDATA_BIT); //���أ�����AT+QHTTPPOST ���ص��Ǵ�����
		}
		return 1;
	}
	
	ptFindResult = strstr(ptStrStart,"ERROR");
	if(ptFindResult != NULL)
	{
		xEventGroupSetBits(xCreatedEventGroup, ALL_AT_BIT | POSTDATA_BIT);
		#if 	BC66_ANALYSIS_DEBUG
		printf("return 0\r\n");
		#endif
		return 0;
	}	
	return 0;
}

uint8_t BC66_Analysis_QHTTPREAD_Cmd(char *pdata)
{
	char *ptStrStart ;
	char *ptFindResult ;
	#if 	BC66_ANALYSIS_DEBUG
	printf("QSSLCFG analysis\r\n");
	#endif
	
	ptStrStart = (char*)Sim80x.UsartRxBuffer;
	ptFindResult = strstr(ptStrStart,"OK");
	if(ptFindResult != NULL)
	{
		#if 	BC66_ANALYSIS_DEBUG
		printf("return 1\r\n");
		#endif
		return 1;
	}	
	
	ptFindResult = strstr(ptStrStart,"ERROR");
	if(ptFindResult != NULL)
	{
		xEventGroupSetBits(xCreatedEventGroup, ALL_AT_BIT | QSSLCFG_BIT);
		#if 	BC66_ANALYSIS_DEBUG
		printf("return 0\r\n");
		#endif
		return 0;
	}	
	return 0;
}

uint8_t BC66_Analysis_SEVER_Addr_Cmd(char *pdata)
{
	char *ptStrStart ;
	char *ptFindResult ;
	
	ptStrStart = (char*)Sim80x.UsartRxBuffer;
	ptFindResult = strstr(ptStrStart,"OK");
	if(ptFindResult != NULL)
	{
		#if 	BC66_ANALYSIS_DEBUG
		printf("return 1\r\n");
		#endif
		return 1;
	}
	
	ptFindResult = strstr(ptStrStart,"ERROR");
	if(ptFindResult != NULL)
	{
		xEventGroupSetBits(xCreatedEventGroup, ALL_AT_BIT | SEVERADDR_BIT);
		#if 	BC66_ANALYSIS_DEBUG
		printf("return 0\r\n");
		#endif
		return 0;
	}		
	return 0;
}

uint8_t BC66_Analysis_QHTTPPOST_Cmd(char *pdata)
{
	char *ptStrStart ;
	char *ptFindResult ;
	#if 	BC66_ANALYSIS_DEBUG
	printf("qhttpcfg analysis\r\n");
	#endif
	ptStrStart = (char*)Sim80x.UsartRxBuffer;
	ptFindResult = strstr(ptStrStart,">");
	if(ptFindResult != NULL)
	{
		#if 	BC66_ANALYSIS_DEBUG
		printf("return 1\r\n");
		#endif
		return 1;
	}	
	
	ptFindResult = strstr(ptStrStart,"ERROR");
	if(ptFindResult != NULL)
	{
		xEventGroupSetBits(xCreatedEventGroup, ALL_AT_BIT | QSSLCFG_BIT);
		#if 	BC66_ANALYSIS_DEBUG
		printf("return 0\r\n");
		#endif
		return 0;
	}		
	return 0;
}

//url:
//postdata ������ͷ������
//data:��ʽ���ģ�����ͷ���������ݣ�������˵�ַ
void EncodePostDataStru(char *url,char *postdata,char **data)
{
//	char data[1024] = {0};
	char data_length[4];
	if(postdata != NULL)  //POST����
	{
		myitoa(strlen(postdata),data_length,10); //��ȡ��������ݲ��ֵ���ʵ���ȣ�ȥ��\r\n�����ַ�
		strcat(*data,"POST ");
//		strcat(*data,url);
		strncat(*data,url, strlen(url)-2);  //ȥ��\r\n�����ַ�
		strcat(*data," HTTP/1.1");
		strcat(*data,"\r\n");//POST https://test.kop4.com/meter/warning/TZ00000131 HTTP/1.1
		
		strcat(*data,X_LOCALE);
		strcat(*data,"\r\n");
		
		strcat(*data,X_API_KEY);
		strcat(*data,"\r\n");
		
		strcat(*data,"Content-Type: application/json");
		strcat(*data,"\r\n");
		
		strcat(*data,HOST);
		strcat(*data,"\r\n");
		
		strcat(*data,"Content-Length:");
		strcat(*data,data_length);
		strcat(*data,"\r\n");
		strcat(*data,"\r\n");
		
		strcat(*data,postdata);
//		strcat(*data,"\r\n");		
		
		strcat(*data,"\r\n");		//��������ʱ�ĵĻس���
	}
	else //GET����
	{
		strcat(*data,"GET ");
		strncat(*data,url, strlen(url)-2);  //ȥ��\r\n�����ַ�
		strcat(*data," HTTP/1.1");
		strcat(*data,"\r\n");//GET https://test.kop4.com/meter/command/TZ00000235 HTTP/1.1
		
		strcat(*data,X_LOCALE);
		strcat(*data,"\r\n");
		
		strcat(*data,X_API_KEY);
		strcat(*data,"\r\n");		
		
		strcat(*data,HOST);
		strcat(*data,"\r\n");		
		
		strcat(*data,"\r\n");	
		strcat(*data,"\r\n");	
		
		strcat(*data,"\r\n"); //��������ʱ�ĵĻس���
	}

}


//postcookingsecsion ���ͺ���
void  PostCookingSecsion(void)  //SendReportDataPacket
{
	Stru_Sever_Info_t *struSeverInfo;
	uint8_t result = 0 , i = 0; //���ڱ�ʶ���Ƿ���Ӧ�˵�ǰ��ָ��
	char *ptUrl,*ptPost;
	char *ptPostData;
	ErrorStatus ErrorFlag;
	char *ptMeterID;
	char *ptPostDataStru;
	volatile uint16_t u8UrlLength = 0;

	while(REAL_DATA_Credit.CookingSessionSendNumber < REAL_DATA_Credit.CookingSessionEnd)
	{
		Send_AT_cmd[URL_LENGTH_ARRAY].SendCommand =(char *)malloc(20);
		Send_AT_cmd[POST_LENGTH_ARRAY].SendCommand =(char *)malloc(20);
		memset(Send_AT_cmd[URL_LENGTH_ARRAY].SendCommand,0,20 *sizeof(char));
		memset(Send_AT_cmd[POST_LENGTH_ARRAY].SendCommand,0,20 *sizeof(char));
		
		ptPostDataStru = (char *) malloc(1024*sizeof(char));
		memset(ptPostDataStru,0,1024*sizeof(char));
		
		struSeverInfo = (struct SeverInfo *) malloc(sizeof(struct SeverInfo));
		ptMeterID = (char *) malloc(sizeof(char)*50);
//		ptMeterID = (char *) pvPortMalloc(sizeof(char)*50);
		ptPostData = (char *) malloc(700 *sizeof(char));
		memset(ptPostData,0,700 *sizeof(char));
		
		printf("\r\ncompare %d %d\r\n",REAL_DATA_Credit.CookingSessionSendNumber,REAL_DATA_Credit.CookingSessionEnd);
		printf("******PostCookingSecsion******\r\n");
		Cooking_Session_READ(REAL_DATA_Credit.CookingSessionSendNumber);//���͵�ʱ��ӿ�ʼλ�ÿ�ʼ��ȡ,���ͳɹ�,������һ
		refreshCookingSessionReport(&CookingSessionReport);		
		
		encodeCookingPacket(&ptPostData,&CookingSessionReport); //��� cookingsecsion POST����������
		printf("%s\r\n",ptPostData);
		
		struSeverInfo->Sendsever = SEVER_URL;
		u8UrlLength = strlen(struSeverInfo->Sendsever);
		struSeverInfo->SeverVer = SEVER_VERSION;
		printf("card id%s\r\n",CookingSessionReport.CARD_ID);
		struSeverInfo->CardID = CookingSessionReport.CARD_ID;
//		struSeverInfo->CardID = "/9088450934850394385";
		
		strcat(ptMeterID,"cookingSession/");
		strcat(ptMeterID,CONFIG_Meter.MeterNo);
		struSeverInfo->MeterId = ptMeterID;
//		struSeverInfo->MeterId = "meter/cookingSession/TZ00000235";
		ptUrl = Sever_Address_GET( struSeverInfo,"");
		
		Send_AT_cmd[URL_ADDR_ARRAY].SendCommand = ptUrl; //URL��ַ
		u8UrlLength = strlen(ptUrl)-2;
		CmdLength(u8UrlLength,25);  //���ݷ���URL�ĳ���		��ȡURL�ĳ������AT+QHTTPURL=XX,60
		
		EncodePostDataStru(ptUrl,ptPostData,&ptPostDataStru);
//		ptPost = Post_Data_Cmd( ptPostData);
		Send_AT_cmd[POST_DATA_ARRAY].SendCommand = ptPostDataStru;
		u8UrlLength = strlen(ptPostDataStru)-2;
		CmdLength(strlen(ptPostDataStru)-2,27);  //���ݷ���POST�ĳ���
		
		ErrorFlag =SendPostCommand();
		
		free(ptMeterID);
		free(struSeverInfo);
		free(ptUrl);
//		free(ptPost);
		free(ptPostDataStru);
		free(Send_AT_cmd[URL_LENGTH_ARRAY].SendCommand);
		free(Send_AT_cmd[POST_LENGTH_ARRAY].SendCommand);		
		free(ptPostData);
		
		if(ErrorFlag != ERROR)   //���ͳɹ��������кż�1
		{
			REAL_DATA_Credit.CookingSessionSendNumber++;
			REAL_DATA_Credit_Write();//������cooking ,�������		
			printf("******end PostCookingSecsion******\r\n");
		}
		else  //����ʧ�ܲ����£���һ���������ϴ�
		{
			printf("******PostCookingSecsion fail!******\r\n");
			return ;			
		}
			
		
		
	}
}

//PostMeterStatus ���ͺ���
void  PostMeterStatus(void)  //SendReportStatePacket
{
	Stru_Sever_Info_t *struSeverInfo;
	uint8_t result = 0 , i = 0; //���ڱ�ʶ���Ƿ���Ӧ�˵�ǰ��ָ��
	char *ptUrl,*ptPost;
	char *ptPostData;
	char *cDataTime ;
	char *ptMeterID;
	char *ptPostDataStru;
	volatile uint16_t u8UrlLength = 0;	
	
//	M26_Sni_Init();
	Send_AT_cmd[URL_LENGTH_ARRAY].SendCommand =(char *)malloc(20);
	Send_AT_cmd[POST_LENGTH_ARRAY].SendCommand =(char *)malloc(20);
	memset(Send_AT_cmd[URL_LENGTH_ARRAY].SendCommand,0,20 *sizeof(char));
	memset(Send_AT_cmd[POST_LENGTH_ARRAY].SendCommand,0,20 *sizeof(char));

	ptPostDataStru = (char *) malloc(1024*sizeof(char));
	memset(ptPostDataStru,0,1024*sizeof(char));
	
	struSeverInfo = (struct SeverInfo *) malloc(sizeof(struct SeverInfo));
	ptMeterID = (char *) malloc(sizeof(char)*50);
	ptPostData = (char *) malloc(700 *sizeof(char));
	memset(ptPostData,0,700 *sizeof(char));
	
	printf("******PostMeterStatus******\r\n");
	
	refreshReportStatePacket(&reportStatePacket);		
	encodeMeterStatusPacket(&ptPostData,&reportStatePacket); //��� cookingsecsion POST����������
	
	struSeverInfo->Sendsever = SEVER_URL;
	u8UrlLength = strlen(struSeverInfo->Sendsever);
	struSeverInfo->SeverVer = SEVER_VERSION;
	struSeverInfo->CardID = "";
	strcat(ptMeterID,"status/");
	strcat(ptMeterID,CONFIG_Meter.MeterNo);
	struSeverInfo->MeterId = ptMeterID;
//	struSeverInfo->MeterId = "meter/status/TZ00000111";
	ptUrl = Sever_Address_GET( struSeverInfo,"");
	
	Send_AT_cmd[URL_ADDR_ARRAY].SendCommand = ptUrl; //URL��ַ
	u8UrlLength = strlen(ptUrl)-2;
	CmdLength(u8UrlLength,25);  //���ݷ���URL�ĳ���		��ȡURL�ĳ������AT+QHTTPURL=XX,60
	
	EncodePostDataStru(ptUrl,ptPostData,&ptPostDataStru);
	Send_AT_cmd[POST_DATA_ARRAY].SendCommand = ptPostDataStru;
	u8UrlLength = strlen(ptPostDataStru)-2;
	CmdLength(strlen(ptPostDataStru)-2,27);  //���ݷ���POST�ĳ���
	
	SendPostCommand();
	
	free(ptMeterID);
	free(struSeverInfo);
	free(ptUrl);
//	free(ptPost);
	free(ptPostDataStru);
	free(Send_AT_cmd[URL_LENGTH_ARRAY].SendCommand);
	free(Send_AT_cmd[POST_LENGTH_ARRAY].SendCommand);		
	free(ptPostData);	
	
	printf("******end PostMeterStatus******\r\n");

}

//PostMeterWarning ���ͺ���
void  PostMeterWarning(void)  //SendWarnPacket();
{
	Stru_Sever_Info_t *struSeverInfo;
//	uint8_t result = 0 , i = 0; //���ڱ�ʶ���Ƿ���Ӧ�˵�ǰ��ָ��
	char *ptUrl,*ptPost;
	char *ptPostData;
	char *cDataTime ;
	char *ptMeterID;
	char *ptPostDataStru;
	volatile uint16_t u8UrlLength = 0;	
	
//	M26_Sni_Init();
	Send_AT_cmd[URL_LENGTH_ARRAY].SendCommand =(char *)malloc(20);
	Send_AT_cmd[POST_LENGTH_ARRAY].SendCommand =(char *)malloc(20);
	memset(Send_AT_cmd[URL_LENGTH_ARRAY].SendCommand,0,20 *sizeof(char));
	memset(Send_AT_cmd[POST_LENGTH_ARRAY].SendCommand,0,20 *sizeof(char));
	
	ptPostDataStru = (char *) malloc(1024*sizeof(char));
	memset(ptPostDataStru,0,1024*sizeof(char));
	
	struSeverInfo = (struct SeverInfo *) malloc(sizeof(struct SeverInfo));
	ptMeterID = (char *) malloc(sizeof(char)*50);
	ptPostData = (char *) malloc(700 *sizeof(char));
	memset(ptPostData,0,700 *sizeof(char));
	
	printf("******PostMeterWarning******\r\n");
	refreshWaringPacket(&waringPacket);
	encodeWarningsPacket(&ptPostData,&waringPacket); //��� cookingsecsion POST����������
	
	struSeverInfo->Sendsever = SEVER_URL;
	u8UrlLength = strlen(struSeverInfo->Sendsever);
	struSeverInfo->SeverVer = SEVER_VERSION;
	struSeverInfo->CardID = "";
	
//	strcat(ptMeterID,"meter/settings/");
	strcat(ptMeterID,"warning/");
	strcat(ptMeterID,CONFIG_Meter.MeterNo);
//	strcat(ptMeterID,"TZ00000131");
	struSeverInfo->MeterId = ptMeterID;
//	struSeverInfo->MeterId = "TZ00000131";
	ptUrl = Sever_Address_GET( struSeverInfo,"");
	
	Send_AT_cmd[URL_ADDR_ARRAY].SendCommand = ptUrl; //URL��ַ
	u8UrlLength = strlen(ptUrl)-2;
	CmdLength(u8UrlLength,25);  //���ݷ���URL�ĳ���		��ȡURL�ĳ������AT+QHTTPURL=XX,60
	
	EncodePostDataStru(ptUrl,ptPostData,&ptPostDataStru);
//	ptPost = Post_Data_Cmd( ptPostData);
	Send_AT_cmd[POST_DATA_ARRAY].SendCommand = ptPostDataStru;
	u8UrlLength = strlen(ptPostDataStru)-2;
	CmdLength(strlen(ptPostDataStru)-2,27);  //���ݷ���POST�ĳ���
	
	SendPostCommand();
	
	free(ptMeterID);
	free(struSeverInfo);
	free(ptUrl);
//	free(ptPost);
	free(ptPostDataStru);
	free(Send_AT_cmd[URL_LENGTH_ARRAY].SendCommand);
	free(Send_AT_cmd[POST_LENGTH_ARRAY].SendCommand);		
	free(ptPostData);	
	
	printf("******end PostMeterWarning******\r\n");
}

//PostMeterHardware ���ͺ���
void  PostMeterHardware(void)  //SendWarnPacket();
{
	Stru_Sever_Info_t *struSeverInfo;
//	uint8_t result = 0 , i = 0; //���ڱ�ʶ���Ƿ���Ӧ�˵�ǰ��ָ��
	char *ptUrl,*ptPost;
	char *ptPostData;
	char *cDataTime ;
	char *ptMeterID;
	char *ptPostDataStru;
	volatile uint16_t u8UrlLength = 0;	
	
//	M26_Sni_Init();
	Send_AT_cmd[URL_LENGTH_ARRAY].SendCommand =(char *)malloc(20);
	Send_AT_cmd[POST_LENGTH_ARRAY].SendCommand =(char *)malloc(20);
	memset(Send_AT_cmd[URL_LENGTH_ARRAY].SendCommand,0,20 *sizeof(char));
	memset(Send_AT_cmd[POST_LENGTH_ARRAY].SendCommand,0,20 *sizeof(char));
	
	
	ptPostDataStru = (char *) malloc(1024*sizeof(char));
	memset(ptPostDataStru,0,1024*sizeof(char));
	
	struSeverInfo = (struct SeverInfo *) malloc(sizeof(struct SeverInfo));
	ptMeterID = (char *) malloc(sizeof(char)*50);
	ptPostData = (char *) malloc(700 *sizeof(char));
	memset(ptPostData,0,700 *sizeof(char));
	
	printf("******PostMeterHardware******\r\n");
	refreshInformationPacket(&InformationPacket);
	encodeHardwarePacket(&ptPostData,&InformationPacket); 
	
	struSeverInfo->Sendsever = SEVER_URL;
	u8UrlLength = strlen(struSeverInfo->Sendsever);
	struSeverInfo->SeverVer = SEVER_VERSION;
	struSeverInfo->CardID = "";
	
	strcat(ptMeterID,"hardware/");
	strcat(ptMeterID,CONFIG_Meter.MeterNo);
	struSeverInfo->MeterId = ptMeterID;	
//	struSeverInfo->MeterId = "meter/hardware/TZ00000131";
	ptUrl = Sever_Address_GET( struSeverInfo,"");
	
	Send_AT_cmd[URL_ADDR_ARRAY].SendCommand = ptUrl; //URL��ַ
	u8UrlLength = strlen(ptUrl)-2;
	CmdLength(u8UrlLength,25);  //���ݷ���URL�ĳ���		��ȡURL�ĳ������AT+QHTTPURL=XX,60
	
	EncodePostDataStru(ptUrl,ptPostData,&ptPostDataStru);
//	ptPost = Post_Data_Cmd( ptPostData);
	Send_AT_cmd[POST_DATA_ARRAY].SendCommand = ptPostDataStru;
	u8UrlLength = strlen(ptPostDataStru)-2;
	CmdLength(strlen(ptPostDataStru)-2,27);  //���ݷ���POST�ĳ���
	
	SendPostCommand();
	
	free(ptMeterID);
	free(struSeverInfo);	
	free(ptUrl);
//	free(ptPost);
	free(ptPostDataStru);
	free(Send_AT_cmd[URL_LENGTH_ARRAY].SendCommand);
	free(Send_AT_cmd[POST_LENGTH_ARRAY].SendCommand);		
	free(ptPostData);	
	
	printf("******end PostMeterHardware******\r\n");
}

//PostMeterSettings ���ͺ���
void  PostMeterSettings(void)  //
{
	Stru_Sever_Info_t *struSeverInfo;
//	uint8_t result = 0 , i = 0; //���ڱ�ʶ���Ƿ���Ӧ�˵�ǰ��ָ��
	char *ptUrl,*ptPost;
	char *ptPostData;
	char *cDataTime ;
	volatile uint16_t u8UrlLength = 0;
	char *ptMeterID;
	char *ptPostDataStru;
	CONFIG_Meter_t stReadMeterConfig;
	
//	M26_Sni_Init();
	Send_AT_cmd[URL_LENGTH_ARRAY].SendCommand =(char *)malloc(20);
	Send_AT_cmd[POST_LENGTH_ARRAY].SendCommand =(char *)malloc(20);
	memset(Send_AT_cmd[URL_LENGTH_ARRAY].SendCommand,0,20 *sizeof(char));
	memset(Send_AT_cmd[POST_LENGTH_ARRAY].SendCommand,0,20 *sizeof(char));
	
	ptPostDataStru = (char *) malloc(1024*sizeof(char));
	memset(ptPostDataStru,0,1024*sizeof(char));
	
	struSeverInfo = (struct SeverInfo *) malloc(sizeof(struct SeverInfo));
	ptMeterID = (char *) malloc(sizeof(char)*50);
	ptPostData = (char *) malloc(700 *sizeof(char));
	memset(ptPostData,0,700 *sizeof(char));
	
	printf("******PostMeterSettings******\r\n");
	refreshSetupPacket(&SetupPacket);
//	MB85RS16A_READ(CONFIG_Meter_Address,(uint8_t*)(&stReadMeterConfig),sizeof(CONFIG_Meter_t));
	
	encodeSettingsPacket(&ptPostData,&SetupPacket); 
	
	
	struSeverInfo->Sendsever = SEVER_URL;
	u8UrlLength = strlen(struSeverInfo->Sendsever);
	struSeverInfo->SeverVer = SEVER_VERSION;
	struSeverInfo->CardID = "";
	
//	strcat(ptMeterID,"meter/settings/");
	strcat(ptMeterID,"settings/");
	strcat(ptMeterID,CONFIG_Meter.MeterNo);
	struSeverInfo->MeterId = ptMeterID;
//	struSeverInfo->MeterId = "meter/settings/TZ00000525";
	ptUrl = Sever_Address_GET( struSeverInfo,"");
	
	Send_AT_cmd[URL_ADDR_ARRAY].SendCommand = ptUrl; //URL��ַ
	u8UrlLength = strlen(ptUrl)-2;
	CmdLength(u8UrlLength,25);  //���ݷ���URL�ĳ���		��ȡURL�ĳ������AT+QHTTPURL=XX,60
//	printf("length = %d\r\n",strlen(ptPostData));
	EncodePostDataStru(ptUrl,ptPostData,&ptPostDataStru);
	Send_AT_cmd[POST_DATA_ARRAY].SendCommand = ptPostDataStru;
	u8UrlLength = strlen(ptPostDataStru)-2;
	printf("u8UrlLength = %d",u8UrlLength);
	CmdLength(strlen(ptPostDataStru)-2,27);  //���ݷ���POST�ĳ���
	
	SendPostCommand();
	
	free(ptMeterID);
	free(struSeverInfo);
	free(ptUrl);
//	free(ptPost);
	free(ptPostDataStru);
	free(Send_AT_cmd[URL_LENGTH_ARRAY].SendCommand);
	free(Send_AT_cmd[POST_LENGTH_ARRAY].SendCommand);		
	free(ptPostData);	
	
	printf("******end PostMeterSettings******\r\n");
}


//GetMeterSettings ���ͺ���
void  GetMeterSettings(void)  //
{
	Stru_Sever_Info_t *struSeverInfo;
//	uint8_t result = 0 , i = 0; //���ڱ�ʶ���Ƿ���Ӧ�˵�ǰ��ָ��
	char *ptUrl;
	char *cDataTime ;
	volatile uint16_t u8UrlLength = 0;	
	char *ptMeterID;
	char *ptPostDataStru;
	
	Send_AT_cmd[URL_LENGTH_ARRAY].SendCommand =(char *)malloc(20);
	Send_AT_cmd[POST_LENGTH_ARRAY].SendCommand =(char *)malloc(20);
	memset(Send_AT_cmd[URL_LENGTH_ARRAY].SendCommand,0,20 *sizeof(char));
	memset(Send_AT_cmd[POST_LENGTH_ARRAY].SendCommand,0,20 *sizeof(char));
	
	ptPostDataStru = (char *) malloc(1024*sizeof(char));
	memset(ptPostDataStru,0,1024*sizeof(char));
	
	struSeverInfo = (struct SeverInfo *) malloc(sizeof(struct SeverInfo));
	ptMeterID = (char *) malloc(sizeof(char)*50);
//	ptPostData = (char *) malloc(500 *sizeof(char));
//	memset(ptPostData,0,500 *sizeof(char));
	
	printf("******GetMeterSettings******\r\n");
	
	struSeverInfo->Sendsever = SEVER_URL;
	u8UrlLength = strlen(struSeverInfo->Sendsever);
	struSeverInfo->SeverVer = SEVER_VERSION;
	struSeverInfo->CardID = ""; //���ֶ�Ϊ�գ���card_id
	strcat(ptMeterID,"settings/");
	strcat(ptMeterID,CONFIG_Meter.MeterNo);
//	struSeverInfo->MeterId = "/settings/TZ00000525";
	struSeverInfo->MeterId = ptMeterID;
	ptUrl = Sever_Address_GET( struSeverInfo,"");
	
	Send_AT_cmd[URL_ADDR_ARRAY].SendCommand = ptUrl; //URL��ַ
	u8UrlLength = strlen(ptUrl)-2;
	CmdLength(u8UrlLength,25);  //���ݷ���URL�ĳ���		��ȡURL�ĳ������AT+QHTTPURL=XX
	
	EncodePostDataStru(ptUrl,NULL,&ptPostDataStru);
	Send_AT_cmd[POST_DATA_ARRAY].SendCommand = ptPostDataStru;
	CmdLength(strlen(ptPostDataStru)-2,27);  //���ݷ���POST�ĳ���
	SendPostCommand();
	
	free(ptMeterID);
	free(struSeverInfo);
	free(ptUrl);
//	free(ptPost);
	free(ptPostDataStru);
	free(Send_AT_cmd[URL_LENGTH_ARRAY].SendCommand);
	free(Send_AT_cmd[POST_LENGTH_ARRAY].SendCommand);		
//	free(ptPostData);	
	
	printf("******end GetMeterSettings******\r\n");
}

//GetMeterCommand ���ͺ���
void  GetMeterCommand(void)  //
{
	Stru_Sever_Info_t *struSeverInfo;
//	uint8_t result = 0 , i = 0; //���ڱ�ʶ���Ƿ���Ӧ�˵�ǰ��ָ��
	char *ptUrl;
	char *cDataTime ;
	volatile uint16_t u8UrlLength = 0;	
	char *ptMeterID;
	char *ptPostDataStru;
	
	Send_AT_cmd[URL_LENGTH_ARRAY].SendCommand =(char *)malloc(20);
	Send_AT_cmd[POST_LENGTH_ARRAY].SendCommand =(char *)malloc(20);
	memset(Send_AT_cmd[URL_LENGTH_ARRAY].SendCommand,0,20 *sizeof(char));
	memset(Send_AT_cmd[POST_LENGTH_ARRAY].SendCommand,0,20 *sizeof(char));
	
	ptPostDataStru = (char *) malloc(1024*sizeof(char));
	memset(ptPostDataStru,0,1024*sizeof(char));
	
	struSeverInfo = (struct SeverInfo *) malloc(sizeof(struct SeverInfo));
	ptMeterID = (char *) malloc(sizeof(char)*50);
//	ptPostData = (char *) malloc(500 *sizeof(char));
//	memset(ptPostData,0,500 *sizeof(char));
	
	printf("******GetMeterCommand******\r\n");
	
	struSeverInfo->Sendsever = SEVER_URL;
	u8UrlLength = strlen(struSeverInfo->Sendsever);
	struSeverInfo->SeverVer = SEVER_VERSION;
	struSeverInfo->CardID = ""; //���ֶ�Ϊ�գ���card_id
	strcat(ptMeterID,"command/");
	strcat(ptMeterID,CONFIG_Meter.MeterNo);
//	struSeverInfo->MeterId = "/settings/TZ00000525";
	struSeverInfo->MeterId = ptMeterID;
	ptUrl = Sever_Address_GET( struSeverInfo,"");
	
	Send_AT_cmd[URL_ADDR_ARRAY].SendCommand = ptUrl; //URL��ַ
	u8UrlLength = strlen(ptUrl)-2;
	CmdLength(u8UrlLength,25);  //���ݷ���URL�ĳ���		��ȡURL�ĳ������AT+QHTTPURL=XX
	
	EncodePostDataStru(ptUrl,NULL,&ptPostDataStru);
	Send_AT_cmd[POST_DATA_ARRAY].SendCommand = ptPostDataStru;
	CmdLength(strlen(ptPostDataStru)-2,27);  //���ݷ���POST�ĳ���
	SendPostCommand();
	
	free(ptMeterID);
	free(struSeverInfo);
	free(ptUrl);
//	free(ptPost);
	free(ptPostDataStru);
	free(Send_AT_cmd[URL_LENGTH_ARRAY].SendCommand);
	free(Send_AT_cmd[POST_LENGTH_ARRAY].SendCommand);		
//	free(ptPostData);	
	
	printf("******end GetMeterCommand******\r\n");
}

//GetMeterFirmware ���ͺ���
void  GetMeterFirmware(void)  //
{
	Stru_Sever_Info_t *struSeverInfo;
//	uint8_t result = 0 , i = 0; //���ڱ�ʶ���Ƿ���Ӧ�˵�ǰ��ָ��
	char *ptUrl;
	char *cDataTime ;
	volatile uint16_t u8UrlLength = 0;	
	char *ptMeterID;
	char *ptPostDataStru;
	
	Send_AT_cmd[URL_LENGTH_ARRAY].SendCommand =(char *)malloc(20);
	Send_AT_cmd[POST_LENGTH_ARRAY].SendCommand =(char *)malloc(20);
	memset(Send_AT_cmd[URL_LENGTH_ARRAY].SendCommand,0,20 *sizeof(char));
	memset(Send_AT_cmd[POST_LENGTH_ARRAY].SendCommand,0,20 *sizeof(char));
	
	ptPostDataStru = (char *) malloc(1024*sizeof(char));
	memset(ptPostDataStru,0,1024*sizeof(char));
	
	struSeverInfo = (struct SeverInfo *) malloc(sizeof(struct SeverInfo));
	ptMeterID = (char *) malloc(sizeof(char)*50);
//	ptPostData = (char *) malloc(500 *sizeof(char));
//	memset(ptPostData,0,500 *sizeof(char));
	
	printf("******GetMeterFirmware******\r\n");
	
	struSeverInfo->Sendsever = SEVER_URL;
	u8UrlLength = strlen(struSeverInfo->Sendsever);
	struSeverInfo->SeverVer = SEVER_VERSION;
	struSeverInfo->CardID = ""; //���ֶ�Ϊ�գ���card_id
	strcat(ptMeterID,"firmware/");
	strcat(ptMeterID,CONFIG_Meter.MeterNo);
//	struSeverInfo->MeterId = "/settings/TZ00000525";
	struSeverInfo->MeterId = ptMeterID;
	ptUrl = Sever_Address_GET( struSeverInfo,"");
	
	Send_AT_cmd[URL_ADDR_ARRAY].SendCommand = ptUrl; //URL��ַ
	u8UrlLength = strlen(ptUrl)-2;
	CmdLength(u8UrlLength,25);  //���ݷ���URL�ĳ���		��ȡURL�ĳ������AT+QHTTPURL=XX
	
	EncodePostDataStru(ptUrl,NULL,&ptPostDataStru);
	Send_AT_cmd[POST_DATA_ARRAY].SendCommand = ptPostDataStru;
	CmdLength(strlen(ptPostDataStru)-2,27);  //���ݷ���POST�ĳ���
	SendPostCommand();
	
	free(ptMeterID);
	free(struSeverInfo);
	free(ptUrl);
//	free(ptPost);
	free(ptPostDataStru);
	free(Send_AT_cmd[URL_LENGTH_ARRAY].SendCommand);
	free(Send_AT_cmd[POST_LENGTH_ARRAY].SendCommand);		
//	free(ptPostData);	
	
	printf("******end GetMeterFirmware******\r\n");
}
