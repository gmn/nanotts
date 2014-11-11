#ifndef __WAV_H__
#define __WAV_H__ 1

struct wavinfo_t {
    int         format;
    int         rate;
    int         width;
    int         channels;
    int         samples;
    int         dataofs;        // chunk starts this many bytes from file start
};

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

// function signatures
void PrintWavinfo( struct wavinfo_t * w );
int GetWavInfo( const unsigned char *data, int size, struct wavinfo_t * info );

#endif /* __WAV_H__ */
