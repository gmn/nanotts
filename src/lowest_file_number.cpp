
/*
 *
 * This is a helper script for writing numbered filenames.
 * To write an auto generated filename safely without overwriting previously written files,
 *   we have to check for files with the current prefix so we dont over write them.
 *
 */


#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <limits.h>

#if defined(_WIN32) || defined(_WIN64)
 #include <windows.h>
#else
 #include <dirent.h>
#endif


static bool _getint( const char * p, long int *li ) {
    if ( !p || !*p || !li ) {
        return false;
    }
    long int ival = strtol( p, 0, 10 );
    if ( errno == EINVAL || errno == ERANGE || ival == LONG_MAX || ival == LONG_MIN ) {
        return false;
    }
    *li = ival;
    return true;
}

#if defined(_WIN32) || defined(_WIN64)
int GetNextLowestFilenameNumber( const char * prefix, const char * suffix, int zeropad ) {
	HANDLE hFind;
	WIN32_FIND_DATA FindFileData;
	int count = 0; // 0000
	size_t plen = strlen(prefix);
	const char * c;
    char matching_regex[32];
    sprintf( matching_regex, "./*%s", suffix );

	if ( (hFind = FindFirstFile( matching_regex, &FindFileData )) != INVALID_HANDLE_VALUE ) {
		do {
			printf( "Found: %s\n", FindFileData.cFileName );
			if ( (c = strstr(FindFileData.cFileName, prefix)) ) {
				c += plen;
				char buf[10];
				memcpy( buf, c, zeropad );
				buf[ zeropad ] = 0;
				long int x;
				if ( !_getint(buf, &x) )
					continue;
				if ( x > count )
					count = x;
			}
		} while ( FindNextFile( hFind, &FindFileData ) );
		FindClose( hFind );
	}
	return count + 1;
}

#else /* Not Windows */

int GetNextLowestFilenameNumber( const char * prefix, const char * suffix, int zeropad ) {
    DIR * dir_p = opendir( "." );
    if ( !dir_p ) {
        fprintf( stderr, "can't read directory: \".\"\n" );
        return -1;
    }

    struct dirent * ent = 0;
    int count = 0; // 0000
    size_t plen = strlen( prefix );
    const char * c;

    while ( (ent = readdir( dir_p )) )
    {
        if ( *ent->d_name != '.' && ent->d_type == DT_REG ) {
            if ( (c = strstr( ent->d_name, prefix )) ) {
                c += plen;
                char buf[10];
                memcpy( buf, c, zeropad );
                buf[ zeropad ] = 0;
                long int x;
                if ( !_getint( buf, &x ) )
                    continue;
                if ( x > count )
                    count = x;
            }
        }
    }
    closedir( dir_p );

    return count + 1;
}
#endif

