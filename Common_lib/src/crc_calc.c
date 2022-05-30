#include <stdio.h>
#include <inttypes.h>
#include "crc_calc.h"

static uint8_t reverse_byte(uint8_t byte);

/**
	*	@brief Calculation crc8-Maxim/Dallas with polynom
	*	@param *data - pointer to data
	* @param size - bytes count
	* @param poly - type of polynom
	*	@note	 Init val = 0
	*				 in reflection  true
	*				 out reflection true
	*				 final XOR 			false
	*				 check for "123456789" = 0xA1 (for polynom 0x31)
	*/
uint8_t crc8_calculation(const uint8_t *data, const uint32_t size, const uint8_t poly)
{
	uint8_t crc = 0;
	if (data != NULL)
	{
		for (uint8_t i = 0; i < size; i++)
		{
			crc ^= reverse_byte(data[i]);	/// in reflection + XOR
			for (uint8_t j = 0; j < 8; j++)
			{
				crc = (crc & 0x80) ? (crc << 1) ^ poly : (crc << 1);
			}
		}
		crc = reverse_byte(crc);	/// out reflection
	}
	return crc;
#if 1
#else  
	if (data != NULL)
	{
		for (uint8_t i = 0; i < size; i++)
		{
			crc ^= data[i];
			for (uint8_t j = 0; j < 8; j++)	/// for 1 byte
			{
				crc = (crc & 0x80) ? (crc << 1) ^ poly : (crc << 1);
			}
		}
	}
	return crc;
#endif
}
/**
	*	@brief Reverse byte
	*/
static uint8_t reverse_byte(uint8_t byte)
{
	uint8_t r = 0;
  for (uint8_t i = 0; i < 8; i++)
	{
		r <<= 1;
    r |= byte & 1;
    byte >>= 1;
  }
  return r;
}
