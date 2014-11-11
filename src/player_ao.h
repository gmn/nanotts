#ifndef __PLAYER_AO_H__
#define __PLAYER_AO_H__

#include <ao/ao.h>
#include "wav.h"

class AudioPlayer_AO {

    ao_sample_format    pcm_format;
    ao_device *         pcm_device;
    int                 pcm_driver;

public:
    AudioPlayer_AO();
    ~AudioPlayer_AO();

    int  pcmSetup();
    void pcmPlay( char * , unsigned int );
    void pcmShutdown();

    void OpenWavAndPlay( const char * );
    void OpenAndPlay( const char * );

    int  getPcmFormat( unsigned char *, unsigned int );
};

#endif /* __PLAYER_AO_H__ */

