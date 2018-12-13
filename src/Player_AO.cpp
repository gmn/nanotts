// Player_AO.cpp

#include <stdio.h>
#include "player_ao.h"
#include "mmfile.h"

/*
================================================
    AudioPlayer_AO methods
================================================
*/
AudioPlayer_AO::AudioPlayer_AO() : pcm_device(0), pcm_driver(0)
{
    memset( &pcm_format, 0xbcbcbcbc, sizeof(pcm_format) );
}

AudioPlayer_AO::~AudioPlayer_AO()
{
    if ( pcm_device )
        pcmShutdown();
}

int AudioPlayer_AO::pcmSetup()
{
    /* -- Initialize -- */
    ao_initialize();

    /* -- Setup for default driver -- */
    pcm_driver = ao_default_driver_id();

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
}

void AudioPlayer_AO::pcmShutdown() {
    ao_close(pcm_device);
    ao_shutdown();
    pcm_device = 0;
}

void AudioPlayer_AO::OpenWavAndPlay( const char * filename )
{
    // open the audio file
    mmfile_t * mmap_wav = new mmfile_t( filename );
    if ( !mmap_wav->data ) {
        fprintf( stderr, "error mmap'ing .wav\n" );
        pcmShutdown();
        return;
    }

    // read header meta info
    int offset = getPcmFormat( mmap_wav->data, mmap_wav->size );

    // initialize system audio
    pcmSetup();

    // start playing at beginning of PCM samples
    pcmPlay( (char*)(mmap_wav->data + offset), mmap_wav->size - offset );

    fprintf( stderr, "finished playback\n" );

    // close the mmap stream
    delete mmap_wav;

    // close the audio system
    pcmShutdown();
}

int AudioPlayer_AO::getPcmFormat( unsigned char * data, unsigned int length )
{
    struct wavinfo_t wavinfo;
    GetWavInfo( data, length, &wavinfo );

    // set the PCM format from wav header
    memset( &pcm_format, 0, sizeof(pcm_format) );

    pcm_format.bits         = wavinfo.width * 8;
    pcm_format.channels     = wavinfo.channels;
    pcm_format.rate         = wavinfo.rate;
    pcm_format.byte_format  = AO_FMT_LITTLE;

    return wavinfo.dataofs;
}

void AudioPlayer_AO::OpenAndPlay( const char * filename )
{
    // FIXME: put file-type detection here. do by extension and send files to different codec handlers

    this->OpenWavAndPlay( filename );
}

void AudioPlayer_AO::DefaultPCMFormat() {
//{bits = 16, rate = 16000, channels = 1, byte_format = 1, matrix = 0x0}
    pcm_format.bits         = 16;
    pcm_format.rate         = 16000;
    pcm_format.channels     = 1;
    pcm_format.byte_format  = 1;
    pcm_format.matrix       = 0x0;
}
