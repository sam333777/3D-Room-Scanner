
#include <stdint.h>
#include "PLL.h"
#include "SysTick.h"
#include "uart.h"
#include "onboardLEDs.h"
#include "tm4c1294ncpdt.h"
#include "VL53L1X_api.h"





#define I2C_MCS_ACK             0x00000008  // Data Acknowledge Enable
#define I2C_MCS_DATACK          0x00000008  // Acknowledge Data
#define I2C_MCS_ADRACK          0x00000004  // Acknowledge Address
#define I2C_MCS_STOP            0x00000004  // Generate STOP
#define I2C_MCS_START           0x00000002  // Generate START
#define I2C_MCS_ERROR           0x00000002  // Error
#define I2C_MCS_RUN             0x00000001  // I2C Master Enable
#define I2C_MCS_BUSY            0x00000001  // I2C Busy
#define I2C_MCR_MFE             0x00000010  // I2C Master Function Enable

#define MAXRETRIES              5           // number of receive attempts before giving up
void I2C_Init(void){
  SYSCTL_RCGCI2C_R |= SYSCTL_RCGCI2C_R0;           													// activate I2C0
  SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1;          												// activate port B
  while((SYSCTL_PRGPIO_R&0x0002) == 0){};																		// ready?

    GPIO_PORTB_AFSEL_R |= 0x0C;           																	// 3) enable alt funct on PB2,3       0b00001100
    GPIO_PORTB_ODR_R |= 0x08;             																	// 4) enable open drain on PB3 only

    GPIO_PORTB_DEN_R |= 0x0C;             																	// 5) enable digital I/O on PB2,3
//    GPIO_PORTB_AMSEL_R &= ~0x0C;          																// 7) disable analog functionality on PB2,3

                                                                            // 6) configure PB2,3 as I2C
//  GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R&0xFFFF00FF)+0x00003300;
  GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R&0xFFFF00FF)+0x00002200;    //TED
    I2C0_MCR_R = I2C_MCR_MFE;                      													// 9) master function enable
    I2C0_MTPR_R = 0b0000000000000101000000000111011;                       	// 8) configure for 100 kbps clock (added 8 clocks of glitch suppression ~50ns)
//    I2C0_MTPR_R = 0x3B;                                        						// 8) configure for 100 kbps clock
        
}

//The VL53L1X needs to be reset using XSHUT.  We will use PG0
void PortG_Init(void){
    //Use PortG0
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R6;                // activate clock for Port N
    while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R6) == 0){};    // allow time for clock to stabilize
    GPIO_PORTG_DIR_R &= 0x00;                                        // make PG0 in (HiZ)
  GPIO_PORTG_AFSEL_R &= ~0x01;                                     // disable alt funct on PG0
  GPIO_PORTG_DEN_R |= 0x01;                                        // enable digital I/O on PG0
                                                                                                    // configure PG0 as GPIO
  //GPIO_PORTN_PCTL_R = (GPIO_PORTN_PCTL_R&0xFFFFFF00)+0x00000000;
  GPIO_PORTG_AMSEL_R &= ~0x01;                                     // disable analog functionality on PN0

    return;
}

void PortM0M1M2M3_Init(void){
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R11;                 
	while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R11) == 0){};         
	GPIO_PORTM_DIR_R = 0b00000000;      								       
  GPIO_PORTM_DEN_R = 0b00001111;  
	return;
}

void PortH0H1H2H3_Init(void){
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R7;                 
	while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R7) == 0){};         
	GPIO_PORTH_DIR_R = 0b00001111;       
  GPIO_PORTH_DEN_R = 0b00001111;  
	return;
}


//Used this for AD2 demo light 
void PortE0E1E2E3_Init(void){
SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4; // activate the clock for Port E
while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R4) == 0){}; // allow time for clock to stabilize
    GPIO_PORTE_DIR_R = 0b00001111;
    GPIO_PORTE_DEN_R = 0b00001111; // Enable PE0:PE3 
return;
}

