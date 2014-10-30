//
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
//#include <io.h>

int main( int argc, char ** argv )
{
    char buf[8000];
    size_t x = 0;
    ssize_t xx = 0;
    size_t i;

    memset( buf, 0, 8000 );


    // detect if stdin is coming from a pipe
    if ( ! isatty(fileno(stdin)) ) // On windows prefix with underscores: _isatty, _fileno
    {
        // stdin is a file or a pipe, not a terminal
        x = fread( buf, 1, 8000, stdin );
        for ( i = 0 ; i < x ; i++ ) {
            if ( buf[i] == ' ' )
                buf[i] = '-';
        }
    }
    else
        x = sprintf( buf, "no input from stdin detected.\n" );

/*
    // system call
    xx = read( 0, buf, 8000 );
    write( 1, buf, xx );

    memset( buf, 0, 8000 );
*/

    // stdin & stdout are FILE *
    //x = fread( buf, 1, 8000, stdin );
    fwrite( buf, 1, x, stdout );

    return 0;
}
