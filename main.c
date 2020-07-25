#include "stm32f4xx.h"                 
#define osObjectsPublic                     
#include "osObjects.h"     

// Variables 
osThreadId send_Thread;
char array[10] , array_byte;
int16_t array_Cnt = 0;

// Function Prototype
void config_Hardware_LEDs(void);
void config_UART4(uint32_t BaudRate, int16_t Priority , int16_t SubPriority);
void send_Thread_Fnc();

// UART INTERRUPTS
void UART4_IRQHandler(void)
{		
	
	if(USART_GetITStatus(UART4, USART_IT_RXNE) == SET)		              
	{	
		array_byte = USART_ReceiveData(UART4);
		
		if(array_byte != '.')	 						{			array[array_Cnt++] = array_byte;			}
		else															{			osSignalSet(send_Thread, 0x01);	 			}
		
		USART_ClearITPendingBit(UART4, USART_IT_RXNE); 			
	}	
	
	if(USART_GetITStatus(UART4,USART_IT_TC) != RESET)			{		USART_ClearITPendingBit(UART4, USART_IT_TC);		}	
}


// Thread Definations
osThreadDef (send_Thread_Fnc, osPriorityNormal, 1, 0); 

// Void loop
int main (void) {
	
	config_Hardware_LEDs();
	config_UART4(115200,1,0);
	
  osKernelInitialize ();   
	
	send_Thread = osThreadCreate (osThread(send_Thread_Fnc), NULL);
	
  osKernelStart ();                       

	while(1)
	{
		GPIO_ToggleBits(GPIOD,GPIO_Pin_12);	
		//USART_SendData(UART4,'U');
		int a=1000000;
		while(a--);
	}
}

// send_Thread Function

void send_Thread_Fnc()
{
		while(1)
		{			
			osSignalWait(0x01, osWaitForever);
			
			for(int i=0; i< array_Cnt; i++)
			{			
				USART_SendData(UART4,array[i]);
				osDelay(5);
				//while(USART_GetFlagStatus(UART4,USART_IT_TC) == RESET);
			}
			array_Cnt = 0;
			
			for(int i=0; i< 10; i++)					{			array[i] = NULL;			}
			
		}
}

// Configuration Functions UART and Hardware LEDs
void config_Hardware_LEDs(void)
{
	GPIO_InitTypeDef 	GPIO_InitStructer;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	
	GPIO_InitStructer.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructer.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructer.GPIO_Pin	 = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15; // PC11 - Rx /// PC10 - TX
	GPIO_InitStructer.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_InitStructer.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(GPIOD, &GPIO_InitStructer);

}

void config_UART4(uint32_t BaudRate, int16_t Priority , int16_t SubPriority)
{
	
	GPIO_InitTypeDef 	GPIO_InitStructer;
	USART_InitTypeDef	UART_InitStructer;
	NVIC_InitTypeDef  NVIC_InitStructer;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	
	GPIO_InitStructer.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructer.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructer.GPIO_Pin	 = GPIO_Pin_11 | GPIO_Pin_10; // PC11 - Rx /// PC10 - TX
	GPIO_InitStructer.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_InitStructer.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_PinAFConfig(GPIOC , GPIO_PinSource11 , GPIO_AF_UART4);
  GPIO_PinAFConfig(GPIOC , GPIO_PinSource10 , GPIO_AF_UART4);
	
	GPIO_Init(GPIOC, &GPIO_InitStructer);

	UART_InitStructer.USART_BaudRate = BaudRate;
	UART_InitStructer.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	UART_InitStructer.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	UART_InitStructer.USART_Parity = USART_Parity_No;
  UART_InitStructer.USART_StopBits = USART_StopBits_1;
  UART_InitStructer.USART_WordLength = USART_WordLength_8b;  
  USART_Init(UART4 , &UART_InitStructer);
	USART_Cmd(UART4 , ENABLE);
	
	USART_ClearITPendingBit(UART4, USART_IT_RXNE); 	
	USART_ClearITPendingBit(UART4, USART_IT_TC); 	
	
  USART_ITConfig(UART4, USART_IT_RXNE,ENABLE);
  USART_ITConfig(UART4, USART_IT_TC,ENABLE);

	NVIC_InitStructer.NVIC_IRQChannel = UART4_IRQn;
  NVIC_InitStructer.NVIC_IRQChannelPreemptionPriority = Priority;
  NVIC_InitStructer.NVIC_IRQChannelSubPriority = SubPriority;
  NVIC_InitStructer.NVIC_IRQChannelCmd = ENABLE;
   
  NVIC_Init(&NVIC_InitStructer);

}