//controls rotation clockwise or counter
void rotationControl(int direction, int step, int delay){

	//counter-clockwise
	if(direction == -1){
		for(int i=0; i<step; i++){
			GPIO_PORTH_DATA_R = 0b00001100;
			SysTick_Wait10ms(delay);
			GPIO_PORTH_DATA_R = 0b00000110;
			SysTick_Wait10ms(delay);
			GPIO_PORTH_DATA_R = 0b00000011;
			SysTick_Wait10ms(delay);
			GPIO_PORTH_DATA_R = 0b00001001;
			SysTick_Wait10ms(delay);
		}

	}
	//clockwise
	else if(direction == 1){
		for(int i=0; i<step; i++){
			GPIO_PORTH_DATA_R = 0b00001001;
			SysTick_Wait10ms(delay);
			GPIO_PORTH_DATA_R = 0b00000110;
			SysTick_Wait10ms(delay);
			GPIO_PORTH_DATA_R = 0b00000110;
			SysTick_Wait10ms(delay);
			GPIO_PORTH_DATA_R = 0b00001100;
			SysTick_Wait10ms(delay);

		}
	}

}




//XSHUT is an active-low shutdown input; 
//board pulls it up to VDD to enable the sensor 
//	Driving this pin low puts the sensor into hardware standby
void VL53L1X_XSHUT(void){
    GPIO_PORTG_DIR_R |= 0x01;                                        // make PG0 out
    GPIO_PORTG_DATA_R &= 0b11111110;                                 //PG0 = 0
    FlashAllLEDs();
    SysTick_Wait10ms(10);
    GPIO_PORTG_DIR_R &= ~0x01;                                            // make PG0 input (HiZ)
    
}




uint16_t	dev = 0x29;			//address of the ToF sensor as an I2C slave peripheral
int status=0;

//main program
int main(void) {
  uint8_t byteData, sensorState=0, myByteArray[10] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF} , i=0;
  uint16_t wordData;
  uint16_t Distance;
  uint16_t SignalRate;
  uint16_t AmbientRate;
  uint16_t SpadNum; 
  uint8_t RangeStatus;
  uint8_t dataReady;

	// Iinitialize ports, and everything needed
	PLL_Init();	
	SysTick_Init();
	onboardLEDs_Init();
	I2C_Init();
	UART_Init();
	PortM0M1M2M3_Init();
	PortH0H1H2H3_Init();
	PortE0E1E2E3_Init();


	


	int mynumber = 1;

	status = VL53L1X_GetSensorId(dev, &wordData);

	//sprintf(printf_buffer,"(Model_ID, Module_Type)=0x%x\r\n",wordData);
	//UART_printf(printf_buffer);

	// Booting ToF chip
	while(sensorState==0){
		status = VL53L1X_BootState(dev, &sensorState);
		SysTick_Wait10ms(10);
  }
	FlashAllLEDs();
	//UART_printf("ToF Chip Booted!\r\n Please Wait...\r\n");
	
	status = VL53L1X_ClearInterrupt(dev); /* clear interrupt has to be called to enable next interrupt*/
	
  /* This function must to be called to initialize the sensor with the default setting  */
  status = VL53L1X_SensorInit(dev);
	Status_Check("SensorInit", status);

	
  /* Optional functions to be used to change the main ranging parameters according the application requirements to get the best ranging performances */
