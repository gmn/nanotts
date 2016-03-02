#ifndef __MMFILE_H__
#define __MMFILE_H__


#ifdef _WIN
	#define _CRT_SECURE_NO_DEPRECATE
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#ifndef _WIN
	#include <sys/types.h> 		// mmap
	#include <sys/mman.h>		// mmap
#endif  /* _WIN */
#include <sys/stat.h>			// fstat

#ifdef _WIN
	#include <windows.h>
	#include <process.h> 		// exit
#endif /*_WIN */

#ifdef _WIN
#define PATH_SEP '\\'
#else
#define PATH_SEP '/'
#endif


struct mmfile_t 
{
	char *          filename;
	FILE *          fp;
	unsigned int    fileno;
	unsigned int    size;
    unsigned char * data;


// windows file handling
#ifdef _WIN
	HANDLE hFile;
	HANDLE hFileMapping;
	LPVOID lpFileBase;
#endif

	mmfile_t( void ) : filename(0), fp(0), fileno(0), size(0), data(0) {}
	mmfile_t( const char * );
	~mmfile_t() ;
    void open( const char * );
    void close();
};

#endif /* __MMFILE_H__ */

