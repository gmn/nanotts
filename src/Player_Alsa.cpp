
// extremely basic Alsa stream device that has hardcoded PCM parameters
#include "Player_Alsa.h"

Player_Alsa::Player_Alsa() : blocking_flag( 0 ), interface_started( false ) {
}

Player_Alsa::~Player_Alsa() {
    StreamClose();
}

int Player_Alsa::StreamOpen() {
    const char * device = "default";
    int err;

    if ( interface_started ) {
        return STREAM_ERROR;
    }

    // open the PCM device
    if ( (err = snd_pcm_open( &handle, device, SND_PCM_STREAM_PLAYBACK, blocking_flag )) < 0 ) {
        fprintf( stderr, "error: opening pcm device failed %s\n", snd_strerror(err) );
    }

    // setup our pcm state (on the snd_pcm_t handle)
    if ( (err = snd_pcm_set_params( handle,
                                    SND_PCM_FORMAT_S16_LE,
                                    SND_PCM_ACCESS_RW_INTERLEAVED,
                                    1,
                                    16000,
                                    1,
                                    50000 ) ) < 0 ) {   /* 0.05 sec */
        fprintf( stderr, "Playback open error: %s\n", snd_strerror(err) );
    }

    interface_started = true;

    return STREAM_OK;
}

int Player_Alsa::SubmitFrames( unsigned char * buffer, unsigned int frame_count )
{
    snd_pcm_sframes_t frames = snd_pcm_writei( handle, buffer, frame_count );

    if ( frames < 0 ) {
        fprintf( stderr, "error: snd_pcm_writei: %s\n", snd_strerror( frames ) );
        fflush( stdout );
        frames = snd_pcm_recover( handle, frames, 0 );
        if ( frames < 0 ) {
            fprintf( stderr, "error: snd_pcm_writei failed: %s\n", snd_strerror( frames ) );
            fflush( stdout );
            return STREAM_ERROR;
        }
    }

//    if ( frames > 0 ) {
//        printf( "Played %li frames\n", frames );
//    }

    return STREAM_OK;
}

int Player_Alsa::StreamClose()
{
    if ( interface_started ) {
        snd_pcm_close( handle );
        interface_started = false;
        return STREAM_OK;
    } else {
        return STREAM_ERROR;
    }
}

