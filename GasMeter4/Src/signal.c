#include "includes.h"

extern bool IsNeedRestart;
QueueHandle_t SendATQueue = NULL; 			//用于表示当前正在与无线通讯模组
SemaphoreHandle_t  Semaphore_Uart_Rec = NULL; //该信号量用于指示串口是否接收到完整数据帧
SemaphoreHandle_t  Semaphore_AT_Response = NULL;
EventGroupHandle_t xCreatedEventGroup = NULL;

EventGroupHandle_t BC66_AT_EventGroup = NULL;
EventGroupHandle_t BC66_AT_Deal = NULL;

uint8_t AT_work_Flag = 0; //该 命令表示现在正在进行AT指令的交互工作，该位为1时，在空闲任务的勾子函数中可进行喂狗。基于HTTPS的AT指令可能会长期不给回复，在这期间应进行喂狗
//#define SEVER_URL "https://ateei9d448.execute-api.eu-west-1.amazonaws.com/"
//#define SEVER_VERSION "testing/"
#define SEVER_URL "https://test.kop4.com/"
#define SEVER_VERSION "meter/"
#define X_LOCALE "X-Locale:dev"
#define X_API_KEY "X-Api-Key: Qg4EIGj5piiDNVb4P7Th8cop60TJU2xmtIKwxo20"
#define HOST "Host:test.kop4.com"


#define URL_LENGTH_ARRAY 24 //at+qhttpurl=26 指令的数组下标
#define URL_ADDR_ARRAY 25 	//URL的地址 指令的数组下标
#define POST_LENGTH_ARRAY 26 //at+qhttppost=152 指令的数组下标
#define POST_DATA_ARRAY 27 //POST数据的长度 指令的数组下标

#define M26GETCOMMANDLEN 4
#define M26POSTCOMMANDLEN 5
#define BC66POSTCOMMANDLEN 5
#define ANALYSIS_PRINT 1


