#ifndef __StreamHandler__
#define __StreamHandler__

#include "PlayerInterface.h"

class StreamHandler : public PlayerInterface {
public:
    PlayerInterface * player;

public:
    StreamHandler();
    virtual ~StreamHandler();
    virtual int StreamOpen();
    virtual int SubmitFrames( unsigned char * frames, unsigned int frame_count );
    virtual int StreamClose();
};


#if 0
StreamHandler::StreamHandler() : player( 0 ) {
}

StreamHandler::~StreamHandler() {
    StreamClose();
    if ( player ) {
        delete player;
        player = 0;
    }
}

int StreamHandler::StreamOpen() {
    if ( player ) {
        player->StreamOpen();
    }
    return 0;
}

int StreamHandler::SubmitFrames( unsigned char * frames, unsigned int frame_count ) {
    if ( player ) {
        player->SubmitFrames( frames, frame_count );
    }
    return 0;
}

int StreamHandler::StreamClose() {
    if ( player ) {
        player->StreamClose();
    }
    return 0;
}
#endif

#endif // __StreamHandler__
