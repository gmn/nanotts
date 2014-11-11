// encoder_ao.cpp

#include <stdio.h>
#include "encoder_ao.h"
#include "mmfile.h"

/*
================================================
    AudioPlayer_AO methods
================================================
*/
int AudioPlayer_AO::pcmSetup()
{
    /* -- Initialize -- */
    ao_initialize();

    /* -- Setup for default driver -- */
    pcm_driver = ao_default_driver_id();

    // set the PCM format from wav header
    memset(&pcm_format, 0, sizeof(pcm_format));
    pcm_format.bits         = 16;
    pcm_format.channels     = 1;
    pcm_format.rate         = 16000;
    pcm_format.byte_format  = AO_FMT_LITTLE;

    /* -- Open driver -- */
    pcm_device = ao_open_live(pcm_driver, &pcm_format, 0 /* no options */);
    if (pcm_device == 0) {
        fprintf(stderr, "Error opening device.\n");
        return -1;
    }
    return 0;
}

void AudioPlayer_AO::pcmPlay( char * buffer, unsigned int bytes )
{
    ao_play( pcm_device, buffer, bytes );
    fprintf( stderr, "finished playback\n" );
}

void AudioPlayer_AO::pcmShutdown() {
    ao_close(pcm_device);
    ao_shutdown();
    pcm_device = 0;
}

void AudioPlayer_AO::OpenWavAndPlay( const char * filename ) 
{
    mmfile_t * mmap_wav = new mmfile_t( filename );

    if ( !mmap_wav->data ) {
        fprintf( stderr, "error mmap'ing .wav\n" );
        pcmShutdown();
        return;
    }

    // initialize system audio 
    pcmSetup();

    // read header meta info
    int offset = getPcmFormat( mmap_wav->data, mmap->size );

    // start playing at beginning of PCM samples
    pcmPlay( (char*)(mmap_wav->data + offset), mmap_wav->size - offset );

    // close the mmap stream
    delete mmap_wav;

    // close the audio system
    pcmShutdown();
}

int AudioPlayer_AO::getPcmFormat( const char * data, unsigned int * length ) 
{
    struct wavinfo_t wavinfo;
    GetWavInfo( data, length, &wavinfo );
    PrintWavinfo( &wavinfo );

    // set the PCM format from wav header
    memset( &pcm_format, 0, sizeof(pcm_format) );

    pcm_format.bits         = wavinfo.width * 8;
    pcm_format.channels     = wavinfo.channels;
    pcm_format.rate         = wavinfo.rate;
    pcm_format.byte_format  = AO_FMT_LITTLE;

    return wavinfo.dataofs;
}

void AudioPlayer_AO:OpenAndPlay( const char * filename )
{
    // FIXME: file-type detection by extension and send files to different codec handlers

    this->OpenWavAndPlay( filename );
}