stru_P4_command_t Send_AT_cmd[]={
/***************************************************M26 https AT指令，测试域名非test.kop4.com,方法非自定义头部************************************/
	          //  u8CmdNum  	SendCommand																													pFun
/*0*/			{     	1,			 "AT\r\n",																												Analysis_AT_Cmd					},
/*1*/			{     	2,			 "AT+CSQ\r\n",																										Analysis_CSQ_Cmd				},
/*2*/			{     	3,			 "AT+QIREGAPP\r\n",																								Analysis_QIREGAPP_Cmd		},//设置APN 用户名密码 初始化TCP/IP任务
/*3*/			{     	4,			 "AT+QIACT\r\n",																									Analysis_QIACT_Cmd			},//激活GPRS PDP连接
/*4*/			{     	5,			 "AT+QILOCIP\r\n",																								Analysis_QILOCIP_Cmd		},//获取本地IP地址
/*5*/			{     	6,			 "AT+QSSLCFG=\"sni\",0,1\r\n",																		Analysis_SNI_Cmd				},
/*6*/			{     	7,			 "AT+QSSLCFG=\"https\",1\r\n",																		Analysis_QSSLCFG_Cmd		},//启用HTTPS功能 
/*7*/			{     	8,			 "AT+QSSLCFG=\"httpsctxi\",0\r\n",																Analysis_QSSLCFG_Cmd		},//为HTPS配置SSL上下文索引
/*8*/			{     	9,			 NULL											,																				Analysis_QHTTPURL_Cmd		}, //AT+QHTTPURL=88,60
/*9*/			{     	10,			 NULL,																														Analysis_SEVER_Addr_Cmd	},
/*10*/		{     	11,			 "AT+QHTTPGET=60,120\r\n",																				Analysis_QHTTPGET_Cmd		},  //GET请求
/*11*/		{     	12,			 "AT+QHTTPREAD\r\n",																							Analysis_QHTTPREAD_Cmd	},//读取HTTPS服务器的响应
/*12*/		{     	13,			 "AT+QIDEACT\r\n",																								Analysis_QIDEACT_Cmd		},//关闭当前GPRS/CSD场景
/*13*/		{     	14,			 "at+qhttpcfg=\"CONTENT-TYPE\",\"application/json\"\r\n",					Analysis_QSSLCFG_Cmd		},//设置JSON格式
/*14*/		{     	15,			 NULL,																														Analysis_QHTTPPOST_Cmd	},//AT+QHTTPPOST=272\r\n
/*15*/		{     	16,			 NULL,																														Analysis_POSTDATA_Cmd		}, //POST 指令携带的数据
/*16*/		{     	17,			 "AT+QHTTPCFG=\"requestheader\",1",															Analysis_QHTTPCFG_Cmd			}, //开启自定义头部功能
/*17*/		{     	18,			 "AT+QHTTPCFG=\"requestheader\",0",															Analysis_QHTTPCFG_Cmd			}, //关闭自定义头部功能
/************************************************BC66 https AT指令，测试域名为test.kop4.com,方法为自定义头部***********************************************/
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
uint8_t u8BC66HTTPNum[5] = {19,20,21,22,23}; //用于HTTPS的初始化AT指令数组下标

/* 实现itoa函数的源码 */ 
static char *myitoa(int num,char *str,int radix) 
{  
	/* 索引表 */ 
	char index[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"; 
	unsigned unum; /* 中间变量 */ 
	int i=0,j,k; 
	char temp;
	/* 确定unum的值 */ 
	if(radix==10&&num<0) /* 十进制负数 */ 
	{ 
		unum=(unsigned)-num; 
		str[i++]='-'; 
	} 
	else unum=(unsigned)num; /* 其它情况 */ 
	/* 逆序 */ 
	do  
	{ 
		str[i++]=index[unum%(unsigned)radix]; 
		unum/=radix; 
	}while(unum); 
	str[i]='\0'; 
	/* 转换 */ 
	if(str[0]=='-') k=1; /* 十进制负数 */ 
	else k=0; 
	/* 将原来的“/2”改为“/2.0”，保证当num在16~255之间，radix等于16时，也能得到正确结果 */ 
	 
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
	/* 创建二值信号量，首次创建信号量计数值是0 */
	Semaphore_Uart_Rec = xSemaphoreCreateBinary();
	
	if(Semaphore_Uart_Rec == NULL)
	{
		printf("Semaphore creat failed!\r\n");
			/* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
	}
	Semaphore_AT_Response  = xSemaphoreCreateBinary();
	if(Semaphore_AT_Response == NULL)
	{
		printf("Semaphore creat failed!\r\n");
			/* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
	}	
	SendATQueue = xQueueCreate(1, sizeof(uint8_t));
	if( SendATQueue == 0 )
	{
			printf("create failed\r\n");
	}
	
	/* 创建事件标志组 */
	xCreatedEventGroup = xEventGroupCreate();
	
	if(xCreatedEventGroup == NULL)
	{
			/* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
	}
	BC66_AT_Deal = xEventGroupCreate();
	if(BC66_AT_Deal == NULL)
	{
		printf("create BC66_AT_Deal failed");
			/* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
	}
	
	BC66_AT_EventGroup = xEventGroupCreate();
	if(BC66_AT_EventGroup == NULL)
	{
		printf("create BC66_AT_EventGroup failed");
			/* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
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
	//版本号拼接
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

//该函数用于实现对发送URL（sever）长度的计算，并将计算结果添充到Send_AT_cmd[7].SendCommand
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
	if(cmd_num == Send_AT_cmd[URL_LENGTH_ARRAY].u8CmdNum)  //如果需要填充AT+QHTTPURL指令
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

	if(cmd_num == Send_AT_cmd[POST_LENGTH_ARRAY].u8CmdNum)  //如果需要填充AT+QHTTPPOST=272
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

//向AWS发送GET请求
static ErrorStatus SendGetCommand()
{
	uint8_t i = 0;
	EventBits_t event_value = 0;
	uint8_t u8QIDEACTSendcnt = 0; //用于记录AT+QIDEACT的次数
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
					i = M26GETCOMMANDLEN -1; //遇到AT指令的错误，则让使其在下一次循环当中让其断开
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

//向AWS发送POST请求
static ErrorStatus SendPostCommand()
{
	uint8_t i = 0;
	uint8_t u8Lenth = 0;
	BaseType_t xResult;
	EventBits_t event_value = 0;
	uint8_t u8ErrorFlag = 0;
	uint8_t u8QIDEACTSendcnt = 0; //用于记录AT+QIDEACT的次数
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
			xEventGroupSetBits(BC66_AT_Deal, 1);//告知解析函数 有新的AT指令发送了
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


//M26 SNI功能测试，在GET和POST前要打开SNI功能 对于模块来说只进行一次初始化即可。
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
					xEventGroupSetBits(BC66_AT_Deal, 1);//告知解析函数 有新的AT指令发送了
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
//AT 指令错误处理
//正确返回1，错误返回0。
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
//POST 指令携带的数据错误处理
//正确返回1，错误返回0。
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

//AT+CSQ指令错误处理
//正确返回1，错误返回0。
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

//AT+QIREGAPP指令错误处理
//正确返回1，错误返回0。
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

//AT+QIACT指令错误处理
//正确返回1，错误返回0。
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

//AT+QIACT指令获取本地IP
//正确返回1，错误返回0。
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

//URL地址
//正确返回1，错误返回0。
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

//AT+QHTTPPOST指令
//正确返回1，错误返回0。
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

//AT+QSSLCFG指令
//正确返回1，错误返回0。
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

//AT+QHTTPURL指令
//正确返回1，错误返回0。
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

//AT+QHTTPURL指令
//正确返回1，错误返回0。
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
//AT+QSCLK=0 低功耗解析函数处理
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
			xEventGroupSetBits(BC66_AT_EventGroup,BC66_POSTDATA_BIT); //返回，但是AT+QHTTPPOST 返回的是错误码
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
//postdata 不包括头的数据
//data:格式化的（包括头与数据内容），放入此地址
void EncodePostDataStru(char *url,char *postdata,char **data)
{
//	char data[1024] = {0};
	char data_length[4];
	if(postdata != NULL)  //POST操作
	{
		myitoa(strlen(postdata),data_length,10); //获取传输的数据部分的真实长度，去除\r\n两个字符
		strcat(*data,"POST ");
//		strcat(*data,url);
		strncat(*data,url, strlen(url)-2);  //去除\r\n两个字符
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
		
		strcat(*data,"\r\n");		//发送数据时的的回车键
	}
	else //GET操作
	{
		strcat(*data,"GET ");
		strncat(*data,url, strlen(url)-2);  //去除\r\n两个字符
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
		
		strcat(*data,"\r\n"); //发送数据时的的回车键
	}

}


//postcookingsecsion 发送函数
void  PostCookingSecsion(void)  //SendReportDataPacket
{
	Stru_Sever_Info_t *struSeverInfo;
	uint8_t result = 0 , i = 0; //用于标识，是否响应了当前的指令
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
		Cooking_Session_READ(REAL_DATA_Credit.CookingSessionSendNumber);//发送的时候从开始位置开始读取,发送成功,索引加一
		refreshCookingSessionReport(&CookingSessionReport);		
		
		encodeCookingPacket(&ptPostData,&CookingSessionReport); //组包 cookingsecsion POST的数据内容
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
		
		Send_AT_cmd[URL_ADDR_ARRAY].SendCommand = ptUrl; //URL地址
		u8UrlLength = strlen(ptUrl)-2;
		CmdLength(u8UrlLength,25);  //根据发送URL的长度		获取URL的长度添充AT+QHTTPURL=XX,60
		
		EncodePostDataStru(ptUrl,ptPostData,&ptPostDataStru);
//		ptPost = Post_Data_Cmd( ptPostData);
		Send_AT_cmd[POST_DATA_ARRAY].SendCommand = ptPostDataStru;
		u8UrlLength = strlen(ptPostDataStru)-2;
		CmdLength(strlen(ptPostDataStru)-2,27);  //根据发送POST的长度
		
		ErrorFlag =SendPostCommand();
		
		free(ptMeterID);
		free(struSeverInfo);
		free(ptUrl);
//		free(ptPost);
		free(ptPostDataStru);
		free(Send_AT_cmd[URL_LENGTH_ARRAY].SendCommand);
		free(Send_AT_cmd[POST_LENGTH_ARRAY].SendCommand);		
		free(ptPostData);
		
		if(ErrorFlag != ERROR)   //发送成功，将序列号加1
		{
			REAL_DATA_Credit.CookingSessionSendNumber++;
			REAL_DATA_Credit_Write();//发送完cooking ,保存序号		
			printf("******end PostCookingSecsion******\r\n");
		}
		else  //发送失败不更新，下一周期重新上传
		{
			printf("******PostCookingSecsion fail!******\r\n");
			return ;			
		}
			
		
		
	}
}

//PostMeterStatus 发送函数
void  PostMeterStatus(void)  //SendReportStatePacket
{
	Stru_Sever_Info_t *struSeverInfo;
	uint8_t result = 0 , i = 0; //用于标识，是否响应了当前的指令
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
	encodeMeterStatusPacket(&ptPostData,&reportStatePacket); //组包 cookingsecsion POST的数据内容
	
	struSeverInfo->Sendsever = SEVER_URL;
	u8UrlLength = strlen(struSeverInfo->Sendsever);
	struSeverInfo->SeverVer = SEVER_VERSION;
	struSeverInfo->CardID = "";
	strcat(ptMeterID,"status/");
	strcat(ptMeterID,CONFIG_Meter.MeterNo);
	struSeverInfo->MeterId = ptMeterID;
//	struSeverInfo->MeterId = "meter/status/TZ00000111";
	ptUrl = Sever_Address_GET( struSeverInfo,"");
	
	Send_AT_cmd[URL_ADDR_ARRAY].SendCommand = ptUrl; //URL地址
	u8UrlLength = strlen(ptUrl)-2;
	CmdLength(u8UrlLength,25);  //根据发送URL的长度		获取URL的长度添充AT+QHTTPURL=XX,60
	
	EncodePostDataStru(ptUrl,ptPostData,&ptPostDataStru);
	Send_AT_cmd[POST_DATA_ARRAY].SendCommand = ptPostDataStru;
	u8UrlLength = strlen(ptPostDataStru)-2;
	CmdLength(strlen(ptPostDataStru)-2,27);  //根据发送POST的长度
	
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

//PostMeterWarning 发送函数
void  PostMeterWarning(void)  //SendWarnPacket();
{
	Stru_Sever_Info_t *struSeverInfo;
//	uint8_t result = 0 , i = 0; //用于标识，是否响应了当前的指令
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
	encodeWarningsPacket(&ptPostData,&waringPacket); //组包 cookingsecsion POST的数据内容
	
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
	
	Send_AT_cmd[URL_ADDR_ARRAY].SendCommand = ptUrl; //URL地址
	u8UrlLength = strlen(ptUrl)-2;
	CmdLength(u8UrlLength,25);  //根据发送URL的长度		获取URL的长度添充AT+QHTTPURL=XX,60
	
	EncodePostDataStru(ptUrl,ptPostData,&ptPostDataStru);
//	ptPost = Post_Data_Cmd( ptPostData);
	Send_AT_cmd[POST_DATA_ARRAY].SendCommand = ptPostDataStru;
	u8UrlLength = strlen(ptPostDataStru)-2;
	CmdLength(strlen(ptPostDataStru)-2,27);  //根据发送POST的长度
	
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

//PostMeterHardware 发送函数
void  PostMeterHardware(void)  //SendWarnPacket();
{
	Stru_Sever_Info_t *struSeverInfo;
//	uint8_t result = 0 , i = 0; //用于标识，是否响应了当前的指令
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
	
	Send_AT_cmd[URL_ADDR_ARRAY].SendCommand = ptUrl; //URL地址
	u8UrlLength = strlen(ptUrl)-2;
	CmdLength(u8UrlLength,25);  //根据发送URL的长度		获取URL的长度添充AT+QHTTPURL=XX,60
	
	EncodePostDataStru(ptUrl,ptPostData,&ptPostDataStru);
//	ptPost = Post_Data_Cmd( ptPostData);
	Send_AT_cmd[POST_DATA_ARRAY].SendCommand = ptPostDataStru;
	u8UrlLength = strlen(ptPostDataStru)-2;
	CmdLength(strlen(ptPostDataStru)-2,27);  //根据发送POST的长度
	
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

//PostMeterSettings 发送函数
void  PostMeterSettings(void)  //
{
	Stru_Sever_Info_t *struSeverInfo;
//	uint8_t result = 0 , i = 0; //用于标识，是否响应了当前的指令
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
	
	Send_AT_cmd[URL_ADDR_ARRAY].SendCommand = ptUrl; //URL地址
	u8UrlLength = strlen(ptUrl)-2;
	CmdLength(u8UrlLength,25);  //根据发送URL的长度		获取URL的长度添充AT+QHTTPURL=XX,60
//	printf("length = %d\r\n",strlen(ptPostData));
	EncodePostDataStru(ptUrl,ptPostData,&ptPostDataStru);
	Send_AT_cmd[POST_DATA_ARRAY].SendCommand = ptPostDataStru;
	u8UrlLength = strlen(ptPostDataStru)-2;
	printf("u8UrlLength = %d",u8UrlLength);
	CmdLength(strlen(ptPostDataStru)-2,27);  //根据发送POST的长度
	
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


//GetMeterSettings 发送函数
void  GetMeterSettings(void)  //
{
	Stru_Sever_Info_t *struSeverInfo;
//	uint8_t result = 0 , i = 0; //用于标识，是否响应了当前的指令
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
	struSeverInfo->CardID = ""; //此字段为空，无card_id
	strcat(ptMeterID,"settings/");
	strcat(ptMeterID,CONFIG_Meter.MeterNo);
//	struSeverInfo->MeterId = "/settings/TZ00000525";
	struSeverInfo->MeterId = ptMeterID;
	ptUrl = Sever_Address_GET( struSeverInfo,"");
	
	Send_AT_cmd[URL_ADDR_ARRAY].SendCommand = ptUrl; //URL地址
	u8UrlLength = strlen(ptUrl)-2;
	CmdLength(u8UrlLength,25);  //根据发送URL的长度		获取URL的长度添充AT+QHTTPURL=XX
	
	EncodePostDataStru(ptUrl,NULL,&ptPostDataStru);
	Send_AT_cmd[POST_DATA_ARRAY].SendCommand = ptPostDataStru;
	CmdLength(strlen(ptPostDataStru)-2,27);  //根据发送POST的长度
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

//GetMeterCommand 发送函数
void  GetMeterCommand(void)  //
{
	Stru_Sever_Info_t *struSeverInfo;
//	uint8_t result = 0 , i = 0; //用于标识，是否响应了当前的指令
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
	struSeverInfo->CardID = ""; //此字段为空，无card_id
	strcat(ptMeterID,"command/");
	strcat(ptMeterID,CONFIG_Meter.MeterNo);
//	struSeverInfo->MeterId = "/settings/TZ00000525";
	struSeverInfo->MeterId = ptMeterID;
	ptUrl = Sever_Address_GET( struSeverInfo,"");
	
	Send_AT_cmd[URL_ADDR_ARRAY].SendCommand = ptUrl; //URL地址
	u8UrlLength = strlen(ptUrl)-2;
	CmdLength(u8UrlLength,25);  //根据发送URL的长度		获取URL的长度添充AT+QHTTPURL=XX
	
	EncodePostDataStru(ptUrl,NULL,&ptPostDataStru);
	Send_AT_cmd[POST_DATA_ARRAY].SendCommand = ptPostDataStru;
	CmdLength(strlen(ptPostDataStru)-2,27);  //根据发送POST的长度
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

//GetMeterFirmware 发送函数
void  GetMeterFirmware(void)  //
{
	Stru_Sever_Info_t *struSeverInfo;
//	uint8_t result = 0 , i = 0; //用于标识，是否响应了当前的指令
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
	struSeverInfo->CardID = ""; //此字段为空，无card_id
	strcat(ptMeterID,"firmware/");
	strcat(ptMeterID,CONFIG_Meter.MeterNo);
//	struSeverInfo->MeterId = "/settings/TZ00000525";
	struSeverInfo->MeterId = ptMeterID;
	ptUrl = Sever_Address_GET( struSeverInfo,"");
	
	Send_AT_cmd[URL_ADDR_ARRAY].SendCommand = ptUrl; //URL地址
	u8UrlLength = strlen(ptUrl)-2;
	CmdLength(u8UrlLength,25);  //根据发送URL的长度		获取URL的长度添充AT+QHTTPURL=XX
	
	EncodePostDataStru(ptUrl,NULL,&ptPostDataStru);
	Send_AT_cmd[POST_DATA_ARRAY].SendCommand = ptPostDataStru;
	CmdLength(strlen(ptPostDataStru)-2,27);  //根据发送POST的长度
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
