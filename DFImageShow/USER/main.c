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
 ALIENTEK战舰STM32开发板实验41
 图片显示 实验 
 技术支持：www.openedv.com
 淘宝店铺：http://eboard.taobao.com 
 关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 广州市星翼电子科技有限公司  
 作者：正点原子 @ALIENTEK
************************************************/

u8 f[10];
u8 l[10];
u8 d[15];
u8 lenshow[15],lenshow2[15];	



UINT bww;
//char buf[100];
//char buf1[10]="ffffffff";
	

//清空屏幕并在右上角显示"RST"
void Load_Drow_Dialog(void)
{
//	LCD_Clear(WHITE);	//清屏  
//	ai_load_picfile("0:/PICTURE/dfdf.jpg",0,0,lcddev.width,lcddev.height,1);
		//LCD_Clear(WHITE);	//清屏   
//	if(ai_load_picfile("1:/picture.jpg",0,0,lcddev.width,lcddev.height,1))
//	{
//		Show_Str(30,170,240,16," xianshicuowu",16,0);
//	
//	}
	//DF_ShowPic((u8 *)gImage_5565);
 	POINT_COLOR=BLUE;	//设置字体为蓝色 
	LCD_ShowString(lcddev.width-24,0,200,16,16,"RST");//显示清屏区域
	LCD_DrawRectangle(lcddev.width-48, 48, lcddev.width,0);
  	POINT_COLOR=RED;	//设置画笔蓝色 
}
////////////////////////////////////////////////////////////////////////////////
//电容触摸屏专有部分
//画水平线
//x0,y0:坐标
//len:线长度
//color:颜色
void gui_draw_hline(u16 x0,u16 y0,u16 len,u16 color)
{
	if(len==0)return;
	LCD_Fill(x0,y0,x0+len-1,y0,color);	
}
//画实心圆
//x0,y0:坐标
//r:半径
//color:颜色
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
//两个数之差的绝对值 
//x1,x2：需取差值的两个数
//返回值：|x1-x2|
u16 my_abs(u16 x1,u16 x2)
{			 
	if(x1>x2)return x1-x2;
	else return x2-x1;
}  
//画一条粗线
//(x1,y1),(x2,y2):线条的起始坐标
//size：线条的粗细程度
//color：线条的颜色
void lcd_draw_bline(u16 x1, u16 y1, u16 x2, u16 y2,u8 size,u16 color)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	if(x1<size|| x2<size||y1<size|| y2<size)return; 
	delta_x=x2-x1; //计算坐标增量 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //设置单步方向 
	else if(delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//水平线 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//画线输出 
	{  
		gui_fill_circle(uRow,uCol,size,color);//画点 
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
 //5个触控点的颜色												 




const u16 POINT_COLOR_TBL[CT_MAX_TOUCH]={RED,GREEN,BLUE,BROWN,GRED};  
//电阻触摸屏测试函数
int count=0,len=0;
void rtp_test(void)
{
	u8 key;
	u8 i=0;	  
	
	//先初始化状态一遍
	LCD_Clear(WHITE);	//清屏   
					Load_Drow_Dialog();//清除
					tp_dev.c2=0;
					tp_dev.sta=0;
	
	
	while(1)
	{
		
			if(USART_RX_STA&0x80000)
		{			

			
			len=USART_RX_STA&0x1ffff;//得到此次接收到的数据长度
			//printf("len=%d,USART_RX_BUF[3906],USART_RX_BUF[3907],%x,%x\n",len,USART_RX_BUF[3906],USART_RX_BUF[3907]);
		//	delay_ms(10);
			
			
			
//			
//			
//			 while(f_open (&fil,"1:/picture.jpg", FA_CREATE_ALWAYS|FA_WRITE|FA_READ))
//	{
//	Show_Str(30,170,240,16,"创建文件错误!",16,0);
//		delay_ms(200);				  
//		LCD_Fill(30,170,240,186,WHITE);//清除显示	     
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
		//	f_close(&fil); //f_close 很关键，写完数据立刻close，显示才会正常
		
		LCD_Clear(WHITE);	//清屏   
	if(ai_load_picfile("1:/picture.jpg",0,0,lcddev.width,lcddev.height,1))
	{
		Show_Str(30,170,240,16," xianshicuowu",16,0);
	
	}
		USART_RX_STA=0;
	
		
	
		
	//显示图片  
		}
		
		
		//DF_ShowPic();
	 	key=KEY_Scan(0);
		tp_dev.scan(0); 		 
		if(tp_dev.sta&TP_PRES_DOWN)			//触摸屏被按下
		{	
		 	if(tp_dev.x[0]<lcddev.width&&tp_dev.y[0]<lcddev.height)
			{	
				if(tp_dev.x[0]>(lcddev.width-48)&&tp_dev.y[0]<48){
					LCD_Clear(WHITE);	//清屏   
					Load_Drow_Dialog();//清除
					if(ai_load_picfile("1:/picture.jpg",0,0,lcddev.width,lcddev.height,1))
	        {
		       Show_Str(30,170,240,16," xianshicuowu",16,0);
	
	        }
					tp_dev.c2=0;
					tp_dev.sta=0;
				}
				else if(tp_dev.c2==0)
				{
					Load_Drow_Dialog();//清除
					TP_Draw_Big_Point(tp_dev.x[0],tp_dev.y[0],BLACK);		//画图	
				//	TP_Draw_Big_Point(tp_dev.x[1],tp_dev.y[1],WHITE);		//画图   清除上一画面
					sprintf((char *)f,"(%d,%d)",tp_dev.x[0],tp_dev.y[0]);
						LCD_ShowString(tp_dev.x[0]+16,tp_dev.y[0],200,16,16,f);	
					//	LCD_ShowString(tp_dev.x[1]+16,tp_dev.y[1],200,16,16,"       ");	
				}					
				else if(tp_dev.c2==1)
				{
					Load_Drow_Dialog();//清除
					
					
					TP_Draw_Big_Point(tp_dev.x[1],tp_dev.y[1],BLACK);		//画图	
				
						LCD_ShowString(tp_dev.x[1]+16,tp_dev.y[1],200,16,16,f);	
					
					TP_Draw_Big_Point(tp_dev.x[0],tp_dev.y[0],RED);		//画图	
			//		TP_Draw_Big_Point(tp_dev.x[2],tp_dev.y[2],WHITE);		//画图   清除上一画面
					
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
					Load_Drow_Dialog();//清除
				//	tp_dev.c2=0;
				
					tp_dev.x[1]=0;
			    tp_dev.y[1]=0;
			    tp_dev.x[2]=0;
			    tp_dev.y[2]=0;
					
				}					
			}
			
		}
		else delay_ms(10);	//没有按键按下的时候 	  
		
		if(key==KEY0_PRES)	//KEY0按下,则执行校准程序
		{
			LCD_Clear(WHITE);//清屏
		    TP_Adjust();  	//屏幕校准  
			Load_Drow_Dialog();
		}
		//i++;
		//if(i%20==0)LED0=!LED0;
	}
}
//电容触摸屏测试函数
void ctp_test(void)
{
	u8 t=0;
	u8 i=0;	  	    
 	u16 lastpos[5][2];		//记录最后一次的数据 
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
					lcd_draw_bline(lastpos[t][0],lastpos[t][1],tp_dev.x[t],tp_dev.y[t],2,POINT_COLOR_TBL[t]);//画线
					lastpos[t][0]=tp_dev.x[t];
					lastpos[t][1]=tp_dev.y[t];
					if(tp_dev.x[t]>(lcddev.width-24)&&tp_dev.y[t]<16)
					{
						Load_Drow_Dialog();//清除
					}
				}
			}else lastpos[t][0]=0XFFFF;
		}
		
		delay_ms(5);i++;
		if(i%20==0)LED0=!LED0;
	}	


}

