/**
 *	\file plreglib.c 
 *	\author Manuel Gaiser, manuel.gaiser@hs-pforzheim.de
 *	inspired by http://fpgacpu.wordpress.com/2013/05/28/how-to-design-and-access-a-memory-mapped-device-part-two/
 *	\version 1.0
 *	\brief An PL register access library that implements access to a GPIO devices and scrambler functionality
 *		within programmable logic.
 *		This library is not thread-safe. Root privileges are needed for usage.
 *		The usage of mmap may cause security problems.
 *	\defgroup plreg PLREG
 *	\{
 *	\par Overview
 * 	This library implements functions for PL register access to control GPIO devices and a scrambler
 * 	design inside the PL of a Zedboard.
 *
 *	\par Prerequisites
 *	A bit file containing GPIO devices and a scrambler design needs to be loaded into the PL before running any
 *	application that uses it.
 *	\}
 */
#include "plreglib.h"


/**
 *	\ingroup plreg
 *	\defgroup plreglib PLREG library functions
 *
 * 	\{
 */


/**
 *	\brief Maps a memory address to a pointer for PL register access
 *	\param reg_addr	An integer value as register address
 *	\param reg_ptr	Points to a pointer representing the register address
 *	\return Zero if successful, nonzero otherwise
 *	\see 	plreglib.h
 *
 *	Maps a memory address to a pointer for PL register access
 */

int PLREG_Open(unsigned int reg_addr, unsigned int **reg_ptr)
{

#ifdef SIMULATE_PLREG
	*(reg_ptr) = NULL;

	return EPLREG_NOERROR;
#else
	int fd;
	void *ptr;
	unsigned page_addr, page_offset;
	unsigned page_size=sysconf(_SC_PAGESIZE);

	fd = open ("/dev/mem", O_RDWR);
	if (fd < 1) {
		return -EPLREG_DEVICE_OPEN_ERROR;
	}

	/* mmap the device into memory */
	page_addr = (reg_addr & (~(page_size-1)));
	page_offset = reg_addr - page_addr;
	ptr = mmap(NULL, page_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, page_addr);

	*(reg_ptr) = (unsigned int *)ptr + (page_offset / sizeof(unsigned int));

	return EPLREG_NOERROR;
#endif // SIMULATE_PLREG
}


/**
 *	\brief Unmaps a memory address from to a pointer
 *	\param reg_addr	An integer value as register address
 *	\param reg_ptr	pointer representing the register address
 *	\return Zero if successful, nonzero otherwise
 *	\see 	plreglib.h
 *
 *	Unmaps a memory address from a pointer
 */
int PLREG_Close(unsigned int reg_addr, unsigned int *reg_ptr)
{
#ifdef SIMULATE_PLREG
	return EPLREG_NOERROR;
#else
	unsigned page_addr, page_offset;
	unsigned page_size=sysconf(_SC_PAGESIZE);

	page_addr = (reg_addr & (~(page_size-1)));
	page_offset = reg_addr - page_addr;

	/* unmap the device */
	munmap(reg_ptr - page_offset, page_size);

	return EPLREG_NOERROR;
#endif // SIMULATE_PLREG	
}


/**
 *	\brief Set scrambler generator polynom
 *
 *	\param reg_ptr	A pointer representing a register address
 *	\param GP		An integer value representing the generator polynom
 *	\return 		Zero if successfully set the generator polynom, nonzero otherwise
 *	\see 			plreglib.h
 *
 *	The function will set the scrambler's generator polynom for further use 
 *	with PLREG_Scramble().
 */
int PLREG_SetGeneratorPolynom(unsigned int *reg_ptr, int GP)
{
#ifdef SIMULATE_PLREG
	// set generator polynom, use bits 15..1
	GP_15_1 = GP & 0xFFFE;

	return EPLREG_NOERROR;
#else
	*reg_ptr = GP;

	return EPLREG_NOERROR;
#endif // SIMULATE_PLREG
}



/**
 *	\brief Scramble data
 *
 *	\param reg_ptr	A pointer representing a register address
 *	\param operand	An integer value as scrambler input value
 *	\param result	An integer pointer pointing to a variable the scrambler result 
 *					is to be written to
 *	\return 		Zero if successfully executed, nonzero otherwise
 *	\see 			plreglib.h
 *
 *	Scrambles a given operand.
 */
int PLREG_Scramble(unsigned int *reg_ptr, int operand, int *result)
{
#ifdef SIMULATE_PLREG
	int i, res0;

	// scramble operand and return result
	res0 = (operand >> 15) & 0x0001;
	if (res0 == 1) {
		*result = (((GP_15_1 & 0xFFFE) ^ (operand << 1)) & 0xFFFE) | res0;
	} else {
		*result = (((GP_15_1 & 0x0000) ^ (operand << 1)) & 0xFFFE) | res0;
	}

	return EPLREG_NOERROR;
#else
	// Write operand and read scrambled data
	*reg_ptr = operand;
	*result = *(reg_ptr);

	return EPLREG_NOERROR;
#endif // SIMULATE_PLREG
}

/**
 *	\}
 */
