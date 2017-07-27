/**********************************************************************
* Signal university- Telecom engineer department
* 
* Code for ds1307_led7seg CLOCK 
*
* Brief	:  Source File for 89s52
* Version	: 1.1
* Date	:	19. Jul. 2015
* Author	: Do Ngoc Tuan
* Phone		: 0986588017
* This code describes for DS1307 CLOCK 
***********************************************************************
**********************************************************************/
#include <p89v51rx2.h>
#define SCL    P2_0			   // noi chan DS1307 voi 89s52 
#define SDA    P2_1

sbit menu = P1^1;				// cac nut dieu chinh h
sbit ok = P1^0;
sbit up = P1^3;
sbit down = P1^2;


		 
//==================================Ham convert=======================================================
unsigned int BCD2Dec(unsigned int BCD){
	unsigned int L, H;
	L=BCD & 0x0F;
	H=(BCD>>4)*10;
	return (H+L);
}
unsigned int Dec2BCD(unsigned int Dec){
	unsigned int L, H;
	L=Dec % 10;
	H=(Dec/10)<<4;
	return (H+L);
}
//=====================================du lieu lef7seg=====================================================
unsigned char led[10]={0x40,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90}; 
unsigned char sec,min,hour,day,month,year,s,m,h,i;

 void delay(unsigned int time)   // moi so tuong ung 1000us .
{														   
	while (--time)				 
	{
		unsigned char temp;
		temp = 80;
		while(--temp);
	}
}

 void delay_ms(unsigned int t){
        unsigned int i,j;
        for(i=0;i<t;i++)
        for(j=0;j<125;j++);
}

void delay_us(unsigned int t){
        unsigned int i;
        for(i=0;i<t;i++);
}
/****************Cac ham giao tiep I2C***************************/
void I2C_time_start(){
          SCL = 1; 
          SDA = 1;
          SDA = 0; 
          SCL = 0; 
}
void I2C_time_stop(){
          SCL = 1; 
          SDA = 0; 
          SDA=1;
          SCL=1; 
}
unsigned char I2C_write(unsigned char dat){
          unsigned char i;
          for (i=0;i<8;i++){
                    SDA = (dat & 0x80); 
                    SCL=1;
                    SCL=0;
                    dat<<=1;
          }
          SCL = 1; 
          SCL = 0;
          return dat;
}
unsigned char I2C_read(void){
          bit rd_bit;
          unsigned char i, dat;
          dat = 0x00;
          for(i=0;i<8;i++)
		  {                        /* For loop read data 1 byte */
                    SCL = 1;
                    rd_bit = SDA; /* Keep for check acknowledge */
                    dat = dat<<1;
                    dat = dat | rd_bit; /* Keep bit data in dat */
                    SCL = 0; /* Clear SCL */
          }
          return dat;
}	
//==============================Khai bao cho DS1307================================================
 
	   /***************Dia chi giao tiep DS1307*********************/
#define DS1307_SEC      0x00 //Giay
#define DS1307_MIN      0x01 //Phut
#define DS1307_HOUR     0x02 //Gio
#define DS1307_DAY      0x03 //Thu
#define DS1307_DATE     0x04 //Ngay
#define DS1307_MONTH    0x05 //Thang
#define DS1307_YEAR     0x06 //Nam
//====================
void edit_m();
void edit_h();
void display_time();
void scan_buzzer();
void on_buzzer();
void off_buzzer();
void bip_buzer();

//===========================

void DS1307_Write(unsigned char addr,unsigned char dat){
          unsigned int temp;
          temp = dat ; /*HEX to BCD*/
          
          I2C_time_start(); /* time_start i2c bus */
          I2C_write(0XD0); /* Connect to DS1307 */
          I2C_write(addr); /* Requetime_5 RAM address on DS1307 */
          I2C_write(temp); 
          I2C_time_stop();
}
unsigned char DS1307_Read(unsigned char addr){
          unsigned int tm,ret;
          I2C_time_start(); /* time_start i2c bus */
          I2C_write(0xD0); /* Connect to DS1307 */
          I2C_write(addr); /* Requetime_5 RAM address on DS1307 */
          I2C_time_start(); /* time_start i2c bus */
          I2C_write(0XD1); /* Connect to DS1307 for Read */
          ret = I2C_read(); /* Receive data */
          I2C_time_stop();
          return ret;
}
void DS1307_Set(){
          DS1307_Write(DS1307_SEC,sec);
          DS1307_Write(DS1307_MIN,min); 
          DS1307_Write(DS1307_HOUR,hour); 
          DS1307_Write(DS1307_DATE,day); 
          DS1307_Write(DS1307_MONTH,month); 
          DS1307_Write(DS1307_YEAR,year); 
}
void DS1307_GetTime(){
         
          hour=DS1307_Read(DS1307_HOUR);
          min=DS1307_Read(DS1307_MIN);
          sec=DS1307_Read(DS1307_SEC);
}
void DS1307_GetDate(){
         
          day=DS1307_Read(DS1307_DATE);
          month=DS1307_Read(DS1307_MONTH);
          year=DS1307_Read(DS1307_YEAR);
}
//===================================ham edit h, m, display^^========================================
void edit_m()
{
bip_buzer();
while(ok)
{
		     if((up==0) )
		 {    
			  m++;
			  if(m>59)m=0;
			  while(!up)
			  {
			   if(m>59)m=0;
			  P0=led[m%10]; P2_3=0;delay(20);P2_3=1;
		      P0=led[m/10]; P2_2=0;delay(20);P2_2=1;
		      }
	     }   
		  /***************************/
			 if((down==0) )
		 { 
			  if(m==0)m=59;
			  m--;
			  while(!down)
			    {
			  if(m==0)m=59;
			  P0=led[m%10]; P2_3=0;delay(20);P2_3=1;
		      P0=led[m/10]; P2_2=0;delay(20);P2_2=1;
		        }
        }
	P0=led[m%10]; P2_3=0;delay(20);P2_3=1;
    P0=led[m/10]; P2_2=0;delay(20);P2_2=1;
	 if(menu==0)	//chon sang edit_h
	{while(!menu);
	edit_h();}	
}
min=Dec2BCD(m);
DS1307_Write(DS1307_MIN,min);
delay(20);		
}	

