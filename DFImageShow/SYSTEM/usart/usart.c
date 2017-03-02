#include "sys.h"
#include "usart.h"	  
#include "ff.h" 
////////////////////////////////////////////////////////////////////////////////// 	 
//���ʹ��ucos,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos ʹ��	  
#endif
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������
//����1��ʼ��		   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/8/18
//�汾��V1.5
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved
//********************************************************************************
//V1.3�޸�˵�� 
//֧����Ӧ��ͬƵ���µĴ��ڲ���������.
//�����˶�printf��֧��
//�����˴��ڽ��������.
//������printf��һ���ַ���ʧ��bug
//V1.4�޸�˵��
//1,�޸Ĵ��ڳ�ʼ��IO��bug
//2,�޸���USART_RX_STA,ʹ�ô����������ֽ���Ϊ2��14�η�
//3,������USART_REC_LEN,���ڶ��崮�����������յ��ֽ���(������2��14�η�)
//4,�޸���EN_USART1_RX��ʹ�ܷ�ʽ
//V1.5�޸�˵��
//1,�����˶�UCOSII��֧��
////////////////////////////////////////////////////////////////////////////////// 	  
 
 
 
 

//////////////////////////////////////////////////////////////////
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
_sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 

/*ʹ��microLib�ķ���*/
 /* 
int fputc(int ch, FILE *f)
{
	USART_SendData(USART1, (uint8_t) ch);

	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) {}	
   
    return ch;
}
int GetKey (void)  { 

    while (!(USART1->SR & USART_FLAG_RXNE));

    return ((int)(USART1->DR & 0x1FF));
}
*/
 
#if EN_USART1_RX   //���ʹ���˽���
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	


u8 USART_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
u8 *USART_RX_BUF_PT=USART_RX_BUF;
u8 USART_RX_HEAD_BUF[8];            //֡ͷ���뻺�壬Э��8�ֽ�
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
FIL fil;
u32 USART_RX_STA=0,USART_RX_LEN=0;       //����״̬���	,��������ֽ���  
u8 RxTimeout=0;             //���ճ�ʱ ��tim3��ʱ��50ms��150msû�н��봮�ڽ����ж���Ϊ��ʱ��Ŀǰд�Ŀ�����bug��û����

uint8_t w = 0;
  
void uart_init(u32 bound){
  //GPIO�˿�����
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��USART1��GPIOAʱ��
  
	//USART1_TX   GPIOA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
  GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.9
   
  //USART1_RX	  GPIOA.10��ʼ��
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
  GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.10  

  //Usart1 NVIC ����
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
   //USART ��ʼ������

	USART_InitStructure.USART_BaudRate = bound;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

  USART_Init(USART1, &USART_InitStructure); //��ʼ������1
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�������ڽ����ж�
  USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ���1 

}



//////////////////////////////////////////////////////////////////////////////////////////////////

void USART1_IRQHandler(void)                	//����1�жϷ������
	{
		
	u8 Res;
		UINT CRes;
#if SYSTEM_SUPPORT_OS 		//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntEnter();    
#endif
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
		{
			if(RxTimeout==0)
			{
				USART_RX_STA=0;
				w=0;
			//	CRes = f_lseek(&fil, 0);
			}
			RxTimeout=3;	
			
			
		Res =USART_ReceiveData(USART1);	//��ȡ���յ�������
		
		//USART_SendData(USART1, Res);//�򴮿�1��������
	//	while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//�ȴ����ͽ���
			

			if((USART_RX_STA&0x20000)==0)
			{
				USART_RX_HEAD_BUF[w]=Res;
			w++;
			if(w==8)
			{
				if((USART_RX_HEAD_BUF[0]== 0xDF)&&(USART_RX_HEAD_BUF[1]==0xDF)&&(USART_RX_HEAD_BUF[2]==0xFD)&&(USART_RX_HEAD_BUF[3]==0xFD))    //֡ͷDFDFFDFD
				{
					USART_RX_LEN=((u32)USART_RX_HEAD_BUF[4]<<24)|((u32)USART_RX_HEAD_BUF[5]<<16)|((u32)USART_RX_HEAD_BUF[6]<<8)|((u32)USART_RX_HEAD_BUF[7]);
					if(USART_RX_LEN!=0)
					{
						USART_RX_STA|=0x20000;                      //֡ͷУ��ɹ�
					}
				}
				w=0;
			}
		}
			else                                               //֡ͷУ��ɹ�,��ʼ��������
			{
				if((USART_RX_STA&0x40000)==0)
				{
				USART_RX_BUF[USART_RX_STA&0X1FFFF]=Res ;
					// f_write (&fil, Res, 1, &CRes);       ֱ�Ӷ����У���ʱ�����ж�ͨ��
					USART_RX_STA++;
			 
					
				
				
				if((USART_RX_STA&0X1FFFF)==USART_RX_LEN)
				{
					//if(f_size(&fil)==USART_RX_LEN)
           USART_RX_STA|=0x40000;                         //���ݽ������ˣ�  ����  ĩβoxod,oxoa��У��
				}
			}
				else
				{
					USART_RX_HEAD_BUF[w]=Res;
					w++;
					if(w==2)
					{
						if((USART_RX_HEAD_BUF[0]== 0x0d)&&(USART_RX_HEAD_BUF[1]==0x0a))
						{
							while(f_open (&fil,"1:/picture.jpg", FA_CREATE_ALWAYS|FA_WRITE|FA_READ))
							{
								;
							}
	
							f_write (&fil, USART_RX_BUF, USART_RX_LEN, &CRes);
							f_close(&fil); //f_close �ܹؼ���д����������close����ʾ�Ż�����,��Ϊÿ��Ҫ���´򿪴�����
							USART_RX_STA|=0x80000;
							w=0;
						}
						else                        //�������
						{
							//CRes = f_lseek(&fil, 0);
							w=0;
							USART_RX_STA=0;
						}
					}
					
				}
			}
			
			
			
		}
		
		
		
		
	 
		
 
//          USART_RX_BUF[USART_RX_STA&0X3FFFF]=Res ;
//					USART_RX_STA++;
//					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//�������ݴ���,���¿�ʼ����	  
//          if(USART_RX_STA==(13490+12-1))USART_RX_STA|=0x80000;
//				}
#if SYSTEM_SUPPORT_OS 	//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntExit();  											 
#endif
} 

//void USART1_IRQHandler(void)                	//����1�жϷ������
//	{
//	u8 Res;
//#if SYSTEM_SUPPORT_OS 		//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
//	OSIntEnter();    
//#endif
//	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
//		{
//		Res =USART_ReceiveData(USART1);	//��ȡ���յ�������
//		
//		if((USART_RX_STA&0x8000)==0)//����δ���
//			{
//			if(USART_RX_STA&0x4000)//���յ���0x0d
//				{
//				if(Res!=0x0a)USART_RX_STA=0;//���մ���,���¿�ʼ
//				else USART_RX_STA|=0x8000;	//��������� 
//				}
//			else //��û�յ�0X0D
//				{	
//				if(Res==0x0d)USART_RX_STA|=0x4000;
//				else
//					{
//					USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
//					USART_RX_STA++;
//					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//�������ݴ���,���¿�ʼ����	  
//					}		 
//				}
//			}   		 
//     } 
//#if SYSTEM_SUPPORT_OS 	//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
//	OSIntExit();  											 
//#endif
//} 
#endif	

