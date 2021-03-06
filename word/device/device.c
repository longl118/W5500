#include "device.h"
#include "config.h"
#include "socket.h"
#include "ult.h"
#include "w5500.h"
#include <stdio.h> 
#include <string.h>
#include "flash.h"
#include "spi2.h"
CONFIG_MSG  ConfigMsg, RecvMsg;

uint8 txsize[MAX_SOCK_NUM] = {2,2,2,2,2,2,2,2};
uint8 rxsize[MAX_SOCK_NUM] = {2,2,2,2,2,2,2,2};
extern uint8 MAC[6];//public buffer for DHCP, DNS, HTTP
//public buffer for DHCP, DNS, HTTP
uint8 pub_buf[1460];
 
void GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_GPIOB |RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO , ENABLE);
  // Port A output
  GPIO_InitStructure.GPIO_Pin =GPIO_Pin_0|GPIO_Pin_1| GPIO_Pin_2 |GPIO_Pin_3; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
//  GPIO_ResetBits(GPIOA, GPIO_Pin_0);
//  GPIO_ResetBits(GPIOA, GPIO_Pin_1);
//  GPIO_SetBits(GPIOA, GPIO_Pin_2); // led off
//  GPIO_SetBits(GPIOA, GPIO_Pin_3); // led off
  // Port B output;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_SetBits(GPIOB, GPIO_Pin_9);
  // Port C input
//  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
//  GPIO_Init(GPIOC, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;//????flash
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_SetBits(GPIOB, GPIO_Pin_7);	
}

void Reset_W5500(void)
{
  GPIO_ResetBits(GPIOB, WIZ_RESET);
  Delay_us(2);  
  GPIO_SetBits(GPIOB, WIZ_RESET);
  Delay_ms(1600);
}
//reboot 
void reboot(void)
{
  pFunction Jump_To_Application;
  uint32 JumpAddress;
  JumpAddress = *(vu32*) (0x00000004);
  Jump_To_Application = (pFunction) JumpAddress;
  Jump_To_Application();
}

void set_network(void)
{
	uint8 ip[4];
	setSHAR(ConfigMsg.mac);/*????Mac??ַ*/
	setSUBR(ConfigMsg.sub);/*????????????*/
	setGAR(ConfigMsg.gw);/*????Ĭ??????*/
	setSIPR(ConfigMsg.lip);/*????Ip??ַ*/

  //Init. TX & RX Memory size of w5500
	sysinit(txsize, rxsize); /*??ʼ??8??socket*/

	setRTR(2000);/*????????ʱ??ֵ*/
	setRCR(3);/*???????????·??ʹ???*/

	getSIPR (ip);
	printf(" IP : %d.%d.%d.%d\r\n", ip[0],ip[1],ip[2],ip[3]);
	getSUBR(ip);
	printf(" SN : %d.%d.%d.%d\r\n", ip[0],ip[1],ip[2],ip[3]);
	getGAR(ip);
	printf(" GW : %d.%d.%d.%d\r\n", ip[0],ip[1],ip[2],ip[3]);

	printf("???????? : %x,%x,%x,%x,%x,%x\r\n",ConfigMsg.Code[0],ConfigMsg.Code[1],ConfigMsg.Code[2],ConfigMsg.Code[3],ConfigMsg.Code[4],ConfigMsg.Code[5]);
	printf("DTMBƵ?? : %d\r\n", (ConfigMsg.DTMB_Freq[0]<<24)+(ConfigMsg.DTMB_Freq[1]<<16)+(ConfigMsg.DTMB_Freq[2]<<8)+ConfigMsg.DTMB_Freq[3]);
	printf("??ƵƵ?? : %d\r\n", (ConfigMsg.FM_Freq[0]<<8)+ConfigMsg.FM_Freq[1]);
	printf("JTXD????Ƶ?? : %d\r\n", (ConfigMsg.Sx_Freq[0]<<8)+ConfigMsg.Sx_Freq[1]);

}

void set_default(void)
{
  uint8 mac[6]={0x00,0x08,0xdc,0x11,0x11,0x11};
  uint8 lip[4]={192,168,1,111};
  uint8 sub[4]={255,255,255,0};
  uint8 gw[4]={192,168,1,1};
  uint8 dns[4]={8,8,8,8};
  memcpy(ConfigMsg.lip, lip, 4);
  memcpy(ConfigMsg.sub, sub, 4);
  memcpy(ConfigMsg.gw,  gw, 4);
  memcpy(ConfigMsg.mac, mac,6);
  memcpy(ConfigMsg.dns,dns,4);

  ConfigMsg.dhcp=0;
  ConfigMsg.debug=1;
  ConfigMsg.fw_len=0;
  
  ConfigMsg.state=NORMAL_STATE;
  ConfigMsg.sw_ver[0]=FW_VER_HIGH;
  ConfigMsg.sw_ver[1]=FW_VER_LOW;
  
}

void write_config_to_eeprom(void)
{
	STMFLASH_Write(STM32_FLASH_BASE,(u16 *)&ConfigMsg,CONFIG_MSG_LEN);
}
u16 Buf[CONFIG_MSG_LEN];
void get_config(void)
{
  	uint16 i,j;
	for(i=0,j=0;i<CONFIG_MSG_LEN;i+=2,j++)
	{
		Buf[j] = STMFLASH_ReadHalfWord(STM32_FLASH_BASE+i);
	}
	memcpy(&ConfigMsg,Buf,CONFIG_MSG_LEN);
	
}
