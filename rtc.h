#ifndef __RTC_H__
#define __RTC_H__



#define RTC_RESET_POINTER   0xff
#define DS1307_ADR			104

/*адреса регистров*/

#define RTC_SEC_ADR     0x00
#define RTC_MIN_ADR     0x01
#define RTC_HOUR_ADR    0x02
#define RTC_DAY_ADR		0x03
#define RTC_DATE_ADR    0x04
#define RTC_MONTH_ADR   0x05
#define RTC_YEAR_ADR    0x06

/*пользовательские функции*/

void RTC_Init(void);
void RTC_SetValue(uint8_t adr, uint8_t data);
uint8_t RTC_GetValue(void);
uint8_t	RTC_Decode(uint8_t value);
uint8_t RTC_Encode(uint8_t value);

#endif /* __RTC_H__ */