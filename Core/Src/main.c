/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "math.h"
#include "ps2.h"
#include "sys.h"
#include "delay.h"
#include "stm32_pca9685.h"
#include "stdlib.h"
#include "string.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
unsigned char Counter_of_Stop = 0,Counter_of_Judge=0;

unsigned int MotorSpeed_right_front,MotorSpeed_right_rear,MotorSpeed_left_front,MotorSpeed_left_rear;  // Output to Motor
int MotorDirection_right_front,MotorDirection_right_rear,MotorDirection_left_front,MotorDirection_left_rear; // Output Direction to Motor
// int MotorOutput_right_front,MotorOutput_left_front,MotorOutput_right_rear,MotorOutput_left_rear;		  // Motor Current Speed, get by Encoder

uint8_t cmd[3] = {0x01,0x42,0x00};
uint8_t PS2data[9] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

int Motor_Judge=0;

int state_plate=0;
int state_box=0;
int Num_plate=0;
int Num_box=0;

float p_m=1.0,i_m=0,d_m=0;
float Last_error_m,Last_Last_error_m,Last_Position_PID_result;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

int fputc(int ch, FILE *p)
{
	while((USART1->SR&0X40)==0);
	USART1->DR = ch;
	
	return ch;
}


void PS2_Get(void)
{
	uint8_t i = 0;
	
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4,GPIO_PIN_RESET);
		
	HAL_SPI_TransmitReceive(&hspi1,&cmd[0],&PS2data[0],1,0xffff);
	delay_us(10);
	HAL_SPI_TransmitReceive(&hspi1,&cmd[1],&PS2data[1],1,0xffff);
	delay_us(10);
	HAL_SPI_TransmitReceive(&hspi1,&cmd[2],&PS2data[2],1,0xffff);
	delay_us(10);
	for(i = 3;i <9;i++)
	{
		HAL_SPI_TransmitReceive(&hspi1,&cmd[2],&PS2data[i],1,0xffff);
		delay_us(10);
	}
  /* print PS2 signal
		for(i=0;i<9;i++)
	{
	  printf("PS2:%d \r\n",PS2data[i]);
	}
	*/
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4,GPIO_PIN_SET);
}

void Set_Motor_Speed_right_front(int direction,int duty_cycle)
{
	if(direction==1) // forward
  {
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, 0);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, duty_cycle);
	}
	if(direction==0) // back
  {
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, duty_cycle);
	}
}
	
void Set_Motor_Speed_right_rear(int direction,int duty_cycle)
{
	if(direction==1) // forward
  {
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, 0);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, duty_cycle);
	}
	if(direction==0) // back
  {
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, duty_cycle);
	}
}

void Set_Motor_Speed_left_front(int direction,int duty_cycle)
{
	if(direction==1) // forward
  {
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, duty_cycle);
	}
	if(direction==0) // back
  {
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 0);
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, duty_cycle);
	}
}

void Set_Motor_Speed_left_rear(int direction,int duty_cycle)
{
	if(direction==1) // forward
  {
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, duty_cycle);
	}
	if(direction==0) // back
  {
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 0);
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, duty_cycle);
	}
}

void Back_Plate(int s)
{
	if(s==0)
	{
	PCA_MG9XX(8,70);
	PCA_MG9XX(10,110);
	state_plate=1;
	}
  if(s==1)
	{
  PCA_MG9XX(8,170);
	PCA_MG9XX(10,10);
	state_plate=0;
  }
}

void Limit_box(int s)
{
	if(s==0)
	{
	PCA_MG9XX(1,110);
	PCA_MG9XX(3,70);
	state_box=1;
	}
  if(s==1)
	{
  PCA_MG9XX(1,0);
	PCA_MG9XX(3,171);
	state_box=0;
  }
}

int Dec_to_Bin(int n)
{
    int sum = 0;
    int y, x = 1;
    while (n != 0)
    {
        y = n % 2;
        sum += x * y;
        x *= 10;
        n /= 2;
    }
    return sum;
}

