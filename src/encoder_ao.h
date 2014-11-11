#ifndef __AUDIOPLAYER_AO_H__
#define __AUDIOPLAYER_AO_H__

#include <wav.h>

class AudioPlayer_AO {

    ao_sample_format    pcm_format;
    ao_device *         pcm_device;
    int                 pcm_driver;

public:
    AudioPlayer_AO() : pcm_device(0), pcm_driver(0) {
        memset(&pcm_format, 0xcbcbcbcb, sizeof(pcm_format));
    }

    int  pcmSetup();
    void pcmPlay( char * , unsigned int );
    void pcmShutdown();

    void OpenWavAndPlay( const char * );
    void OpenAndPlay( const char * );

    int  getPcmFormat( const char *, unsigned int * );
};

#endif /* __AUDIOPLAYER_AO_H__ */

