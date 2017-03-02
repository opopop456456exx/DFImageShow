#include "sys.h"
#include "delay.h"
#include "usart.h" 
#include "led.h" 		 	 
#include "lcd.h"  
#include "key.h"     
//#include "usmart.h" 
#include "malloc.h"
//#include "sdio_sdcard.h"  
#include "w25qxx.h"    
#include "ff.h"  
#include "exfuns.h"   
#include "text.h"
#include "piclib.h"	
#include "timer.h"
#include "string.h"		
#include "touch.h"
#include "math.h"
 
/************************************************
 ALIENTEKս��STM32������ʵ��41
 ͼƬ��ʾ ʵ�� 
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/

u8 f[10];
u8 l[10];
u8 d[15];
u8 lenshow[15],lenshow2[15];	



UINT bww;
//char buf[100];
//char buf1[10]="ffffffff";
	

//�����Ļ�������Ͻ���ʾ"RST"
void Load_Drow_Dialog(void)
{
//	LCD_Clear(WHITE);	//����  
//	ai_load_picfile("0:/PICTURE/dfdf.jpg",0,0,lcddev.width,lcddev.height,1);
		//LCD_Clear(WHITE);	//����   
//	if(ai_load_picfile("1:/picture.jpg",0,0,lcddev.width,lcddev.height,1))
//	{
//		Show_Str(30,170,240,16," xianshicuowu",16,0);
//	
//	}
	//DF_ShowPic((u8 *)gImage_5565);
 	POINT_COLOR=BLUE;	//��������Ϊ��ɫ 
	LCD_ShowString(lcddev.width-24,0,200,16,16,"RST");//��ʾ��������
	LCD_DrawRectangle(lcddev.width-48, 48, lcddev.width,0);
  	POINT_COLOR=RED;	//���û�����ɫ 
}
////////////////////////////////////////////////////////////////////////////////
//���ݴ�����ר�в���
//��ˮƽ��
//x0,y0:����
//len:�߳���
//color:��ɫ
void gui_draw_hline(u16 x0,u16 y0,u16 len,u16 color)
{
	if(len==0)return;
	LCD_Fill(x0,y0,x0+len-1,y0,color);	
}
//��ʵ��Բ
//x0,y0:����
//r:�뾶
//color:��ɫ
void gui_fill_circle(u16 x0,u16 y0,u16 r,u16 color)
{											  
	u32 i;
	u32 imax = ((u32)r*707)/1000+1;
	u32 sqmax = (u32)r*(u32)r+(u32)r/2;
	u32 x=r;
	gui_draw_hline(x0-r,y0,2*r,color);
	for (i=1;i<=imax;i++) 
	{
		if ((i*i+x*x)>sqmax)// draw lines from outside  
		{
 			if (x>imax) 
			{
				gui_draw_hline (x0-i+1,y0+x,2*(i-1),color);
				gui_draw_hline (x0-i+1,y0-x,2*(i-1),color);
			}
			x--;
		}
		// draw lines from inside (center)  
		gui_draw_hline(x0-x,y0+i,2*x,color);
		gui_draw_hline(x0-x,y0-i,2*x,color);
	}
}  
//������֮��ľ���ֵ 
//x1,x2����ȡ��ֵ��������
//����ֵ��|x1-x2|
u16 my_abs(u16 x1,u16 x2)
{			 
	if(x1>x2)return x1-x2;
	else return x2-x1;
}  
//��һ������
//(x1,y1),(x2,y2):��������ʼ����
//size�������Ĵ�ϸ�̶�
//color����������ɫ
void lcd_draw_bline(u16 x1, u16 y1, u16 x2, u16 y2,u8 size,u16 color)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	if(x1<size|| x2<size||y1<size|| y2<size)return; 
	delta_x=x2-x1; //������������ 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //���õ������� 
	else if(delta_x==0)incx=0;//��ֱ�� 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//ˮƽ�� 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //ѡȡ�������������� 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//������� 
	{  
		gui_fill_circle(uRow,uCol,size,color);//���� 
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
}   
////////////////////////////////////////////////////////////////////////////////
 //5�����ص����ɫ												 




const u16 POINT_COLOR_TBL[CT_MAX_TOUCH]={RED,GREEN,BLUE,BROWN,GRED};  
//���败�������Ժ���
int count=0,len=0;
void rtp_test(void)
{
	u8 key;
	u8 i=0;	  
	
	//�ȳ�ʼ��״̬һ��
	LCD_Clear(WHITE);	//����   
					Load_Drow_Dialog();//���
					tp_dev.c2=0;
					tp_dev.sta=0;
	
	
	while(1)
	{
		
			if(USART_RX_STA&0x80000)
		{			

			
			len=USART_RX_STA&0x1ffff;//�õ��˴ν��յ������ݳ���
			//printf("len=%d,USART_RX_BUF[3906],USART_RX_BUF[3907],%x,%x\n",len,USART_RX_BUF[3906],USART_RX_BUF[3907]);
		//	delay_ms(10);
			
			
			
//			
//			
//			 while(f_open (&fil,"1:/picture.jpg", FA_CREATE_ALWAYS|FA_WRITE|FA_READ))
//	{
//	Show_Str(30,170,240,16,"�����ļ�����!",16,0);
//		delay_ms(200);				  
//		LCD_Fill(30,170,240,186,WHITE);//�����ʾ	     
//		delay_ms(200);		
//}
//	
//	
//	f_write (&fil, USART_RX_BUF, len, &CRes);/////////////////////////////////////////////




	   printf("%d\n",len);
		//printf("hh\n");
//			LCD_ShowString(60,150,200,16,16,USART_RX_BUF);	
		 sprintf((char *)lenshow,"LEN=%d",USART_RX_LEN);
			LCD_ShowString(60,300,200,16,16,lenshow);
       //printf("sdlen=%d\n",(int)CRes);
			 printf("sdlen=%d\n",(int)f_size(&fil));
		//	f_close(&fil); //f_close �ܹؼ���д����������close����ʾ�Ż�����
		
		LCD_Clear(WHITE);	//����   
	if(ai_load_picfile("1:/picture.jpg",0,0,lcddev.width,lcddev.height,1))
	{
		Show_Str(30,170,240,16," xianshicuowu",16,0);
	
	}
		USART_RX_STA=0;
	
		
	
		
	//��ʾͼƬ  
		}
		
		
		//DF_ShowPic();
	 	key=KEY_Scan(0);
		tp_dev.scan(0); 		 
		if(tp_dev.sta&TP_PRES_DOWN)			//������������
		{	
		 	if(tp_dev.x[0]<lcddev.width&&tp_dev.y[0]<lcddev.height)
			{	
				if(tp_dev.x[0]>(lcddev.width-48)&&tp_dev.y[0]<48){
					LCD_Clear(WHITE);	//����   
					Load_Drow_Dialog();//���
					if(ai_load_picfile("1:/picture.jpg",0,0,lcddev.width,lcddev.height,1))
	        {
		       Show_Str(30,170,240,16," xianshicuowu",16,0);
	
	        }
					tp_dev.c2=0;
					tp_dev.sta=0;
				}
				else if(tp_dev.c2==0)
				{
					Load_Drow_Dialog();//���
					TP_Draw_Big_Point(tp_dev.x[0],tp_dev.y[0],BLACK);		//��ͼ	
				//	TP_Draw_Big_Point(tp_dev.x[1],tp_dev.y[1],WHITE);		//��ͼ   �����һ����
					sprintf((char *)f,"(%d,%d)",tp_dev.x[0],tp_dev.y[0]);
						LCD_ShowString(tp_dev.x[0]+16,tp_dev.y[0],200,16,16,f);	
					//	LCD_ShowString(tp_dev.x[1]+16,tp_dev.y[1],200,16,16,"       ");	
				}					
				else if(tp_dev.c2==1)
				{
					Load_Drow_Dialog();//���
					
					
					TP_Draw_Big_Point(tp_dev.x[1],tp_dev.y[1],BLACK);		//��ͼ	
				
						LCD_ShowString(tp_dev.x[1]+16,tp_dev.y[1],200,16,16,f);	
					
					TP_Draw_Big_Point(tp_dev.x[0],tp_dev.y[0],RED);		//��ͼ	
			//		TP_Draw_Big_Point(tp_dev.x[2],tp_dev.y[2],WHITE);		//��ͼ   �����һ����
					
					sprintf((char *)l,"(%d,%d)",tp_dev.x[0],tp_dev.y[0]);
						LCD_ShowString(tp_dev.x[0]+16,tp_dev.y[0],200,16,16,l);	
			//		LCD_ShowString(tp_dev.x[2]+16,tp_dev.y[2],200,16,16,"       ");	
					
				 	 
					
					LCD_DrawLine(tp_dev.x[1],tp_dev.y[1],tp_dev.x[0],tp_dev.y[0],RED);
			//		 LCD_DrawLine(tp_dev.x[1],tp_dev.y[1],tp_dev.x[2],tp_dev.y[2],WHITE);
					
					sprintf((char *)d,"d=(%d,%d)",tp_dev.x[1]-tp_dev.x[0],tp_dev.y[1]-tp_dev.y[0]);
					LCD_ShowString((tp_dev.x[0]+tp_dev.x[1])/2,(tp_dev.y[0]+tp_dev.y[1])/2,200,16,16,d);
					//LCD_ShowString((tp_dev.x[2]+tp_dev.x[1])/2,(tp_dev.y[2]+tp_dev.y[1])/2,200,16,16,"       ");	
				}					
				else if(tp_dev.c2==2)
				{
					Load_Drow_Dialog();//���
				//	tp_dev.c2=0;
				
					tp_dev.x[1]=0;
			    tp_dev.y[1]=0;
			    tp_dev.x[2]=0;
			    tp_dev.y[2]=0;
					
				}					
			}
			
		}
		else delay_ms(10);	//û�а������µ�ʱ�� 	  
		
		if(key==KEY0_PRES)	//KEY0����,��ִ��У׼����
		{
			LCD_Clear(WHITE);//����
		    TP_Adjust();  	//��ĻУ׼  
			Load_Drow_Dialog();
		}
		//i++;
		//if(i%20==0)LED0=!LED0;
	}
}
//���ݴ��������Ժ���
void ctp_test(void)
{
	u8 t=0;
	u8 i=0;	  	    
 	u16 lastpos[5][2];		//��¼���һ�ε����� 
	while(1)
	{
		tp_dev.scan(0);
		for(t=0;t<CT_MAX_TOUCH;t++)
		{
			if((tp_dev.sta)&(1<<t))
			{
				if(tp_dev.x[t]<lcddev.width&&tp_dev.y[t]<lcddev.height)
				{
					if(lastpos[t][0]==0XFFFF)
					{
						lastpos[t][0] = tp_dev.x[t];
						lastpos[t][1] = tp_dev.y[t];
					}
					lcd_draw_bline(lastpos[t][0],lastpos[t][1],tp_dev.x[t],tp_dev.y[t],2,POINT_COLOR_TBL[t]);//����
					lastpos[t][0]=tp_dev.x[t];
					lastpos[t][1]=tp_dev.y[t];
					if(tp_dev.x[t]>(lcddev.width-24)&&tp_dev.y[t]<16)
					{
						Load_Drow_Dialog();//���
					}
				}
			}else lastpos[t][0]=0XFFFF;
		}
		
		delay_ms(5);i++;
		if(i%20==0)LED0=!LED0;
	}	


}

//�õ�path·����,Ŀ���ļ����ܸ���
//path:·��		    
//����ֵ:����Ч�ļ���
u16 pic_get_tnum(u8 *path)
{	  
	u8 res;
	u16 rval=0;
 	DIR tdir;	 		//��ʱĿ¼
	FILINFO tfileinfo;	//��ʱ�ļ���Ϣ	
	u8 *fn;	 			 			   			     
    res=f_opendir(&tdir,(const TCHAR*)path); 	//��Ŀ¼
  	tfileinfo.lfsize=_MAX_LFN*2+1;				//���ļ�����󳤶�
	tfileinfo.lfname=mymalloc(SRAMIN,tfileinfo.lfsize);//Ϊ���ļ������������ڴ�
	if(res==FR_OK&&tfileinfo.lfname!=NULL)
	{
		while(1)//��ѯ�ܵ���Ч�ļ���
		{
	        res=f_readdir(&tdir,&tfileinfo);       		//��ȡĿ¼�µ�һ���ļ�
	        if(res!=FR_OK||tfileinfo.fname[0]==0)break;	//������/��ĩβ��,�˳�		  
     		fn=(u8*)(*tfileinfo.lfname?tfileinfo.lfname:tfileinfo.fname);			 
			res=f_typetell(fn);	
			if((res&0XF0)==0X50)//ȡ����λ,�����ǲ���ͼƬ�ļ�	
			{
				rval++;//��Ч�ļ�������1
			}	    
		}  
	} 
	return rval;
}



int main(void)
{		
	u8 res;
	 	u16 len;
	UINT CRes=0;
 	DIR picdir;	 		//ͼƬĿ¼
	FILINFO picfileinfo;//�ļ���Ϣ
	u8 *fn;   			//���ļ���
	u8 *pname;			//��·�����ļ���
	u16 totpicnum; 		//ͼƬ�ļ�����
	u16 curindex;		//ͼƬ��ǰ����
	u8 key;				//��ֵ
	u8 pause=0;			//��ͣ���
	u8 t,i;
	u16 temp;
	u16 *picindextbl;	//ͼƬ������ 
	
	delay_init();	    	 //��ʱ������ʼ��	  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	 
	 TIM3_Int_Init(499,7199);//10Khz�ļ���Ƶ�ʣ�������5000Ϊ500ms  
	uart_init(115200);	 	//���ڳ�ʼ��Ϊ115200
 //	usmart_dev.init(72);		//��ʼ��USMART		
 	LED_Init();		  			//��ʼ����LED���ӵ�Ӳ���ӿ�
	KEY_Init();					//��ʼ������
	LCD_Init();			   		//��ʼ��LCD   
	
	 	tp_dev.init();
	
	W25QXX_Init();				//��ʼ��W25Q128
 	my_mem_init(SRAMIN);		//��ʼ���ڲ��ڴ��
	exfuns_init();				//Ϊfatfs��ر��������ڴ�  
 //	f_mount(fs[0],"0:",1); 		//����SD�� 
 		res=f_mount(fs[1],"1:",1); 		//����FLASH.
	

POINT_COLOR=RED;      
	while(font_init()) 		//����ֿ�
	{	    
		LCD_ShowString(30,50,200,16,16,"Font Error!");
		delay_ms(200);				  
		LCD_Fill(30,50,240,66,WHITE);//�����ʾ	     
		delay_ms(200);				  
	}  	 
//	res = f_mkdir("sub1");
//    if (res) die(res);
 	Show_Str(30,50,200,16,"WarShip STM32F1������",16,0);				    	 
	Show_Str(30,70,200,16,"ͼƬ��ʾ����",16,0);				    	 
	Show_Str(30,90,200,16,"KEY0:NEXT KEY2:PREV",16,0);				    	 
	Show_Str(30,110,200,16,"KEY_UP:PAUSE",16,0);				    	 
	Show_Str(30,130,200,16,"����ԭ��@ALIENTEK",16,0);				    	 
	Show_Str(30,150,200,16,"2015��1��20��",16,0);

	Show_Str(30,170,240,16,"��ʼ��ʾ...",16,0); 
	delay_ms(1500);
	piclib_init();										//��ʼ����ͼ	   	   
	curindex=0;											//��0��ʼ��ʾ
	while(f_open (&fil,"1:/picture.jpg", FA_OPEN_ALWAYS|FA_WRITE|FA_READ))
	{
	Show_Str(30,170,240,16,"�����ļ�����!",16,0);
		delay_ms(200);				  
		LCD_Fill(30,170,240,186,WHITE);//�����ʾ	     
		delay_ms(200);		
}
	
	while(1)
	{
		
	
		rtp_test(); 						//����������;
		
		Show_Str(2,2,240,16,"0:/PICTURE/picture.jpg",16,1); 				//��ʾͼƬ����
	}
	
	myfree(SRAMIN,picfileinfo.lfname);	//�ͷ��ڴ�			    
	myfree(SRAMIN,pname);				//�ͷ��ڴ�			    
	myfree(SRAMIN,picindextbl);			//�ͷ��ڴ�		
}