//得到path路径下,目标文件的总个数
//path:路径		    
//返回值:总有效文件数
u16 pic_get_tnum(u8 *path)
{	  
	u8 res;
	u16 rval=0;
 	DIR tdir;	 		//临时目录
	FILINFO tfileinfo;	//临时文件信息	
	u8 *fn;	 			 			   			     
    res=f_opendir(&tdir,(const TCHAR*)path); 	//打开目录
  	tfileinfo.lfsize=_MAX_LFN*2+1;				//长文件名最大长度
	tfileinfo.lfname=mymalloc(SRAMIN,tfileinfo.lfsize);//为长文件缓存区分配内存
	if(res==FR_OK&&tfileinfo.lfname!=NULL)
	{
		while(1)//查询总的有效文件数
		{
	        res=f_readdir(&tdir,&tfileinfo);       		//读取目录下的一个文件
	        if(res!=FR_OK||tfileinfo.fname[0]==0)break;	//错误了/到末尾了,退出		  
     		fn=(u8*)(*tfileinfo.lfname?tfileinfo.lfname:tfileinfo.fname);			 
			res=f_typetell(fn);	
			if((res&0XF0)==0X50)//取高四位,看看是不是图片文件	
			{
				rval++;//有效文件数增加1
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
 	DIR picdir;	 		//图片目录
	FILINFO picfileinfo;//文件信息
	u8 *fn;   			//长文件名
	u8 *pname;			//带路径的文件名
	u16 totpicnum; 		//图片文件总数
	u16 curindex;		//图片当前索引
	u8 key;				//键值
	u8 pause=0;			//暂停标记
	u8 t,i;
	u16 temp;
	u16 *picindextbl;	//图片索引表 
	
	delay_init();	    	 //延时函数初始化	  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	 
	 TIM3_Int_Init(499,7199);//10Khz的计数频率，计数到5000为500ms  
	uart_init(115200);	 	//串口初始化为115200
 //	usmart_dev.init(72);		//初始化USMART		
 	LED_Init();		  			//初始化与LED连接的硬件接口
	KEY_Init();					//初始化按键
	LCD_Init();			   		//初始化LCD   
	
	 	tp_dev.init();
	
	W25QXX_Init();				//初始化W25Q128
 	my_mem_init(SRAMIN);		//初始化内部内存池
	exfuns_init();				//为fatfs相关变量申请内存  
 //	f_mount(fs[0],"0:",1); 		//挂载SD卡 
 		res=f_mount(fs[1],"1:",1); 		//挂载FLASH.
	

POINT_COLOR=RED;      
	while(font_init()) 		//检查字库
	{	    
		LCD_ShowString(30,50,200,16,16,"Font Error!");
		delay_ms(200);				  
		LCD_Fill(30,50,240,66,WHITE);//清除显示	     
		delay_ms(200);				  
	}  	 
//	res = f_mkdir("sub1");
//    if (res) die(res);
 	Show_Str(30,50,200,16,"WarShip STM32F1开发板",16,0);				    	 
	Show_Str(30,70,200,16,"图片显示程序",16,0);				    	 
	Show_Str(30,90,200,16,"KEY0:NEXT KEY2:PREV",16,0);				    	 
	Show_Str(30,110,200,16,"KEY_UP:PAUSE",16,0);				    	 
	Show_Str(30,130,200,16,"正点原子@ALIENTEK",16,0);				    	 
	Show_Str(30,150,200,16,"2015年1月20日",16,0);

	Show_Str(30,170,240,16,"开始显示...",16,0); 
	delay_ms(1500);
	piclib_init();										//初始化画图	   	   
	curindex=0;											//从0开始显示
	while(f_open (&fil,"1:/picture.jpg", FA_OPEN_ALWAYS|FA_WRITE|FA_READ))
	{
	Show_Str(30,170,240,16,"创建文件错误!",16,0);
		delay_ms(200);				  
		LCD_Fill(30,170,240,186,WHITE);//清除显示	     
		delay_ms(200);		
}
	
	while(1)
	{
		
	
		rtp_test(); 						//电阻屏测试;
		
		Show_Str(2,2,240,16,"0:/PICTURE/picture.jpg",16,1); 				//显示图片名字
	}
	
	myfree(SRAMIN,picfileinfo.lfname);	//释放内存			    
	myfree(SRAMIN,pname);				//释放内存			    
	myfree(SRAMIN,picindextbl);			//释放内存		
}









