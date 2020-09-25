#ifndef STM32F4BUFFER_H_INCLUDED
#define STM32F4BUFFER_H_INCLUDED

#include <stdint.h>

typedef enum GSBStatus_
{
    GSB_OK = 0x00,
    GSB_FAIL = 0x01,
    GSB_NO_SPACE_LEFT = 0x02,
    GSB_SIZE_TO_BIG_TO_POP = 0x04,
    GSB_FREED_MORE_THAN_ALLOCATED = 0x08,
    GSB_ALLOCATIONS_LIMIT_REACHED = 0x10
} GSBStatus;

GSBStatus GSB_push(uint32_t size, uint8_t **ppData);
GSBStatus GSB_pop(uint32_t size);
void GSB_resetBuffer();
void GSB_reset();
void GSB_erase();

void GSB_resetStats();
uint32_t GSB_getTotalSpace();
uint32_t GSB_getSpaceUsed();
uint32_t GSB_getSpaceLeft();
uint32_t GSB_getspaceUsedPeak();
int32_t GSB_getAllocsCount();
uint32_t GSB_getAllocsCountPosPeak();
uint32_t GSB_getAllocsCountNegPeak();
GSBStatus GSB_getErrorStatus();

#endif