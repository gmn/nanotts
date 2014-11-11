
#include <stdio.h>
#include <string.h>
#include "wav.h"

void PrintWavinfo( struct wavinfo_t * w ) {
    printf( "format:    %d\nrate:      %d\nwidth:     %d\nchannels:  %d\nsamples:   %d\ndataofs:   %d\n", w->format, w->rate, w->width, w->channels, w->samples, w->dataofs );
}

// returns a pointer to the first byte in the string supplied in chunk
//  or null if not found
// does not match the trailing zero in chunk
const unsigned char * FindMemChunk( const unsigned char *buf, unsigned int buflen, const char *chunk, unsigned int chunklen ) {
    const unsigned char *bp, *cp, *bp2;
    bp = buf;
    while ( bp - buf < buflen ) {
        bp2 = bp;
        cp = (unsigned char *) chunk;
        while ( *cp && *cp == *bp2 ) {
            ++cp; ++bp2;
            if ( cp - (unsigned char *)chunk >= chunklen )
                return bp;
        }
        ++bp;
    }
    return 0;
}

int GetWavInfo( const unsigned char *data, int size, struct wavinfo_t * info )
{
    const unsigned char * p;
    struct WAV_HEADER wh;

    if ( !data )
        return -1;

    if ( !info ) {
        info = malloc( sizeof(struct wavinfo_t) );
    }
    memset( info, 0, sizeof(struct wavinfo_t) );

    if ( !(p = FindMemChunk( data, size, "RIFF", 4 )) )
        return -1;
    memcpy( (void *)&wh.header, (void *)p, sizeof(wh.header) );

    if ( !(p = FindMemChunk( data, size, "fmt ", 4 )) )
        return -1;
    p += 8;
    memcpy( (void *)&wh.format, (void *)p, sizeof(wh.format) );

    if ( !(p = FindMemChunk( data, size, "data", 4 )) )
        return -1;
    memcpy( (void *)&wh.data,   (void *)p, sizeof(wh.data) );

    info->format     = wh.format.formatTag;
    info->rate       = wh.format.samplesPerSec;
    info->width      = wh.format.bitsPerSample / 8;
    info->channels   = wh.format.channels;
    info->samples    = wh.data.len / wh.format.blockAlign;
    info->dataofs    = p - data + 8;

    return 0;
}

