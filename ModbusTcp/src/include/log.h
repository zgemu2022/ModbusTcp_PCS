/*
 * log.h
 *
 *  Created on: 2017-12-19
 *      Author: root
 */

#ifndef LOG_H_
#define LOG_H_

#ifdef __cplusplus

extern "C"
{

#endif
#include "com_include.h"

	void load_CurrentDateTime(char *buf);
	void log_debug_new(const char *title, const char *format, ...); // SOCK_R

#ifdef __cplusplus
}
#endif

#endif /* LOG_H_ */
