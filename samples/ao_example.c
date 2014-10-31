// gcc -o ao_example ao_example.c -lao -ldl -lm

#include <stdio.h>
#include <string.h>
#include <ao/ao.h>


struct wavinfo_t {
    int         format;
    int         rate;
    int         width;
    int         channels;
    int         samples;
    int         dataofs;        // chunk starts this many bytes from file start
};

void PrintWavinfo( struct wavinfo_t * w ) {
    printf( "format:    %d\nrate:      %d\nwidth:     %d\nchannels:  %d\nsamples:   %d\ndataofs:   %d\n", w->format, w->rate, w->width, w->channels, w->samples, w->dataofs );
}


// PCM WAV HEADER BLOCKS
struct fileHeader {
    unsigned int riff;              // 'RIFF'
    unsigned int filesize;          // <file length - 8>
    unsigned int wave;              // 'WAVE'
    unsigned int format;            // 'fmt '
    unsigned int formatLength;      // 0x10 == 16
};

struct waveFormat {
    unsigned short formatTag;           // 1 == PCM 
    unsigned short channels;            // 1 = mono, 2 = stereo
    unsigned int samplesPerSec;         // eg, 44100, 22050, 11025
    unsigned int averageBytesPerSec;    // blockAlign * sampleRate
    unsigned short blockAlign;          // channels * bitsPerSample / 8
    unsigned short bitsPerSample;       // 8 or 16
};

struct chunkHeader {
    unsigned int type;              // 'data'
    unsigned int len;               // <length of the data block>
};

struct WAV_HEADER {
    struct fileHeader header;
    struct waveFormat format;
    struct chunkHeader data;
};



// returns a pointer to the first byte in the string supplied in chunk
// or null
// does not match the trailing zero in chunk
static const unsigned char * FindMemChunk( const unsigned char *buf, int buflen, const char *chunk, int chunklen ) {
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



int main(int argc, char **argv)
{
	ao_device *device;
	ao_sample_format format;
	int default_driver;
	char *buffer;
	int buf_size;
    const char * filename = "pico2wave_output.wav";
    FILE * fp = 0;
    size_t read;
    int sample_size_bytes;


	/* -- Initialize -- */
	ao_initialize();

	/* -- Setup for default driver -- */
	default_driver = ao_default_driver_id();

    char buf[300];

    if ( (fp=fopen(filename, "r")) == 0 ) {
		fprintf(stderr, "Error opening file: %s.\n", filename);
		return 1;
    }

    // parse header and print WAV/PCM format 
    read = fread( buf, 1, 300, fp );
    struct wavinfo_t info;
    GetWavInfo( buf, read, &info );
    fseek( fp, info.dataofs, SEEK_SET );
    PrintWavinfo( &info );

    // set the PCM format from wav header
    memset(&format, 0, sizeof(format));
	format.bits         = info.width * 8;
	format.channels     = info.channels;
	format.rate         = info.rate;
	format.byte_format  = AO_FMT_LITTLE;

	/* -- Open driver -- */
	device = ao_open_live(default_driver, &format, 0 /* no options */);
	if (device == 0) {
		fprintf(stderr, "Error opening device.\n");
		return 1;
	}


	// create read buffer
    sample_size_bytes = format.bits/8 * format.channels;
	buf_size = sample_size_bytes * format.rate;
	buffer = calloc(buf_size, sizeof(char));


    // play loop
    while ( (read=fread(buffer, sizeof(char), buf_size, fp)) > 0 ) {
	    ao_play(device, buffer, read);
    }

    // file and buffer
    fclose( fp );
    free( buffer );

	// AO Close and shutdown 
	ao_close(device);
	ao_shutdown();

    return (0);
}
