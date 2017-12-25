
#define BAUD 9600


#include "main.h"
#include <avr/io.h>
#include <avr/interrupt.h>  
#include <avr/wdt.h>
#include <stdio.h>
#include <util/delay.h>
#include <util/setbaud.h>
#include "delay.h"
#include "onewire.h"
#include "ds18x20.h"
#include "ds2413.h"

#ifdef _EX_RTC
#include "rtc.h"
#endif


void USART_init(){
	UCSR0A=0x00;
	UCSR0B=(1<<RXEN0) | (1<<TXEN0);
	UCSR0C=0x06;
	UBRR0H=UBRRH_VALUE;
	UBRR0L=UBRRL_VALUE;
}

void USART0_write(unsigned char data){
	while(!( UCSR0A & (1 << UDRE0)));
	UDR0 = data;
}

FILE usart_str = FDEV_SETUP_STREAM(USART0_write, NULL, _FDEV_SETUP_WRITE); // ��� ������� printf

void print_address(unsigned char* address) {
	printf("%.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X", address[0],address[1],address[2],address[3],address[4],address[5],address[6],address[7]);
}

unsigned char	nDevices;	// ���������� ��������
unsigned char	owDevicesIDs[MAXDEVICES][8];	// �� ID

// ����� ���� ��������� �� ����
unsigned char search_ow_devices(void){
	unsigned char	i;
   	unsigned char	id[OW_ROMCODE_SIZE];
   	unsigned char	diff, sensors_count;

	sensors_count = 0;

	for( diff = OW_SEARCH_FIRST; diff != OW_LAST_DEVICE && sensors_count < MAXDEVICES ; ){ 
		OW_FindROM( &diff, &id[0] );

      	if( diff == OW_PRESENCE_ERR ) break;
      	if( diff == OW_DATA_ERR )	break;
      	for (i=0;i<OW_ROMCODE_SIZE;i++)
         	owDevicesIDs[sensors_count][i] = id[i];
		
		sensors_count++;
    }
	return sensors_count;
}




