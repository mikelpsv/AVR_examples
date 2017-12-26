#include <avr/io.h>
#include <util/delay.h>
#include "onewire.h"


#define sbi(reg,bit) reg |= (1<<bit)
#define cbi(reg,bit) reg &= ~(1<<bit)
#define ibi(reg,bit) reg ^= (1<<bit)
#define CheckBit(reg,bit) (reg&(1<<bit))



void OW_Set(unsigned char mode){
	if (mode){
		cbi(OW_PORT, OW_BIT); 
		sbi(OW_DDR, OW_BIT);
	}else {
		cbi(OW_PORT, OW_BIT); 
		cbi(OW_DDR, OW_BIT);
	}
}

uint8_t OW_CheckIn(void){
	return CheckBit(OW_PIN, OW_BIT);
}

uint8_t OW_Reset(void){
	uint8_t	status;
	OW_Set(1);
	_delay_us(480);
	OW_Set(0);
	_delay_us(60);
	//Store line value and wait until the completion of 480uS period
	status = OW_CheckIn();
	_delay_us(420);
	//Return the value read from the presence pulse (0=OK, 1=WRONG)
	//	return 1 if found
	//	return 0 if not found
	 return !status;
}

void OW_WriteBit(uint8_t bit){
	//Pull line low for 1uS
	OW_Set(1);
	_delay_us(1);
	//If we want to write 1, release the line (if not will keep low)
	if(bit) OW_Set(0); 
	//Wait for 60uS and release the line
	_delay_us(60);
	OW_Set(0);
}

uint8_t OW_ReadBit(void){
	uint8_t	bit = 0;
	//Pull line low for 1uS
	OW_Set(1);
	_delay_us(1);
	//Release line and wait for 14uS
	OW_Set(0);
	_delay_us(14);
	//Read line value
	if(OW_CheckIn()) bit=1;
	//Wait for 45uS to end and return read value
	_delay_us(45);
	return bit;
}

void OW_WriteByte(uint8_t byte){
	for (uint8_t i=0; i<8; i++) 
		OW_WriteBit(CheckBit(byte, i));
}

uint8_t OW_ReadByte(void){
	uint8_t n=0;
	for (uint8_t i=0; i<8; i++) 
		if (OW_ReadBit()) 
			sbi(n, i);
	return n;
}

uint8_t OW_SearchROM(uint8_t diff, uint8_t *id ){ 	
	uint8_t i, j, next_diff;
	uint8_t b;

	if(!OW_Reset()) 
		return OW_PRESENCE_ERR;       // error, no device found

	OW_WriteByte(OW_CMD_SEARCHROM);     // ROM search command
	
	next_diff = OW_LAST_DEVICE;      // unchanged on last device
	
	i = OW_ROMCODE_SIZE * 8;         // 8 bytes
	do{	
		j = 8;                        // 8 bits
		
		do{ 
			b = OW_ReadBit();			// read bit
			
			if( OW_ReadBit() ){        // read complement bit
				if( b )                 // 11
				return OW_DATA_ERR;  // data error
			}else{ 
				if( !b ) { // 00 = 2 devices
				
				if( diff > i || ((*id & 1) && diff != i) ) { 
						b = 1;               // now 1
						next_diff = i;       // next pass 0
					}
				}
			}
			
         OW_WriteBit( b );               // write bit
         *id >>= 1;
         
		 if( b ) 
			*id |= 0x80;			// store bit
         i--;
		} 
		while( --j );
		id++;                            // next byte
    } 
	while( i );
	
	return next_diff;                  // to continue search
}

void OW_FindROM(uint8_t *diff, uint8_t id[]){
	while(1){
		*diff = OW_SearchROM( *diff, &id[0] );
    	if ( *diff==OW_PRESENCE_ERR || *diff==OW_DATA_ERR ||
    		*diff == OW_LAST_DEVICE ) return;

		return;
    }
}

uint8_t OW_ReadROM(uint8_t *buffer)
{
	if (!OW_Reset()) return 0;
	
	OW_WriteByte(OW_CMD_READROM);
	for (uint8_t i=0; i<8; i++){
		buffer[i] = OW_ReadByte();
	}
 return 1;
}

uint8_t OW_MatchROM(uint8_t *rom){
 	if (!OW_Reset()) return 0;
	
	OW_WriteByte(OW_CMD_MATCHROM);	
	for(uint8_t i=0; i<8; i++){
		OW_WriteByte(rom[i]);
	}
 return 1;
}
