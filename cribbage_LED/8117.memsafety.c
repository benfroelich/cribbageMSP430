//*****************************************************************************
//
//! memsafety.c
//
//! This utility runs before main() and does the following:
//!  (1) Fill the entire heap with SAFETY_PATTERN
//!  (2) Fill the entire stack, with the exception of the first 4 bytes,
//!  with SAFETY_PATTERN.  The first 4 bytes contain the return pointer
//!  and current stack frame, and must not be altered.
//!
//! This allows for the memory to be viewed at runtime to see what sections
//! of stack and heap were used by various functions.
//
//*****************************************************************************
#include <msp430.h>
#include <stdint.h>

//*****************************************************************************
//
//! \def SAFETY_PATTERN defines the pattern to insert into the stack and heap.
//
//*****************************************************************************
#define SAFETY_PATTERN (0xBE)

//*****************************************************************************
//
//! The following are linker symbols to access the stack and heap
//
//! \var _stack is first word of the stack.
//! \var __STACK_END is the address of the last word of the stack.
//! \var _sys_memory is the first word of the heap.
//! \var __SYSMEM_SIZE is the size of the heap
//
//*****************************************************************************
extern int16_t _stack;
extern void * __STACK_END;
extern int16_t _sys_memory;
extern void *__SYSMEM_SIZE;

//*****************************************************************************
//
//! The following function replaces the RTL _system_pre_init() function at link
//! time, and is called by the boot function before autoinit().
//! It initilizes the heap and stack to a known pattern (SAFETY_PATTERN) at the
//! beginning of execution.
//
//*****************************************************************************
int16_t _system_pre_init(void)
{
	uint8_t *pMem;
	uint16_t ui16Size;

	// Stop the watchdog timer
	WDTCTL = WDTPW | WDTHOLD;

	// Fill the heap
	pMem = (uint8_t*)(&_sys_memory);
	ui16Size = (uint16_t)(&__SYSMEM_SIZE);
	__no_operation();
	while (ui16Size--)
	{
		*(pMem++) = SAFETY_PATTERN;
	}
	__no_operation();

	// Fill the stack
	pMem = (uint8_t*)(&_stack);
	ui16Size = (uint16_t)((uint8_t*)(&__STACK_END) - pMem);
	ui16Size -= 4;
	__no_operation();
	while (ui16Size--)
	{
		*(pMem++) = SAFETY_PATTERN;
	}
	__no_operation();

	// Return 1 to init segments
	return 1;
}


