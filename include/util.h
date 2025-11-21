#ifndef _UTIL_H
#define _UTIL_H

#include <iostream>
#include "types.h"
#include "stat.h"

#define KU_ERROR_PRINT(fmt, args...) fprintf(stderr, "[ERROR]: %s:%d:%s(): " fmt, \
		        __FILE__, __LINE__, __func__, ##args)

#define KU_INFO_PRINT(fmt, args...) fprintf(stderr, "[INFO]: %d:%s(): " fmt, \
		         __LINE__, __func__, ##args)

#define KU_DEBUG_PRINT(fmt, args...) fprintf(stderr, "[DEBUG]: %s:%d:%s(): " fmt, \
		        __FILE__, __LINE__, __func__, ##args)

#endif

