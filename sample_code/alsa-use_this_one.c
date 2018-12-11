
/*
 *  This extra small demo sends a random samples to your speakers.
 */
#include <alsa/asoundlib.h>

int main( int argc, char ** argv )
{
    const char *device = "default";         /* playback device */
    unsigned char buffer[ 16*1024 ];        /* read buffer */

    int err;
    snd_pcm_t *handle;
    snd_pcm_sframes_t frames;
    FILE * fp = 0;
    size_t bytes_read;
    int blocking_flag = 0; // 0 = blocking; SND_PCM_NONBLOCK = not blocking

    // check file argument
    if ( argc <= 1 ) {
        printf( "no input\n" );
        return 127;
    }

    // open the file for reading
    fp = fopen( argv[1], "rb" );
    if ( !fp ) {
        printf("error: couldn't open: %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    // discard the header
    fread( buffer, 1, 44, fp );

    // open the PCM device
    if ( (err = snd_pcm_open( &handle, device, SND_PCM_STREAM_PLAYBACK, blocking_flag )) < 0 ) {
        printf("Playback open error: %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
    }

    // setup our pcm state (on the snd_pcm_t handle)
    if ((err = snd_pcm_set_params(handle,
                                  SND_PCM_FORMAT_S16_LE,
                                  SND_PCM_ACCESS_RW_INTERLEAVED,
                                  1,
                                  16000,
                                  1,
                                  50000)) < 0) {   /* 0.05 sec */
        printf("Playback open error: %s\n", snd_strerror(err));
        exit( EXIT_FAILURE );
    }

    // read loop
    while( 1 ) {
        bytes_read = fread( buffer, 1, sizeof(buffer), fp );
        if ( bytes_read == 0 ) {
            break;
        }

        frames = snd_pcm_writei( handle, buffer, bytes_read / 2 /*(frame is 16-bit)*/  );
        if (frames < 0) {
            printf( "error: snd_pcm_writei: %s\n", snd_strerror( frames ) );
            fflush( stdout );
            frames = snd_pcm_recover(handle, frames, 0);
        }
        if (frames < 0) {
            printf("snd_pcm_writei failed: %s\n", snd_strerror(frames));
            fflush( stdout );
            break;
        }
        if ( frames > 0 ) {
            printf("Played %li frames\n", frames);
        }
    }

    fclose( fp );
    snd_pcm_close( handle );

    return 0;
}

