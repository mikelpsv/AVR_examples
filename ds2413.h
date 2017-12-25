/*
 * ds2413.h
 *
 * Created: 22.12.2017 16:31:14
 *  Author: mike
 */ 


#ifndef __DS2413_H__
#define __DS2413_H__

//according to the DS2413 datasheet the most significant 6 bits should be set to "1"s
#define DS2413_DEF_STATE 0xFC; 

uint8_t DS2413_SetSwitchState(uint8_t *rom, uint8_t cmd);
uint8_t DS2413_SetSwitchOn(uint8_t *rom);
uint8_t DS2413_SetSwitchOff(uint8_t *rom);

uint8_t DS2413_GetSwitchState(uint8_t *rom);

uint8_t DS2413_GetStateBit(uint8_t state, uint8_t bit);


#endif /* __DS2413_H__ */