/*
 * ds2413.c
 *
 * Created: 22.12.2017 16:30:55
 *
 * Выходы у 2413 - ИНВЕРСНЫЕ!!! Замыкают нагрузку на землю
 * 0 - ключ открыт, 1 - закрыт
 *
 */ 
#include <avr/io.h>
#include "onewire.h"
#include "ds2413.h"


/*
 *	Return 0 - error or state                                            
 */
uint8_t DS2413_SetSwitchState(uint8_t *rom, uint8_t cmd){

	if (!OW_Reset()) return 0;
	
	if (rom)
		OW_MatchROM(rom);
		
	OW_WriteByte(OW_CMD_PIO_WRITE);
				
	OW_WriteByte(cmd);
	OW_WriteByte(~cmd);
		
	uint8_t res = OW_ReadByte();
	
	if(res == 0xAA){
		return OW_ReadByte();;
	}else{
		return 0;
	}
	
}

/*
 *	All off (1111 1111)
 */
uint8_t DS2413_SetSwitchOff(uint8_t *rom){
	uint8_t cmd = 0xFF; // 1111 1111
	return DS2413_SetSwitchState(rom, cmd);
}


/*
 *	All on (1111 1100)
 */
uint8_t DS2413_SetSwitchOn(uint8_t *rom){
	uint8_t cmd = DS2413_DEF_STATE; // 0xFC (1111 1100)
	return DS2413_SetSwitchState(rom, cmd);
}

/*
4 - 7 Complement of b3 to b0 
3 - PIOB Output Latch State (состояние ключа B)
2 - PIOB Pin State (состояние выхода B)
1 - PIOA Output Latch State (состояние ключа A)
0 - PIOA Pin State (состояние выхода A)

Note: Usually, the PIO pin state and PIO Output Latch State are the same. To read from a PIO, the PIO Output
Latch must be 1. If the PIO pin is then pulled low by a switch or external circuitry, the output latch state and pin
state are different.
*/
uint8_t DS2413_GetSwitchState(uint8_t *rom){

	if (!OW_Reset()) return 0;

	if (rom)
		OW_MatchROM(rom);

	// после этой команды slave постоянно шлет состояние пока мастер не сделает сброс
	// Datasheet, page 16: Continues until master sends Reset Pulse	OW_WriteByte(OW_CMD_PIO_READ); 

	// Datasheet, page 17:
	// Read the state of the PIOs 3 times.
	// ???
	uint8_t state = OW_ReadByte();
	
	return state;
}


uint8_t DS2413_GetStateBit(uint8_t state, uint8_t bit){
	return (state & (1<<bit)) > 0;		
}


