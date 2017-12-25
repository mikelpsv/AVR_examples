#include <avr/io.h>
#include "onewire.h"
#include "ds18x20.h"



uint8_t DS18x20_StartMeasure(uint8_t* rom){
	//Reset, skip ROM and start temperature conversion
	if (!OW_Reset()) return 0;
	
	if (rom) 
		OW_MatchROM(rom);
	else 
		OW_WriteByte(OW_CMD_SKIPROM);
	
	OW_WriteByte(THERM_CMD_CONVERTTEMP);
	return 1;
}

#ifdef DS18X20_CHECK_CRC
#define CRC8INIT	0x00
#define CRC8POLY	0x18              //0X18 = X^8+X^5+X^4+X^0

uint8_t crc8(uint8_t *data_in, unsigned int number_of_bytes_to_read )
{
	uint8_t	crc;
	unsigned int	loop_count;
	uint8_t	bit_counter;
	uint8_t	data;
	uint8_t	feedback_bit;

	crc = CRC8INIT;
	
	for (loop_count = 0; loop_count != number_of_bytes_to_read; loop_count++)
	{ 
		data = data_in[loop_count];

		bit_counter = 8;
		do { 
			feedback_bit = (crc ^ data) & 0x01;
			if (feedback_bit==0x01) crc = crc ^ CRC8POLY;

			crc = (crc >> 1) & 0x7F;
			if (feedback_bit==0x01) crc = crc | 0x80;

			data = data >> 1;
			bit_counter--;
		}
		while (bit_counter > 0);
	}
	return crc;
}
#endif 

uint8_t DS18x20_ReadData(uint8_t *rom, uint8_t *buffer){
	
	//Reset, skip ROM and send command to read Scratchpad
	if (!OW_Reset()) return 0;
	
	if (rom) 
		OW_MatchROM(rom);
	else 
		OW_WriteByte(OW_CMD_SKIPROM);
	
	OW_WriteByte(THERM_CMD_RSCRATCHPAD);
	
#ifdef DS18X20_CHECK_CRC
	uint8_t	buff[10] = {1,2,3,4,5,6,7,8,9};
	for (uint8_t i=0; i<9; i++) buff[i] = OW_ReadByte();
	buffer[0] = buff[0]; buffer[1] = buff[1];
	if (crc8(buff, 9)) return 0;	// если контрольная сумма не совпала, возвращаем ошибку
#else 
	//Read Scratchpad (only 2 first bytes)
	buffer[0] = OW_ReadByte(); // Read TL
	buffer[1] = OW_ReadByte(); // Read TH	
	
	buffer[2] = OW_ReadByte(); // Read TH	
	buffer[3] = OW_ReadByte(); // Read TH	
	buffer[4] = OW_ReadByte(); // Read TH	
	buffer[5] = OW_ReadByte(); // Read TH	
	buffer[6] = OW_ReadByte(); // Read TH	
	buffer[7] = OW_ReadByte(); // Read TH	
	buffer[8] = OW_ReadByte(); // Read TH	
#endif

	return 1;
}

void DS18x20_ConvertToThemperature(uint8_t* data, uint8_t* themp)
{
	//Store temperature integer digits and decimal digits
	themp[0] = data[0]>>4;
	themp[0] |= (data[1]&0x07)<<4;
	//Store decimal digits
	themp[1] = data[0]&0xf;
	themp[1] *= 6;	
	if (data[1]>0xFB){
		themp[0] = 127-themp[0];
		themp[0] |= 0b10000000; // если температура минусовая
	} 
}


float DS18x20_ConvertToThemperatureFl(uint8_t* data){
	float	Temperature;
	uint8_t	digit, decimal;
	//Store temperature integer digits and decimal digits
	digit = data[0]>>4;
	digit |= (data[1]&0x07)<<4;
	//Store decimal digits
	decimal = data[0]&0xf;
	decimal *= 6;	
	
	if (data[1]>0xFB) digit = 127-digit;
	if (decimal<100) Temperature = digit + ((float)decimal/100);
		else Temperature = digit + ((float)decimal/1000);
	if (data[1]>0xFB) Temperature = -Temperature;
	
	/*
	if (data[1]>0xFB){
		digit = 127-digit;
		digit |= 0b10000000; // если температура минусовая
	} 
	*/
	return Temperature;
}