//====================================================					
void edit_h()
{
bip_buzer();
while(ok)
{
		     if(up==0) 
		 {    
			  h++;
			  if(h>23)m=0;
			  while(!up)
			  {
			   if(h>23)h=0;
			  P0=led[h%10]; P2_6=0;delay(20);P2_6=1;
		      P0=led[h/10]; P2_7=0;delay(20);P2_7=1;
		      }
	     }   
		  /***************************/
			 if(down==0) 
		 { 
		      if(h==0)h=23;
			  h--;
			  while(!down)
			    {
			  if(h==0)h=23;
			  P0=led[h%10]; P2_6=0;delay(20);P2_6=1;
		      P0=led[h/10]; P2_7=0;delay(20);P2_7=1;
		        }
        }
	P0=led[h%10]; P2_6=0;delay(20);P2_6=1;
    P0=led[h/10]; P2_7=0;delay(20);P2_7=1;
	if(menu==0)	//chon sang display_time
	{while(!menu);
	edit_m();}	
}
hour=Dec2BCD(h);
DS1307_Write(DS1307_HOUR,hour);
delay(20);			
}	
		 
//=================================display_led7seg==================================================
void display_time()
{ DS1307_GetTime();
		 
		  m=BCD2Dec(min);
		  h=BCD2Dec(hour);

		  P3_0=1;
		  if(BCD2Dec(sec)!=s){P3_0=0;}
		    s=BCD2Dec(sec);
		  P0=led[m%10]; P2_3=0;delay(4);P2_3=1;
		  P0=led[m/10]; P2_2=0;delay(4);P2_2=1;
		  P0=led[h%10]; P2_6=0;delay(4);P2_6=1;
		  P0=led[h/10]; P2_7=0;delay(4);P2_7=1;
		  P0=led[m%10]; P2_3=0;delay(4);P2_3=1;
		  P0=led[m/10]; P2_2=0;delay(4);P2_2=1;
		  P0=led[h%10]; P2_6=0;delay(4);P2_6=1;
		  P0=led[h/10]; P2_7=0;delay(4);P2_7=1;
		   scan_buzzer();
		
		  		       
}
//==========================================buzzer===============================================
void scan_buzzer()			// day la cho chung ta hen h tat mo thiet bi, tuy nhien can luu no vao rom ds1307
{ 
		if((h==21)&(m==16)&(s==0)) P3_1=0;
		if((h==21)&(m==20)&(s==0)) P3_1=1; 

		if((h==1)&(m==20)&(s==0)) P3_7=0;
		if((h==1)&(m==23)&(s==0)) P3_7=1; 

	

}


		
//================================================^^=================================================

void main()
{	    
		 keny: ;
  		 
		 P2_7=0;
		 P2_6=0;
		 delay_ms(300);
		 P2_2=0;
		 delay_ms(300);
		 P2_3=0;
		 delay_ms(300);
		 bip_buzer();
		 P2=0xff;

	     hour=DS1307_Read(DS1307_HOUR);
         min=DS1307_Read(DS1307_MIN);
	 	 DS1307_Set();


         while (1)      
   {  
		 DS1307_GetTime();
		 display_time();
		 scan_buzzer();
	  
		                    
         if(!menu&&!up)		  //scan edit
		 { 
		    while(!menu);
		  	edit_m();
		 }    
		 
		 if(up==0) 
		 {    
			  while(!up);
			  P2=0xff;
			  P0=0xff;
			  P3_0=1;
			  goto keny	;
	     }   
		  
		  if(down==0) 
		 {    
			  while(!down);
			  P2=0xff;
			  P0=0xff;
			  P1_1=1;
			  goto keny	;
	     }   
		  
		  if(ok==0) 
		 {    
			  while(!ok);
			  P2=0xff;
			  P0=0xff;
			  P1_1=1;
			  goto keny	;
	     }   
		  
			  
      }




}
		 

		 
