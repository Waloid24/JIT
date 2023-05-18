#ifndef LOGS_HPP
#define LOGS_HPP

#include <stdio.h>
#include <stdlib.h>

FILE * _$logOF (const char * nameFile);
FILE * openFile(const char * nameFile, const char * format);

//================================MY_ASSERT====================================

static const int DEBUG_SOFT = 0;

#ifdef MY_ASSERT
	#undef MY_ASSERT
#endif

#ifndef NDEBUG
	#define MY_ASSERT(instruction, message) \
	if (instruction)\
	{\
		fprintf (stderr, "\v " #message "\n\n");\
		fprintf (stderr, "An error occurred in the file: %s \n\n"\
				"In line:                       %d \n\n"\
				"In function:                   %s \n\n",\
				__FILE__, __LINE__, __PRETTY_FUNCTION__);\
		if (DEBUG_SOFT == 0) abort();\
	}
#endif

#ifdef NDEBUG
	#define MY_ASSERT(instruction, message)
#endif

//=============================================================================

//=============================WORK WITH FILE==================================
//If namefile = nullptr, then a file is created with a standard name "logs.txt"
//-----------------------------------------------------------------------------

#ifndef NO_LOGS
    #define _$log(message, ...)                         \
        do                                              \
        {                                               \
            FILE * file = _$logOF("logs.txt");          \
            fprintf (file, message, ##__VA_ARGS__);		\
			fclose (file);								\
        }                                               \
        while (0)
#endif

#ifdef NO_LOGS
    #define _$log(namefile, message)
#endif

//=============================================================================

#endif 