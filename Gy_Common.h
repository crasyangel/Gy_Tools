#ifndef _GY_COMMON_H_
#define _GY_COMMON_H_

#ifdef __cplusplus
		extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdint.h>


#define GY_PRINT(fmt, ...) \
		printf("%s_line(%d): " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);
		

#define INT8_T      char
#define INT16_T     short
#define INT32_T     int
#define UINT8_T     unsigned char
#define UINT16_T    unsigned short
#define UINT32_T    unsigned int

#ifndef CHAR_T
#define CHAR_T   INT8_T
#endif
		
#define UINT64_T unsigned long long
#define INT64_T long long
		
		
#ifndef HANDLE
#define HANDLE void* 
#endif
		
#ifdef __cplusplus
		}
#endif
		


#endif  //_GY_COMMON_H_
		
