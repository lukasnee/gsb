
/* 
    gsb - global static buffer accessible in push/pop stack-like fashion. 
    Intended for short lifetime buffer allocation in low-to-medium complexity 
    embedded systems as a more predictable alternative to standard heap 
    allocation. 2020-09.

    - To allocate data from the static buffer use GSB_push(size, ppData).
    - To deallocate some data (without erasing) use GSB_pop(size). 

    Allocation head pointer grows from gsb head with address 
    growing up. 

    NOTE! Make sure you do GSB_pop() shortly after a GSB_push() of 
    the same size when allocated buffer is no longer needed !
*/

/*define the big static buffer size below */
#define STATIC_BUFFER_SIZE 108000 
#define MAX_ALLOCATIONS_LIMIT 100 // which can be surpassed. for analization.

/* todo: GLOBAL_STATIC_BUFFER temporary data distribution map for every 
demoboard action. It is to have better buffer management and safer use. */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <gsb.h>

#if (STATIC_BUFFER_SIZE % 4 != 0)
    #error "STATIC_BUFFER_SIZE not alingned by 32-bit word - adjust size."
#endif

#define STATUS_TYPE GSBStatus
#define STATUS_OK GSB_OK
#define STATUS_NULL GSB_FAIL

#define S_BEGIN STATUS_TYPE S_STATUS = STATUS_OK;
#define S_CHECK(f) S_STATUS = (f); if(S_STATUS != STATUS_OK) {goto S_END_LABEL;}
#define S_RETURN(status) S_STATUS = status; goto S_END_LABEL
#define S_NULL_CHECK(p) if(p == NULL) S_RETURN(STATUS_NULL)
#define S_FINAL S_END_LABEL:;
#define S_END return S_STATUS;
#define S_FINAL_END S_FINAL S_END;

typedef struct GSB_
{
    // at the moment
    uint32_t spaceUsed; 
    int32_t allocsCount; // incremented on push, decremented on pop.
    // satistical
    uint32_t spaceUsedPeak;
    uint32_t allocsCountPosPeak;
    uint32_t allocsCountNegPeak;
    GSBStatus errorStatus; 
    
    uint8_t theBigStaticBuffer[STATIC_BUFFER_SIZE];
} GSB;

static GSB gsb = 
{ 
    .spaceUsed = 0, 
    .allocsCount = 0,
    .spaceUsedPeak = 0,
    .allocsCountPosPeak = 0, 
    .allocsCountNegPeak = 0, 
    .errorStatus = GSB_OK, 
    .theBigStaticBuffer = { 0 }
};

uint32_t GSB_getTotalSpace() { return STATIC_BUFFER_SIZE; }
uint32_t GSB_getSpaceUsed() { return gsb.spaceUsed; }
uint32_t GSB_getSpaceLeft() {return (GSB_getTotalSpace() - GSB_getSpaceUsed());}
uint32_t GSB_getspaceUsedPeak() { return gsb.spaceUsedPeak; }
int32_t GSB_getAllocsCount() { return gsb.allocsCount; }
uint32_t GSB_getAllocsCountPosPeak() { return gsb.allocsCountPosPeak; }
uint32_t GSB_getAllocsCountNegPeak() { return gsb.allocsCountNegPeak; }
GSBStatus GSB_getErrorStatus() { return gsb.errorStatus; }

void GSB_resetBuffer() 
{
    gsb.spaceUsed = 0;
    gsb.allocsCount = 0;
}

void GSB_resetStats() 
{ 
    gsb.spaceUsedPeak = 0;
    gsb.allocsCountPosPeak = 0;
    gsb.allocsCountNegPeak = 0;
    gsb.errorStatus = GSB_OK;
}

void GSB_reset() 
{ 
    GSB_resetBuffer();
    GSB_resetStats();
}

void GSB_erase()
{
    memset(sGSB_data(), 0x00, GSB_getTotalSpace());
    GSB_resetBuffer();
}

static sGSB_trackError(GSBStatus error)
{
    gsb.errorStatus |= error;
}

static void sGSB_spaceUsedAdd(uint32_t size)
{
    gsb.spaceUsed += ((size > GSB_getSpaceLeft()) ? GSB_getSpaceLeft() : size);
}

static void sGSB_spaceUsedSubtract(uint32_t size)
{
    gsb.spaceUsed -= ((size > GSB_getSpaceUsed()) ? GSB_getSpaceUsed() : size);
}

static void sGSB_incrementAllocsCount()
{
    gsb.allocsCount++;
    if(gsb.allocsCount > gsb.allocsCountPosPeak)
    {
        gsb.allocsCountPosPeak = gsb.allocsCount;
    }
    if(gsb.allocsCountPosPeak > MAX_ALLOCATIONS_LIMIT)
    {
        sGSB_trackError(GSB_ALLOCATIONS_LIMIT_REACHED);
    }
}

static void sGSB_decrementAllocsCount()
{
    gsb.allocsCount--;
    if(gsb.allocsCount < gsb.allocsCountNegPeak)
    {
        gsb.allocsCountNegPeak = gsb.allocsCount;
    }
    if(gsb.allocsCountNegPeak < 0)
    {
        sGSB_trackError(GSB_FREED_MORE_THAN_ALLOCATED);
    }
}


GSBStatus GSB_push(uint32_t size, uint8_t **ppData)
{
S_BEGIN
    uint8_t * pData = NULL;
    S_NULL_CHECK(ppData);

    if(size <= GSB_getSpaceLeft())
    {
        pData = (uint8_t*)(gsb.theBigStaticBuffer + gsb.spaceUsed);
        sGSB_spaceUsedAdd(size);
        sGSB_incrementAllocsCount();
    }
    else
    {
        sGSB_trackError(GSB_NO_SPACE_LEFT);
        S_RETURN(GSB_NO_SPACE_LEFT);
    }
S_FINAL
    *ppData = pData;
S_END
}

GSBStatus GSB_pop(uint32_t size)
{
S_BEGIN
    if(size <= GSB_getSpaceUsed())
    {
        sGSB_spaceUsedSubtract(size);
        sGSB_decrementAllocsCount();
    }
    else
    {
        sGSB_trackError(GSB_SIZE_TO_BIG_TO_POP);
        S_RETURN(GSB_SIZE_TO_BIG_TO_POP);
    }
S_FINAL_END
}
