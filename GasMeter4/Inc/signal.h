#ifndef __SIGNAL_H
#define __SIGNAL_H

#include "includes.h"

/********************M26 AT command flags start**************************/
/*AT指令错误标志位 位置定义，指示不同的AT指令，对应的事件标志组位置*/
#define AT_BIT 					(1<<0) 	//AT
#define CSQ_BIT 				(1<<1)	//AT+CSQ
#define QIREGAPP_BIT 		(1<<2)	//AT+QIREGAPP
#define QIACT_BIT 			(1<<3)	//AT+QIACT
#define QILOCIP_BIT 		(1<<4)	//AT+QILOCIP
#define SNI_BIT 				(1<<5)	//AT+SNI
#define QSSLCFG_BIT 		(1<<6)	//AT+QSSLCFG
#define QHTTPURL_BIT 		(1<<7)	//AT+QHTTPURL
#define SEVERADDR_BIT 	(1<<8)	//URL address
#define QHTTPGET_BIT 		(1<<9)	//AT+QHTTPGET
#define QHTTPREAD_BIT 	(1<<10)	//AT+QHTTPREAD
#define QIDEACT_BIT 		(1<<11)	//AT+QIDEACT
#define QHTTPPOST_BIT 	(1<<12)	//AT+QHTTPPOST
#define POSTDATA_BIT 		(1<<13)	//AT+POSTDATA
#define QHTTPCFG_BIT		(1<<14) //AT+QHTTPCFG

#define ALL_AT_BIT			(1<<23) //有任意一AT指令错误，该位置位
/*********************end M26 command*************************************/

/********************BC66 AT command flags**************************/
#define BC66_QSCLK_BIT 					(1<<0) 	//AT+QSCLK
#define BC66_QSSLCFG_BIT 				(1<<1) 	//AT+QSSLCFG=1,5,"seclevel",0;AT+QSSLCFG=1,5,"sni",1;AT+QSSLCFG=1,5,"debug",4
#define BC66_QHTTPCFG_BIT 			(1<<2) 	//AT+QHTTPCFG="ssl",1,5;at+qhttpcfg="requestheader",1
#define BC66_QHTTPURL_BIT 			(1<<3) 	//at+qhttpurl=46
#define BC66_QHTTPPOST_BIT 			(1<<4) 	//at+qhttppost=152
#define BC66_QHTTPREAD_BIT 			(1<<5) 	//at+qhttpread=1024
#define BC66_SEVERADDR_BIT 			(1<<6)	//URL address
#define BC66_POSTDATA_BIT 			(1<<7)	//APOSTDATA

#define BC66_ALL_AT_BIT					(1<<23) //有任意一AT指令错误，该位置位
/********************end BC66 command**************************/


typedef uint8_t (*Cmd_Analysis)(char *pCmd);
//add by lwl
typedef struct SeverInfo
{
	char *Sendsever; //http://virtserver.swaggerhub.com/KopaTechnology/KTMeterControl/
	char *MeterId;	//meter/{meterId}/
	char *SeverVer;//1.0.0/
	char *CardID;//card/{cardId}/
//	char Type[]
} Stru_Sever_Info_t;
//add by lwl
typedef struct ATInfo{
	uint8_t  u8CmdNum;
	char     *SendCommand;
//	char     ReceiveAnswer[128];
	Cmd_Analysis pFun;
}stru_P4_command_t;

extern stru_P4_command_t Send_AT_cmd[];
extern QueueHandle_t SendATQueue;
extern SemaphoreHandle_t  Semaphore_Uart_Rec;
extern SemaphoreHandle_t  Semaphore_AT_Response;
extern EventGroupHandle_t xCreatedEventGroup;  //事件标志组，用于指示AT指令的错误，遇错误置相应位
extern EventGroupHandle_t BC66_AT_EventGroup;  //BC66 AT指令事件标志
void AppObjCreate (void);
void M26_HTTP_Init(void );
void BC66_HTTP_Init(void );
void  PostCookingSecsion(void);
void  PostMeterStatus(void);
void  PostMeterWarning(void);
void  PostMeterHardware(void);
void  PostMeterSettings(void);
void  GetMeterSettings(void);

/**************M26 https analysis function*********************/
uint8_t Analysis_AT_Cmd(char *pdata);
uint8_t Analysis_CSQ_Cmd(char *pdata);
uint8_t Analysis_QIREGAPP_Cmd(char *pdata);
uint8_t Analysis_QIACT_Cmd(char *pdata);
uint8_t Analysis_QILOCIP_Cmd(char *pdata);
uint8_t Analysis_SNI_Cmd(char *pdata);
uint8_t Analysis_QSSLCFG_Cmd(char *pdata);
uint8_t Analysis_QHTTPURL_Cmd(char *pdata);
uint8_t Analysis_QHTTPGET_Cmd(char *pdata);
uint8_t Analysis_QHTTPREAD_Cmd(char *pdata);
uint8_t Analysis_QIDEACT_Cmd(char *pdata);
uint8_t Analysis_SEVER_Addr_Cmd(char *pdata);
uint8_t Analysis_QHTTPPOST_Cmd(char *pdata);
uint8_t Analysis_POSTDATA_Cmd(char *pdata);
uint8_t Analysis_POSTDATA_Cmd(char *pdata);
uint8_t Analysis_QHTTPCFG_Cmd(char *pdata);

/**************BC66 https analysis function*********************/
uint8_t BC66_Analysis_QSCLK_Cmd(char *pdata);
uint8_t BC66_Analysis_QSSLCFG_Cmd(char *pdata);
uint8_t BC66_Analysis_QHTTPCFG_Cmd(char *pdata);
uint8_t BC66_Analysis_QHTTPURL_Cmd(char *pdata);
uint8_t BC66_Analysis_POSTDATA_Cmd(char *pdata);
uint8_t BC66_Analysis_QHTTPREAD_Cmd(char *pdata);
uint8_t BC66_Analysis_SEVER_Addr_Cmd(char *pdata);
uint8_t BC66_Analysis_QHTTPPOST_Cmd(char *pdata);
#endif