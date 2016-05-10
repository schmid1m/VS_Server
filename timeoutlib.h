/**
 *	\file timeoutlib.h
 *	\brief Predefined values for timeout handling
 *	\author Marc Juettner, marc.juettner@juettner-itconsult.de
 *	\author Manuel Gaiser, manuel.gaiser@hs-pforzheim.de
 *	\version 1.0
 *
 */
#if !defined _timeoutlib_h_
#define _timeoutlib_h_

#include <sched.h>
#include <signal.h>
#include <unistd.h>

#define TOL_TIMEOUT_NONE		0
#define TOL_TIMEOUT_OCCURRED	1
#define TOL_TIMEOUT_SECS		10


void tol_start_timeout(int seconds);
void tol_stop_timeout(void);
void tol_reset_timeout(void);
int tol_is_timed_out(void);

#endif //#define _timeoutlib_h_
