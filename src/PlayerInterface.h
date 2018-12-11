#ifndef __PlayerInterface__
#define __PlayerInterface__

enum {
    STREAM_OK       = 0,
    STREAM_ERROR    = -1
};

class PlayerInterface {
public:
    virtual ~PlayerInterface() { }
    virtual int StreamOpen()    = 0;
    virtual int SubmitFrames( unsigned char * frames, unsigned int frame_count )    = 0;
    virtual int StreamClose()   = 0;
};

#endif // __PlayerInterface__
