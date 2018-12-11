#ifndef __Player_Alsa__
#define __Player_Alsa__

#include "PlayerInterface.h"
#include <alsa/asoundlib.h>

class Player_Alsa : public PlayerInterface {
public:
    snd_pcm_t *     handle;
    int             blocking_flag;          // 0 = blocking; SND_PCM_NONBLOCK = not blocking
    bool            interface_started;

public:
    Player_Alsa();
    ~Player_Alsa();
    int StreamOpen();
    int SubmitFrames( unsigned char * frames, unsigned int frame_count );
    int StreamClose();
};

#endif // __Player_Alsa__
