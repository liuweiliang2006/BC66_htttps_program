#ifndef __GETANALYSE_H
#define __GETANALYSE_H

#include "includes.h"
#include "MFRC522.h"

void GetCmdEventGroupCreat (void);
void GetAnalyse(uint8_t *ptRecData);
extern void StrToHex(byte *pbDest, char *pszSrc, int nLen);

extern EventGroupHandle_t xGetCmdEventGroup;	
extern REAL_DATA_CALIBRATION_t REAL_DATA_CALIBRATION;
extern REAL_Flow_CALIBRATION_t REAL_Flow_CALIBRATION;
extern CONFIG_GPRS_t CONFIG_GPRS; 
extern update_t update;
extern bool IsSaveUpdate;
//extern RechargePacket_t RechargePacket;

extern bool IsSaveCONFIG_Meter;
extern bool IsSaveCONFIG_GPRS;
extern bool IsSaveCALIBRATION;
extern bool IsSaveFlowCALIBRATION;
//����ָʾGET��־������Ҫ�������Ϣλ
#define GET_CMD_STUP_RESPONSE  (1<<0)  //��Ӧ��get meter settings ����
#define GET_CMD_STUP_REQUIRE  (1<<1)  //������get meter settings ����

#define GET_CMD_CMD_RESPONSE  (1<<2)  //��Ӧ��get meter command ����
#define GET_CMD_CMD_REQUIRE  (1<<3)  //��Ӧ��get meter command ����

#define GET_CMD_UPDATA_RESPONSE  (1<<4)  //��Ӧ��get meter firmware ����
#define GET_CMD_UPDATA_REQUIRE  (1<<5)  //��Ӧ��get meter firmware ����

#endif 