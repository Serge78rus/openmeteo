/*
 * debug.h
 *
 *  Created on: 27 окт. 2019 г.
 *      Author: serge78rus
 */

#ifndef DEBUG_H_
#define DEBUG_H_

/*
 * Debug flag, uncomment it for debug trace to uart
 */
//#define _DEBUG

#ifdef _DEBUG

#include <stdio.h>

#include "uart.h"

#define TRACE() fprintf(&uart, "T [%s:%d]\r\n", __FILE__, __LINE__)
#define TRACEF(message, ...) fprintf(&uart, "T [%s:%d]\t"message"\r\n", __FILE__, __LINE__, __VA_ARGS__)

#define WARN() fprintf(&uart, "W [%s:%d]\r\n", __FILE__, __LINE__)
#define WARNF(message, ...) fprintf(&uart, "W [%s:%d]\t"message"\r\n", __FILE__, __LINE__, __VA_ARGS__)

#define ERROR() fprintf(&uart, "E [%s:%d]\r\n", __FILE__, __LINE__)
#define ERRORF(message, ...) fprintf(&uart, "E [%s:%d]\t"message"\r\n", __FILE__, __LINE__, __VA_ARGS__)

#define FATAL() fprintf(&uart, "F [%s:%d]\r\n", __FILE__, __LINE__); \
		for (;;) {}
#define FATALF(message, ...) fprintf(&uart, "F [%s:%d]\t"message"\r\n", __FILE__, __LINE__, __VA_ARGS__); \
		for (;;) {}

#else

#define TRACE()
#define TRACEF(message, ...)

#define WARN()
#define WARNF(message, ...)

#define ERROR()
#define ERRORF(message, ...)

#define FATAL() for (;;) {}
#define FATALF(message, ...) for (;;) {}

#endif



#endif /* DEBUG_H_ */