//  status = VL53L1X_SetDistanceMode(dev, 2); /* 1=short, 2=long */
//  status = VL53L1X_SetTimingBudgetInMs(dev, 100); /* in ms possible values [20, 50, 100, 200, 500] */
//  status = VL53L1X_SetInterMeasurementInMs(dev, 200); /* in ms, IM must be > = TB */

  status = VL53L1X_StartRanging(dev);   // This function has to be called to enable the ranging
	
	// ********************************************************************************************
	// ********************************************************************************************
	// ********************************************************************************************
	// ********************************************************************************************
	// THIS IS THE END OF THE TIME OF FLIGHT SENSOR INTITIALIZATION


	// Define variables for motor control
	// direction: Direction of motor rotation (CW or CCW)
	// steps: Number of steps in one full rotation 
	// delay: Constant delay between steps in rotation control
	// rotation: Flag indicating whether rotation is enabled
	// step_count: Counter incremented every time a step is taken
	// rotation_depth: Depth counter incremented after every full rotation
	// data_acquisition_trigger: Flag indicating whether data acquisition should be triggered
	// data_acquisition_enabled: Flag indicating whether data acquisition is enabled by Button 2 and controls the additional status LED and data transmission

	int direction = 1;// direction: Direction of motor rotation (CW or CCW)
	int steps = 512;// steps: Number of steps in one full rotation 
	
	int depth = 0;// depth: Depth counter incremented after every full rotation
	int count = 0;// count: Counter incremented every time a step is taken
	int delay = 1;// delay: Constant delay between steps in rotation control
	int rotate = 0;// rotation: Flag indicating whether rotation is enabled
	
	int scan = 0;  // scan: Flag indicating whether data acquisition should be triggered
	int scanEnabled = 0;// 	scanEnabled: Flag indicating whether data acquisition is enabled by Button 2 and controls the additional status LED and data transmission

	
	//main while loop
	while (1){
		
		
		
	// if button 1 gets pressed ==> toggle ROTATE
	if((GPIO_PORTM_DATA_R&0x01)==0){
		
		while((GPIO_PORTM_DATA_R&0x01)==0){SysTick_Wait10ms(10);} 
		rotate = !rotate;
		}
	
		
	//If button 2 gets pressed ==> toggle SCANENABLED
	if ((GPIO_PORTM_DATA_R&0b00000010) == 0){
		
		while((GPIO_PORTM_DATA_R&0b00000010)==0){SysTick_Wait10ms(100);}
		GPIO_PORTF_DATA_R ^= 0b00010000;
		
		scanEnabled = !scanEnabled;
		
	}
		
	// ************************************************************************************************
	// ************************************************************************************************
	// ************************************************************************************************
	// scanEnabled actually gets the scan and flased led 1 my additional stats
	// scan = 1 scans everytime at 16 steps (every 11.25 deg)
	
	if(scan == 1 && scanEnabled == 1) {
		
		//check if portM 1 button is clicked
		while((GPIO_PORTM_DATA_R&0x01)==0){SysTick_Wait10ms(100);}
		
		
		for(int i = 0; i < 1; i++) {
			
			//for ToF sensor's data to be ready
			while (dataReady == 0){
				status = VL53L1X_CheckForDataReady(dev, &dataReady);
						
						VL53L1_WaitMs(dev, 5);
			}
			dataReady = 0;
			
			//ge tthe data from ToF through API
			status = VL53L1X_GetRangeStatus(dev, &RangeStatus);
			status = VL53L1X_GetDistance(dev, &Distance);					//The Measured Distance value
			status = VL53L1X_GetSignalRate(dev, &SignalRate);
			status = VL53L1X_GetAmbientRate(dev, &AmbientRate);
			status = VL53L1X_GetSpadNb(dev, &SpadNum);
			//status = VL53L1_RdByte(dev, 0x010F, &byteData); //for model ID (0xEA)
      //status = VL53L1_RdByte(dev, 0x0110, &byteData); //for module type (0xCC)
      //status = VL53L1_RdWord(dev, 0x010F, &wordData); //for both model ID and type

			status = VL53L1X_ClearInterrupt(dev); /* clear interrupt has to be called to enable next interrupt*/
			// print to UART 
			int step = count;
			//inverts if it is in the opposite direction 
			if (depth%2 == 1) { 
				step = 512-count;
			}
			
			
			
			// sends distance step depth and spadnum over 
			sprintf(printf_buffer,"%u, %u, %u, %u, %u\r\n", RangeStatus, Distance, step, depth, SpadNum);
			/*sprintf(printf_buffer,"%u, %u, %u, %u, %u\r\n", RangeStatus, Distance, SignalRate, AmbientRate,SpadNum);*/
			UART_printf(printf_buffer);
			SysTick_Wait10ms(10);
			}

			
		// make scan = 0 because we just scanned.
		scan = 0;
   }
	
	 
	 
	 
	 // move to the next step if rotate is enabled
	if (rotate == 1){
		rotationControl(direction, 1, delay);
		count++;
	}
	
	// scan every 11.25 degrees or 16 steps and flash led 2 my measument led 
	if (count % 16 == 0 && rotate == 1 && count != 0){
		FlashLED2(1);
		SysTick_Wait10ms(1); 
		scan = 1; 
	}

	// every time it reaches 512, toggle direction and add 1 to depth
	if (count > 512){
			rotate = 0;
			count = 0;
			direction *= -1;
			depth += 1;
	}
	
	
	
	
	
	// for ad2 demo of frequency
	//SysTick_Wait10ms(100);
	//GPIO_PORTE_DATA_R ^= 0b00000001;
	//OR
	//FlashLED4(1);

	
	
	
}

	VL53L1X_StopRanging(dev);
  while(1) {}
}