float DS18x20_ConvertToThemperatureF2(uint8_t* data){
	float	Temperature = 0.0;
	uint8_t	digit, decimal;
	//Store temperature integer digits and decimal digits
	digit = data[0]>>4;
	
	digit |= (data[1]&0x07)<<4;
	//Store decimal digits
	decimal = data[0]&0xf;
	decimal *= 6;	
	/*
	if (data[1]>0xFB) digit = 127-digit;
	if (decimal<100) Temperature = digit + ((float)decimal/100);
		else Temperature = digit + ((float)decimal/1000);
	if (data[1]>0xFB) Temperature = -Temperature;
	*/
	/*
	if (data[1]>0xFB){
		digit = 127-digit;
		digit |= 0b10000000; // если температура минусовая
	} 
	*/
	return Temperature;
}




/**
 * convert temperature from scratchpad
 * in case of error return 200000 (ERR_TEMP_VAL)
 * return value in 10th degrees centigrade
 *
 * 0 - themperature LSB
 * 1 - themperature MSB (all higher bits are sign)
 * 2 - T_H
 * 3 - T_L
 * 4 - B20: Configuration register (only bits 6/5 valid: 9..12 bits resolution); 0xff for S20
 * 5 - 0xff (reserved)
 * 6 - (reserved for B20); S20: COUNT_REMAIN (0x0c)
 * 7 - COUNT PER DEGR (0x10)
 * 8 - CRC
 */
int32_t gettemp_b(uint8_t *scratchpad){
	// detect DS18S20
	int32_t t = 0;
	uint8_t l,m;
	int8_t v;
	if(scratchpad[7] == 0xff) // 0xff can be only if there's no such device or some other error
		//return ERR_TEMP_VAL;
		return 200000;
		
	m = scratchpad[1];
	l = scratchpad[0];
	
	v = l>>4 | ((m & 7)<<4) | (m & 0x80);
	t = ((int32_t)v) * 10L;
	m = (l & 0x0f) >> 1; // add decimal
	// 0   0   1   1   2   2   3   3   4   4   5   5   6   6   7   7 ->
	// 0   1   1   2   3   3   4   4   5   6   6   7   8   8   9   9
	t += (int32_t)m; // t = v*10 + l*0.625 -> convert
	if(m) ++t; // 1->1, 2->3, 3->4, 4->5, 5->6
	if(m > 5) ++t; // 6->8, 7->9

	return t;
}


/**
 * convert temperature from scratchpad
 * in case of error return 200000 (ERR_TEMP_VAL)
 * return value in 10th degrees centigrade
 *
 * 0 - themperature LSB
 * 1 - themperature MSB (all higher bits are sign)
 * 2 - T_H
 * 3 - T_L
 * 4 - B20: Configuration register (only bits 6/5 valid: 9..12 bits resolution); 0xff for S20
 * 5 - 0xff (reserved)
 * 6 - (reserved for B20); S20: COUNT_REMAIN (0x0c)
 * 7 - COUNT PER DEGR (0x10)
 * 8 - CRC
 */
int32_t gettemp_s(uint8_t *scratchpad){
	// detect DS18S20
	int32_t t = 0;
	uint8_t l,m;
	int8_t v;
	if(scratchpad[7] == 0xff) // 0xff can be only if there's no such device or some other error
		//return ERR_TEMP_VAL;
		return 200000;
		
	m = scratchpad[1];
	l = scratchpad[0];
	
	v = l >> 1 | (m & 0x80); // take signum from MSB
	t = ((int32_t)v) * 10L;
	if(l&1) t += 5L; // decimal 0.5

	return t;
}