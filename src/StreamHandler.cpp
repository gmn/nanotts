
#include "StreamHandler.h"

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


