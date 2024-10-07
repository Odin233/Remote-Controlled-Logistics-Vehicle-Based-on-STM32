void Servo_Write(uint8_t address_config,uint16_t data)
{
	if(I2C_Start() == 0) 
	{
		I2C_Stop();
	}
	
	I2C_SendByte(address_write);
	if(!I2C_WaitAck())          
	{
		I2C_Stop();
	}
	
	I2C_SendByte(address_config);            
	if(!I2C_WaitAck())
	{
		I2C_Stop();
	}
	
	I2C_SendByte(data);          
	if(!I2C_WaitAck())
	{
		I2C_Stop();
	}
	
	I2C_Stop();
}
 
uint16_t Servo_Read(uint8_t address_config)
{
	uint8_t data;
	if(I2C_Start() == 0)        
	{
		I2C_Stop();
	}
	I2C_SendByte(address_write);            
	if(!I2C_WaitAck())
	{
		I2C_Stop();
	}
	I2C_SendByte(address_config);            
	if(!I2C_WaitAck())
	{
		I2C_Stop();
	}
	if(I2C_Start() == 0)          
	{
		I2C_Stop();
	}
	I2C_SendByte(address_read);         
	if(!I2C_WaitAck())
	{
		I2C_Stop();
	}
	
	data = I2C_ReceiveByte();
	I2C_SendACK(1);
	I2C_Stop();
	
	return data;
}

/*Mode1???
  D7(RESTART):?1??,??????????????SLEEP??0???500us???????1????
  D6(EXTCLOCK):0-???????1-?????????????????,????SLEEP,
                    ?????(??SLEEP????1),???SLEEP
  D5(AI):0-?????????????1-???????????
  D4(SLEEP):0-??SLEEP??(?????500us)?1-??SLEEP??(??D6?????go to sleep)
  D3(SUB1) D2(SUB2) D1(SUB3)
  D0(ALLCALL):0-?????i2c???1-????i2c??(0x70)*/
  
//??PWM??
void PWM_SetFreq(uint8_t freq)
{
	Servo_Write(mode1,0x00);
	float Lprescal ;
	uint8_t old_mode,now_mode,Nprescal;
	freq *= 0.915;                                       //??
	Lprescal = (float)25000000.0 / 4096 / freq - 1;      //????
	Nprescal = floor(Lprescal + 0.5f);                   //????
	old_mode = Servo_Read(mode1);                        //??mode1??(0x00)
	now_mode = (old_mode&0x7F)|0x10;                     
	/*0x7F:0111 1111
	  0x10:0001 0000*/
	Servo_Write(mode1,now_mode);                         //??????
	Servo_Write(Pre_scale,Nprescal);                     //??PWM??
	Servo_Write(mode1,old_mode);                         //?????
	HAL_Delay(5);
	Servo_Write(mode1,old_mode|0xa1);
	/*0xa1:1010 0001
	  ?RESTART,AI,ALLCALL?1*/
}
 
//??ON???0,??OFF???4096???
void Set_Pwm(uint8_t num,uint32_t on,uint32_t off)
{
	Servo_Write(LED0_ON_L + 4 * num,on);          //??ON???
	Servo_Write(LED0_ON_H + 4 * num,on>>8);       //??ON???
	Servo_Write(LED0_OFF_L + 4 * num,off);        //??OFF???
	Servo_Write(LED0_OFF_H + 4 * num,off>>8);     //??OFF???
}