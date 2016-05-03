/**
 *	\file plreglib.h
 *	\brief PL register access functions
 *	\author Manuel Gaiser, manuel.gaiser@hs-pforzheim.de
 *	\version 1.1
 *
 */
#if !defined _plreglib_h_
#define _plreglib_h_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <time.h>

//DEFINES for use within the PLREG library...
#define SIMULATE_PLREG

#ifdef SIMULATE_PLREG // simulated PL register/scrambler access
	int GP_15_1;
#endif // SIMULATE_PLREG

#define PLREG_ADDR_GP				"0x78c00000"
#define PLREG_ADDR_DATA				"0x78c00004"

#define str2ul(str)					(strtoul(str, NULL, 0))

//ERROR CODES for PLREG library functions
#define EPLREG_NOERROR				0
#define EPLREG_DEVICE_OPEN_ERROR	1

//Function prototypes
int PLREG_Open(unsigned int reg_addr, unsigned int **reg_ptr);
int PLREG_Close(unsigned int reg_addr, unsigned int *reg_ptr);
int PLREG_SetGeneratorPolynom(unsigned int *reg_ptr, int GP);
int PLREG_Scramble(unsigned int *reg_ptr, int operand, int *result);


#endif //#define _plreglib_h_