unsigned char	themperature[3]; // � ���� ������ ����� �������� �����������
int main(void)
{
	stdout = &usart_str; // ���������, ���� ����� �������� printf 
/*
	DDRB = 0b00000010; PORTB = 0b00000010;
	DDRC = 0b00000000; PORTC = 0b00000000;
	DDRD = 0b00000010; PORTD = 0b00000000;
*/
	USART_init(); // �������� uart


//////////////////////////////////////////


#ifdef _EX_RTC
	uint8_t hour, min, sec, day, date, month, year;
  
	RTC_Init();
  
	/*��������� ���������� �������� �����*/
  
	RTC_SetValue(RTC_HOUR_ADR, 9);
	RTC_SetValue(RTC_MIN_ADR, 0);
	RTC_SetValue(RTC_SEC_ADR, 0);
	RTC_SetValue(RTC_DAY_ADR, 0);
	RTC_SetValue(RTC_DATE_ADR, 22);
	RTC_SetValue(RTC_MONTH_ADR, 12);
	RTC_SetValue(RTC_YEAR_ADR, 2017); // TODO: year truncate
	
	
	/* ������*/ 
	
    /*������������� ��������� �� ������� �����*/
    RTC_SetValue(0, RTC_RESET_POINTER);
    /*��������� �����*/
    sec		= RTC_Decode(RTC_GetValue());
    min		= RTC_Decode(RTC_GetValue());
    hour	= RTC_Decode(RTC_GetValue());
    day		= RTC_GetValue();
    date	= RTC_Decode(RTC_GetValue());
    month	= RTC_Decode(RTC_GetValue());
    year	= RTC_Decode(RTC_GetValue());
    
    printf("\r %d:%d:%d %d %d %d %d", hour, min, sec, day, date, month, year);	
#endif


//////////////////////////////////////////		


while(1){


 	
	
	
	
	nDevices = search_ow_devices(); // ���� ��� ����������

	printf("\r---------- Found %d devices ----------", nDevices);


	for (unsigned char i=0; i<nDevices; i++) // ������ �������� ���������� � ����������� ������
	{
		// ������ ���������� ����� �� ��� ��������� ����, ������� ���������� � ������ ����� �������
		switch (owDevicesIDs[i][0])
		{
			case OW_DS18B20_FAMILY_CODE: { // ���� ������ ����������� DS18B20
				printf("\r"); print_address(owDevicesIDs[i]);
				printf(" - Thermometer DS18B20"); 
				
				DS18x20_StartMeasure(owDevicesIDs[i]); // ��������� ���������
				_delay_ms(800);
				
				uint8_t data[8];
				DS18x20_ReadData(owDevicesIDs[i], data);
				
				//float t = DS18x20_ConvertToThemperatureFl(data); // ��������������� ����������� � ���������������� ���
				//printf(": [%3.2f] C", t);
				int tt = gettemp_b(data);
				printf(": [%d] C", tt);
			} break;

			case OW_DS18S20_FAMILY_CODE: { // ���� ������ ����������� DS18B20
				printf("\r"); print_address(owDevicesIDs[i]); // �������� ���� �������� ������, ����� - �����
				printf(" - Thermometer DS18S20"); // �������� ��� ����������
				
				DS18x20_StartMeasure(owDevicesIDs[i]); // ��������� ���������
				_delay_ms(800);
				unsigned char	data[8]; // ���������� ��� �������� �������� � �������� ����� ������
				DS18x20_ReadData(owDevicesIDs[i], data); // ��������� ������
				//float t = DS18x20_ConvertToThemperatureF2(data); // ��������������� ����������� � ���������������� ���
				int tt = gettemp_s(data);
				
				printf(": [%d] C", tt);
				
				
			} break;

			case OW_DS1990_FAMILY_CODE: { // ���� ������ ����������� ���� DS1990
				printf("\r"); print_address(owDevicesIDs[i]); // �������� ���� �������� ������, ����� - �����
				printf(" - Serial button DS1990"); // �������� ��� ����������
			} break;
			case OW_DS2430_FAMILY_CODE: { // ���� ������� EEPROM
				printf("\r"); print_address(owDevicesIDs[i]); // �������� ���� �������� ������, ����� - �����
				printf(" - EEPROM DS2430"); // �������� ��� ����������
			} break;

			case OW_DS2405_FAMILY_CODE: { // ���� ������ ����
				printf("\r"); print_address(owDevicesIDs[i]); // �������� ���� �������� ������, ����� - �����
				printf(" - Switch 2405"); // �������� ��� ����������
			} break;

			case OW_DS2413_FAMILY_CODE: { // ���� ������ ����
				printf("\r"); print_address(owDevicesIDs[i]); // �������� ���� �������� ������, ����� - �����
				printf(" - Switch 2413!!!"); // �������� ��� ����������
				
				// ��� ��������							
				uint8_t state = DS2413_SetSwitchOn(owDevicesIDs[i]);
				printf("\rState pin AB as 0 0 (on): %d", state);
				_delay_ms(1000);
				
				// ��� ���������
				state = DS2413_SetSwitchOff(owDevicesIDs[i]);
				printf("\rState pin AB as 1 1 (off): %d", state);
				_delay_ms(1000);
				
				DS2413_SetSwitchState(owDevicesIDs[i], 0xFE);
				printf("\rState pin A as 0 (on): %d", state);
				_delay_ms(1000);
				
				DS2413_SetSwitchState(owDevicesIDs[i], 0xFC);
				printf("\rState pin B as 0 (on): %d", state);
				_delay_ms(1000);
				
				DS2413_SetSwitchOff(owDevicesIDs[i]);
				
			} break;
			default:{
				printf("\rUndefined family %d", owDevicesIDs[i][0]);
			}
			
		}

	}


}
}