int PID_control(int Encoder_motor_speed,int Set_motor_speed)
{
  float error_m=Set_motor_speed -Encoder_motor_speed;
	float Increment_PID_result=p_m*(error_m-Last_error_m) + i_m*error_m + d_m * (error_m - 2*Last_error_m + Last_Last_error_m);
	Last_Last_error_m=Last_error_m;
	Last_error_m=error_m;
	float result=Last_Position_PID_result+Increment_PID_result;
	Last_Position_PID_result=Last_Position_PID_result+Increment_PID_result;
  return (int)result;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM4_Init();
  MX_USART1_UART_Init();
  MX_SPI1_Init();
  MX_TIM3_Init();
  MX_I2C2_Init();
  MX_I2C1_Init();
  MX_TIM5_Init();
	PCA_MG9XX_Init(60,90);
  /* USER CODE BEGIN 2 */
	// PCA_MG9XX_Init(60,90); // put into /* Initialize all configured peripherals */ zone, otherwise have no effect
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);	    // TIM1_CH1(pwm)
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);	    // TIM1_CH2(pwm)
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);	    // TIM1_CH3(pwm)
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);	    // TIM1_CH4(pwm)
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);	    // TIM3_CH1(pwm)
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);	    // TIM3_CH2(pwm)
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);	    // TIM3_CH3(pwm)
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);	    // TIM3_CH4(pwm)
	
  HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_1); // Open Encoder A
  HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_2); // Open Encoder B
	
  HAL_TIM_Base_Start_IT(&htim2);                // Enable Timer2 Stop
	
	delay_init(168);
	PS2_SetInit();
	delay_ms(500);
  
	// additional servo initialize
  PCA_MG9XX(8,80);
	PCA_MG9XX(10,100);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
			char str_3[8];
			sprintf(str_3,"%d",Dec_to_Bin((int)PS2data[3]));
			// printf("PS2(Byte 3):%s \r\n",str_3);
			
			char str_4[8];
			sprintf(str_4,"%d",Dec_to_Bin((int)PS2data[4]));
      // printf("PS2(Byte 4):%s \r\n",str_4);
			
			if(str_3[6]=='0') // left joystick buttom (Verify Available)
      {
				if(Motor_Judge==0) Motor_Judge=1;
			}
			
			if(str_4[6]=='0') // R3 buttom (Verify Available)
			{
				Num_box++;
				if(Num_box>1000)
				{
				Limit_box(state_box);
				Num_box=0;
				}
			}
			
			if(str_4[7]=='0') // L3 buttom (Verify Available)
			{
				Num_plate++;
				if(Num_plate>1000)
				{
				Back_Plate(state_plate);
				Num_plate=0;
				}
			}
			
      if(Motor_Judge==1 && PS2data[1]==115) // red LED mode, start to Control through PS2
			{
			  
				// Initialize
				MotorSpeed_right_front=0;
				MotorSpeed_right_rear=0;
				MotorSpeed_left_front=0;
				MotorSpeed_left_rear=0;
				MotorDirection_right_front=1;
				MotorDirection_right_rear=1;
				MotorDirection_left_front=1;
				MotorDirection_left_rear=1;
				
				if(PS2data[8]<120) // left joystick, forward speed (Verify available)
			  {
					MotorSpeed_right_front=7200+((127-PS2data[8])/128*200);
					MotorSpeed_right_rear=6900+((127-PS2data[8])/128*200);
					MotorSpeed_left_front=6900+((127-PS2data[8])/128*200);
					MotorSpeed_left_rear=6900+((127-PS2data[8])/128*200);
					
					MotorDirection_right_front=1;
					MotorDirection_right_rear=1;
					MotorDirection_left_front=1;
					MotorDirection_left_rear=1;
				}
				if(PS2data[8]>135 && PS2data[8]<=255) // left joystick, back speed (Verify Available)
			  {
          MotorSpeed_right_front=5500+((PS2data[8]-127)/128*800);
					MotorSpeed_right_rear=5500+((PS2data[8]-127)/128*800);
					MotorSpeed_left_front=5500+((PS2data[8]-127)/128*800);
					MotorSpeed_left_rear=5500+((PS2data[8]-127)/128*800);
					
					MotorDirection_right_front=0;
					MotorDirection_right_rear=0;
					MotorDirection_left_front=0;
					MotorDirection_left_rear=0;
			  }		
				/*
				if(PS2data[7]<120) // left joystick, left turn
			  {
					MotorSpeed_left_front-=(127-PS2data[7])/128*1500;
					MotorSpeed_left_rear-=(127-PS2data[7])/128*1500;
			  }
				if(PS2data[7]>135 && PS2data[7]<=255) // left joystick, right turn
			  {
					MotorSpeed_right_front-=(PS2data[7]-127)/128*1500;
					MotorSpeed_right_rear-=(PS2data[7]-127)/128*1500;
				}
				*/
				if(PS2data[5]<120) // right joystick, left rotation (Verify Available)
			  {
					unsigned int Motorspeed_left_rotation=(127-PS2data[5])/127*5600;
					MotorSpeed_right_front=Motorspeed_left_rotation;
					MotorSpeed_right_rear=Motorspeed_left_rotation;
					MotorSpeed_left_front=Motorspeed_left_rotation;
					MotorSpeed_left_rear=Motorspeed_left_rotation;
					MotorDirection_right_front=1;
					MotorDirection_right_rear=1;
					MotorDirection_left_front=0;
					MotorDirection_left_rear=0;
			  }
				if(PS2data[5]>135 && PS2data[5]<=255) // right joystick, right rotation (Verify available)
			  {
					unsigned int Motorspeed_right_rotation=(PS2data[5]-127)/(255-127)*5600;
					MotorSpeed_right_front=Motorspeed_right_rotation;
					MotorSpeed_right_rear=Motorspeed_right_rotation;
					MotorSpeed_left_front=Motorspeed_right_rotation;
					MotorSpeed_left_rear=Motorspeed_right_rotation;
					MotorDirection_right_front=0;
					MotorDirection_right_rear=0;
					MotorDirection_left_front=1;
					MotorDirection_left_rear=1;
			  }
			  
				if(str_4[4]=='0') // R2, pan right (Not available)
			  {
				  MotorSpeed_right_front=6500;
					MotorSpeed_right_rear=6400;
					MotorSpeed_left_front=7000;
					MotorSpeed_left_rear=7500;
					MotorDirection_right_front=0;
					MotorDirection_right_rear=1;
					MotorDirection_left_front=1;
					MotorDirection_left_rear=0;
			  }
				if(str_4[5]=='0')  // L2, pan left (Not available)
			  {
				  MotorSpeed_right_front=6600;
					MotorSpeed_right_rear=6600;
					MotorSpeed_left_front=6600;
					MotorSpeed_left_rear=6600;
					MotorDirection_right_front=1;
					MotorDirection_right_rear=0;
					MotorDirection_left_front=0;
					MotorDirection_left_rear=1;
			  }
				
				if(str_3[5]=='0') // right joystick buttom, brake (Verify Available)
        {
          MotorSpeed_right_front=0;
					MotorSpeed_right_rear=0;
					MotorSpeed_left_front=0;
					MotorSpeed_left_rear=0;
					MotorDirection_right_front=0;
					MotorDirection_right_rear=0;
					MotorDirection_left_front=0;
					MotorDirection_left_rear=0;
			  }
				
				if(PS2data[8]>115 && PS2data[8]<135 && PS2data[5]>115 && PS2data[5]<135 && str_4[4]=='1' && str_4[5]=='1') // left joystick, keep stop (Verify Available)
			  {
          MotorSpeed_right_front=0;
					MotorSpeed_right_rear=0;
					MotorSpeed_left_front=0;
					MotorSpeed_left_rear=0;
					MotorDirection_right_front=0;
					MotorDirection_right_rear=0;
					MotorDirection_left_front=0;
					MotorDirection_left_rear=0;
			  }
			}
      
      if(Motor_Judge==1 && PS2data[1]==65) // green LED mode (Not available)
			{
				  MotorSpeed_right_front=0;
					MotorSpeed_right_rear=0;
					MotorSpeed_left_front=0;
					MotorSpeed_left_rear=0;
					MotorDirection_right_front=0;
					MotorDirection_right_rear=0;
					MotorDirection_left_front=0;
					MotorDirection_left_rear=0;
			}				
	}
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim == (&htim2))// Tim2 Stop happen
    {
			  
			  Counter_of_Stop++;
				
        if(Counter_of_Stop>1)
        {
          // Oberserve the LED to judge if enter the timer stop
          // HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_2);
          
					// print Encoder value,short (-32768 - 32767)
          // printf("Encoder_rr = %d motor_rr = %d \r\n",MotorSpeed_right_rear,MotorOutput_right_rear);	
					// printf("Encoder_rf = %d motor_rf = %d \r\n",MotorSpeed_right_front,MotorOutput_right_front);
					// printf("Encoder_lr = %d motor_lr = %d \r\n",MotorSpeed_left_rear,MotorOutput_left_rear);
					// printf("Encoder_lf = %d motor_lf = %d \r\n",MotorSpeed_left_front,MotorOutput_left_front);
					
					PS2_Get();
					/*
					char str_3[8];
			    sprintf(str_3,"%d",Dec_to_Bin((int)PS2data[3]));
          printf("PS2(Byte 3):%s \r\n",str_3);
					char str_4[8];
			    sprintf(str_4,"%d",Dec_to_Bin((int)PS2data[4]));
          printf("PS2(Byte 4):%s \r\n",str_4);
					printf("PS2(Byte 1):%d \r\n",PS2data[1]);
          printf("PS2(Byte 4):%d \r\n",PS2data[4]);
					printf("PS2(Byte 5):%d \r\n",PS2data[5]);
				  printf("PS2(Byte 6):%d \r\n",PS2data[6]);
					printf("PS2(Byte 7):%d \r\n",PS2data[7]);
				  printf("PS2(Byte 8):%d \r\n",PS2data[8]);
					printf("Judge:%d \r\n",Motor_Judge);
					*/
					Counter_of_Stop=0;
        }
	
        //1.Get Motor Speed
        
				// MotorSpeed_right_rear = (short)(__HAL_TIM_GET_COUNTER(&htim4)/18);
        // MotorOutput_right_rear=MotorOutput_right_rear*100/7200; 
        // TIM get motor inpulse,the inpulse get by 10ms / 18 is the real rpm
        // __HAL_TIM_SET_COUNTER(&htim4,0);  // Timer return 0
      
        //2.Input the Duty Cycle
				/*
				printf("MotorSpeed_right_front:%d \r\n",MotorSpeed_right_front);
				printf("MotorSpeed_right_rear:%d \r\n",MotorSpeed_right_rear);
				printf("MotorSpeed_left_front:%d \r\n",MotorSpeed_left_front);
				printf("MotorSpeed_left_rear:%d \r\n",MotorSpeed_left_rear);
				*/
				if(Motor_Judge==1 && PS2data[1]==115)
				{
				Set_Motor_Speed_right_front(MotorDirection_right_front,MotorSpeed_right_front);
				Set_Motor_Speed_right_rear(MotorDirection_right_rear,MotorSpeed_right_rear);
				Set_Motor_Speed_left_front(MotorDirection_left_front,MotorSpeed_left_front);
				Set_Motor_Speed_left_rear(MotorDirection_left_rear,MotorSpeed_left_rear);
				}
    }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

