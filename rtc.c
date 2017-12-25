#include <avr/io.h>
#include "rtc.h"

#define F_I2C          50000UL
#define TWBR_VALUE    (((F_CPU)/(F_I2C)-16)/2)

#if ((TWBR_VALUE > 255) || (TWBR_VALUE == 0))
   #error "TWBR value is not correct"
#endif

void RTC_Init(void){
  TWBR = TWBR_VALUE;
  TWSR = 0;
}


void RTC_SetValue(uint8_t adr, uint8_t data){
  /*формируем состояние СТАРТ*/ 
  TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
  while(!(TWCR & (1<<TWINT)));

  /*выдаемна шину пакет SLA-W*/  
  TWDR = (DS1307_ADR<<1)|0;
  TWCR = (1<<TWINT)|(1<<TWEN); 
  while(!(TWCR & (1<<TWINT)));
  
  /*передаем адрес регистра ds1307*/
  TWDR = adr;
  TWCR = (1<<TWINT)|(1<<TWEN); 
  while(!(TWCR & (1<<TWINT)));
  
  /*передаем данные или пропускаем*/
  if (data != RTC_RESET_POINTER){
     /*это чтобы привести данные к BCD формату*/
     //data = ((data/10)<<4) + data%10; 
      
     TWDR = RTC_Encode(data);
     TWCR = (1<<TWINT)|(1<<TWEN); 
     while(!(TWCR & (1<<TWINT)));
  }
  
  /*формируем состояние СТОП*/ 
  TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);  
}

uint8_t RTC_GetValue(void){
  uint8_t data;
  
  /*формируем состояние СТАРТ*/
  TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
  while(!(TWCR & (1<<TWINT))); 
  
  /*выдаемна шину пакет SLA-R*/
  TWDR = (DS1307_ADR<<1)|1;
  TWCR = (1<<TWINT)|(1<<TWEN); 
  while(!(TWCR & (1<<TWINT)));  
  
  /*считываем данные*/
  TWCR = (1<<TWINT)|(1<<TWEN);
  while(!(TWCR & (1<<TWINT)));
  data = TWDR;
  
  /*формируем состояние СТОП*/
  TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
  
  return data; 
}

uint8_t	RTC_Decode(uint8_t value){
	uint8_t decoded = value & 127;
	decoded = (decoded & 15) + 10 * ((decoded & (15 << 4)) >> 4);
	return decoded;
}

uint8_t RTC_Encode(uint8_t value){
	uint8_t encoded = ((value / 10) << 4) + (value % 10);
	return encoded;
}