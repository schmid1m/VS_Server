/**
 *	\file timeoutlib.c
 *	\brief Function definitions for timeout handling
 *	\author Marc Juettner, marc.juettner@juettner-itconsult.de
 *	\author Manuel Gaiser, manuel.gaiser@hs-pforzheim.de
 *	\version 1.1
 *
 */
#include "timeoutlib.h"

/**
 *	\defgroup timeoutlib Timeout handling
 *
 * 	\{
 */
static char cTimeoutFlag = TOL_TIMEOUT_NONE;

/** 
 *	\brief Timeout handler
 *	\param signum	Signal number
 */
void tol_handle_timeout(int signum) {
	cTimeoutFlag = TOL_TIMEOUT_OCCURRED;
	return;
}

/** 
 *	\brief Start timer
 *	\param seconds	Number of seconds until timeout elapses.
 */
inline void tol_start_timeout(int seconds) {
	// set signal handler for timeout
	signal(SIGALRM, tol_handle_timeout);
	// terminate recvfrom if a timeout occurs
	siginterrupt(SIGALRM, 1);
	cTimeoutFlag = TOL_TIMEOUT_NONE;
	alarm(seconds);
	return;
}

/** 
 *	\brief Stop timer
 *	\param
*/
inline void tol_stop_timeout(void) {
	alarm(0);
	signal(SIGALRM, SIG_IGN);
}

/** 
 *	\brief Check if timeout occurred
 *	\return True if timeout occurred, false otherwise.
 */
inline int tol_is_timed_out(void) {
	return (cTimeoutFlag == TOL_TIMEOUT_OCCURRED);
}

/** 
 *	\brief Reset timeout flag
 *	\param
 */
inline void tol_reset_timeout(void) {
	cTimeoutFlag = TOL_TIMEOUT_NONE;
	return;
}

/**
 *	\}
 */
