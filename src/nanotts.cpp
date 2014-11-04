/* nanotts.cpp
 *
 * Copyright (C) 2014 Greg Naughton <greg@naughton.org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *    Convert text to .wav using svox text-to-speech system.
 *    Rewrite of pico2wave.c
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

extern "C" {
#include "svoxpico/picoapi.h"
#include "svoxpico/picoapid.h"
#include "svoxpico/picoos.h"
}

#include "PicoVoices.h"


/*
================================================
Nano

Singleton class to handle workings of this program
================================================
*/
class Nano 
{
private:
    enum inputMode_t {
        IN_STDIN,
        IN_CMDLINE_ARG,
        IN_FILE
    };

    enum outputMode_t {
        OUT_PLAYBACK,
        OUT_STDOUT,
        OUT_FILE
    };

    inputMode_t     in_mode;
    outputMode_t    out_mode;

    const int       my_argc;
    const char **   my_argv;
    const char *    exename;

    char            voice[ 6 ];
    char *          voicedir;
    char *          out_filename;
    char *          in_filename;
    char *          words;
    FILE *          in_fp;
    FILE *          out_fp;

    char *          copy_arg( int );
    
    ao_device *         pcm_device;
    ao_sample_format    pcm_format;
    int                 pcm_driver;

    char            read_buffer[8000];

public:
    Nano( const int, const char ** );

    void PrintUsage();
    void check_args();
    void setup_input_output() ;

    int getInput( char ** data, int * bytes );
    void sendOutput( char * data, int size );

    void pcmSetup();
    void pcmPlay( char * , unsigned int );
    void pcmShutdown();

    const char * getVoice();
    const char * getPath();
};

Nano::Nano( const int i, const char ** v ) : my_argc(i), my_argv(v) {
    strcpy( voice, "en-GB" );
    voicedir = 0;
    out_filename = 0;
    in_filename = 0;
    words = 0;
    in_fp = 0;
    out_fp = 0;
    pcm_device = 0;
}

Nano::~Nano() {
    if ( voicedir )
        delete[] voicedir;
    if ( out_filename )
        delete[] out_filename;
    if ( in_filename )
        delete[] in_filename;
    if ( words )
        delete[] words;

    if ( pcm_device ) {
        pcmShutdown();
    }
}

void Nano::PrintUsage() {
    struct help {
        const char *arg;
        const char *desc;
    } helps[] = {
        { "-h, --help", "displays this" },
        { "-v <voice>", "select voice. Default: en-GB" },
        { "-p <directory>", "Lingware voices directory. Default: \"./lang\"" },
        { "-w <words>", "words. must be correctly quoted" },
        { "-f <filename>", "filename to read input from" },
        { "-o [filename]", "write output to file" },
        { "-c ", "send PCM output to stdout" }
    };

    const char * program = strrchr( my_argv[0], '/' );
    program = !program ? my_argv[0] : program + 1;
    this->exename = program;

    printf( "usage: %s [options]\n", exename );

    unsigned long long int size = *(&helps + 1) - helps;

    for ( unsigned int i = 0; i < size; i++ ) {
        printf( " %-14s   %s\n", helps[i].arg, helps[i].desc );
    }
}

// get argument at index, make a copy and return it
char * Nano::copy_arg( int index ) {
    if ( index + 1 >= argc ) 
        return 0;

    int len = strlen( my_argv[index+1] );
    char * buf = new char[len+1];
    memset( buf, 0, len + 1 );
    strcpy( buf, my_argv[index+1] );
    return buf;
}

int Nano::check_args() {
    // INPUTS               OUTPUTS
    // -w <words>           -o <file>
    // -f <file>            -c stdout
    // default: stdin       default: pcm

    /* heuristic:
        if -w | -f detected, either trump default. if both, take last seen.
        if -o | -c detected, either trump, if both, take last 

        for -w, -f, -o, sanity check their argument, else print error & exit
    */

    // DEFAULTS
    in_mode = IN_STDIN;
    out_mode = OUT_FILE;


    for ( int i = 0; i < my_argc; i++ )
    {
        if ( strcmp( my_argv[i], "-h" ) == 0 || strcmp( my_argv[i], "--help" ) == 0 ) {
            PrintUsage();
            return -1;
        }

        // INPUTS
        else if ( strcmp( my_argv[i], "-w" ) == 0 ) {
            in_mode = IN_CMDLINE_ARG;
            if ( (words = copy_arg( i + 1 )) == 0 )
                return -1;
        } 
        else if ( strcmp( my_argv[i], "-f" ) == 0 ) {
            in_mode = IN_FILE;
            if ( (in_filename = copy_arg( i + 1 )) == 0 )
                return -1;
        }

        // OUTPUTS
        else if ( strcmp( my_argv[i], "-o" ) == 0 ) {
            out_mode = OUT_FILE;
            if ( (out_filename = copy_arg( i + 1 )) == 0 )
                return -1;
        }
        else if ( strcmp( my_argv[i], "-c" ) == 0 ) {
            out_mode = OUT_STDOUT;
        }

        // SVOX
        else if ( strcmp( my_argv[i], "-v" ) == 0 ) {
            if ( i + 1 >= argc )
                return -1;
            strncpy( voice, my_argv[i+1], 5 );
            voice[5] = 0;
        }
        else if ( strcmp( my_argv[i], "-p" ) == 0 ) {
            if ( (voicedir = copy_arg( i + 1 )) == 0 )
                return -1;
        }
    }

    if ( !voicedir ) {
        voicedir = new char[8];
        strcpy( voicedir, "./lang/" );
    }

    return 0;
}

int Nano::setup_input_output() {

    switch ( in_mode ) {
    case IN_STDIN:
        // detect if stdin is coming from a pipe
        if ( ! isatty(fileno(stdin)) ) // On windows prefix with underscores: _isatty, _fileno
        {
            in_fp = stdin;
        }

        break;
    case IN_CMDLINE_ARG:
        // words
        break;
    case IN_FILE:
        if ( (in_fp = fopen( in_filename, "r" )) == 0 )
            return -1;
        break;
    default:
        break;
    };

    switch ( out_mode ) {
    case OUT_PLAYBACK:
        if ( 0 != pcmSetup() )
            return -1;
        break;
    case OUT_STDOUT:
        break;
    case OUT_FILE:
        if ( (out_fp = fopen( out_filename, "w" )) == 0 )
            return -1;
        break;
    }

    return 0;
}

// puts input into *data, and number_bytes into bytes
// returns 0 on no more data
int Nano::getInput( char ** data, int * bytes ) 
{
    int total = 0;
    fread( read_buffer, 1, 8000, in_fp );
    *data = read_buffer;
    *bytes = total;
    return total;
}

// 
void Nano::sendOutput( char * data, unsigned int size ) {
    unsigned int usize = (unsigned) size;
    pcmPlay( data, usize );
}

int Nano::pcmSetup() {
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

void Nano::pcmPlay( char * buffer, unsigned int bytes ) {
    ao_play( pcm_device, buffer, bytes );
}

void Nano::pcmShutdown() {
    ao_close(pcm_device);
    ao_shutdown();
    pcm_device = 0;
}

const char * Nano::getVoice() {
    return voice;
}

const char * Nano::getPath() {
    return voicedir;
}

/*
================================================
Pico

class to encapsulate the workings of the SVox PicoTTS System
================================================
*/
class Pico {
private:
    PicoVoices_t    voices;

    pico_System     picoSystem;
    pico_Resource   picoTaResource;
    pico_Resource   picoSgResource;
    pico_Resource   picoUtppResource;
    pico_Engine     picoEngine;
    picoos_SDFile   sdOutFile;

    pico_Char *     local_text;
    pico_Int16      text_remaining;
    char *          picoLingwarePath;

public:
    Pico() ;
    ~Pico() ;

    void setPath( const char * path =0 );
    int setup() ;
    void cleanup() ;
    void sendTextForProcessing() ;
    unsigned int processText() ;

    void setVoice( const char * );
};


Pico::Pico() {
    picoSystem          = 0;
    picoTaResource      = 0;
    picoSgResource      = 0;
    picoUtppResource    = 0;
    picoEngine          = 0;
    sdOutFile           = 0;
    picoLingwarePath    = 0;
}

Pico::~Pico() {
    cleanup();
}

void Pico::setPath( const char * path ) {
    if ( !path )
        path = "./lang/";
    unsigned int len = strlen( path ) + 1;
    picoLingwarePath = new char[ len ];
    strcpy( picoLingwarePath, path );
}

int Pico::setup() 
{
    /* adaptation layer variables */
    void *          picoMemArea             = 0;
    pico_Char *     picoTaFileName          = 0;
    pico_Char *     picoSgFileName          = 0;
    pico_Char *     picoUtppFileName        = 0;
    pico_Char *     picoTaResourceName      = 0;
    pico_Char *     picoSgResourceName      = 0;
    pico_Char *     picoUtppResourceName    = 0;
    const int       PICO_MEM_SIZE           = 8000;
    const char *    PICO_VOICE_NAME         = "PicoVoice";
    int             ret, getstatus;
    pico_Retstring  outMessage;



    // FIXME: does pico free memArea?  Will need to profile: valgrind or gprof
    picoMemArea = malloc( PICO_MEM_SIZE );

    if ( (ret = pico_initialize( picoMemArea, PICO_MEM_SIZE, &picoSystem )) ) {
        pico_getSystemStatusMessage(picoSystem, ret, outMessage);
        fprintf(stderr, "Cannot initialize pico (%i): %s\n", ret, outMessage);
        goto terminate;
    }

    /* Load the text analysis Lingware resource file.   */
    // FIXME: free this?
    picoTaFileName = (pico_Char *) malloc( PICO_MAX_DATAPATH_NAME_SIZE + PICO_MAX_FILE_NAME_SIZE );

    // path
    if ( !picoLingwarePath )
        setPath();
    strcpy((char *) picoTaFileName, picoLingwarePath);
    
    // check for connecting slash
    unsigned int len = strlen( picoTaFileName );
    if ( picoTaFileName[len-1] != '/' )
        strcat((char*) picoTaFileName, "/");

    // langfile name
    strcat( (char *) picoTaFileName, voices.getTaName() );

    // attempt to load it
    if ( (ret = pico_loadResource(picoSystem, picoTaFileName, &picoTaResource)) ) {
        pico_getSystemStatusMessage(picoSystem, ret, outMessage);
        fprintf(stderr, "Cannot load text analysis resource file (%i): %s\n", ret, outMessage);
        goto unloadTaResource;
    }

    /* Load the signal generation Lingware resource file.   */
    // FIXME: free?
    picoSgFileName      = (pico_Char *) malloc( PICO_MAX_DATAPATH_NAME_SIZE + PICO_MAX_FILE_NAME_SIZE );

    strcpy((char *) picoSgFileName,   picoLingwarePath );
    strcat((char *) picoSgFileName,   voices.getSgName() );

    if ( (ret = pico_loadResource(picoSystem, picoSgFileName, &picoSgResource)) ) {
        pico_getSystemStatusMessage(picoSystem, ret, outMessage);
        fprintf(stderr, "Cannot load signal generation Lingware resource file (%i): %s\n", ret, outMessage);
        goto unloadSgResource;
    }

    /* Get the text analysis resource name.     */
    // FIXME: free?
    picoTaResourceName  = (pico_Char *) malloc( PICO_MAX_RESOURCE_NAME_SIZE );
    if((ret = pico_getResourceName( picoSystem, picoTaResource, (char *) picoTaResourceName ))) {
        pico_getSystemStatusMessage(picoSystem, ret, outMessage);
        fprintf(stderr, "Cannot get the text analysis resource name (%i): %s\n", ret, outMessage);
        goto unloadUtppResource;
    }

    /* Get the signal generation resource name. */
    // FIXME: free?
    picoSgResourceName  = (pico_Char *) malloc( PICO_MAX_RESOURCE_NAME_SIZE );
    if((ret = pico_getResourceName( picoSystem, picoSgResource, (char *) picoSgResourceName ))) {
        pico_getSystemStatusMessage(picoSystem, ret, outMessage);
        fprintf(stderr, "Cannot get the signal generation resource name (%i): %s\n", ret, outMessage);
        goto unloadUtppResource;
    }

    /* Create a voice definition.   */
    if((ret = pico_createVoiceDefinition( picoSystem, (const pico_Char *) PICO_VOICE_NAME ))) {
        pico_getSystemStatusMessage(picoSystem, ret, outMessage);
        fprintf(stderr, "Cannot create voice definition (%i): %s\n", ret, outMessage);
        goto unloadUtppResource;
    }

    /* Add the text analysis resource to the voice. */
    if((ret = pico_addResourceToVoiceDefinition( picoSystem, (const pico_Char *) PICO_VOICE_NAME, picoTaResourceName ))) {
        pico_getSystemStatusMessage(picoSystem, ret, outMessage);
        fprintf(stderr, "Cannot add the text analysis resource to the voice (%i): %s\n", ret, outMessage);
        goto unloadUtppResource;
    }

    /* Add the signal generation resource to the voice. */
    if((ret = pico_addResourceToVoiceDefinition( picoSystem, (const pico_Char *) PICO_VOICE_NAME, picoSgResourceName ))) {
        pico_getSystemStatusMessage(picoSystem, ret, outMessage);
        fprintf(stderr, "Cannot add the signal generation resource to the voice (%i): %s\n", ret, outMessage);
        goto unloadUtppResource;
    }

    /* Create a new Pico engine. */
    if((ret = pico_newEngine( picoSystem, (const pico_Char *) PICO_VOICE_NAME, &picoEngine ))) {
        pico_getSystemStatusMessage(picoSystem, ret, outMessage);
        fprintf(stderr, "Cannot create a new pico engine (%i): %s\n", ret, outMessage);
        goto disposeEngine;
    }

    return 0;


    //
    // partial shutdowns below this line
    //  for pico cleanup in case of startup abort
    //
disposeEngine:
    if ( sdOutFile ) {
        picoos_Common common = (picoos_Common) pico_sysGetCommon(picoSystem);
        picoos_sdfCloseOut(common, &sdOutFile);
        sdOutFile = 0;
    }

    if (picoEngine) {
        pico_disposeEngine( picoSystem, &picoEngine );
        pico_releaseVoiceDefinition( picoSystem, (pico_Char *) PICO_VOICE_NAME );
        picoEngine = 0;
    }
unloadUtppResource:
    if (picoUtppResource) {
        pico_unloadResource( picoSystem, &picoUtppResource );
        picoUtppResource = 0;
    }
unloadSgResource:
    if (picoSgResource) {
        pico_unloadResource( picoSystem, &picoSgResource );
        picoSgResource = 0;
    }
unloadTaResource:
    if (picoTaResource) {  
        pico_unloadResource( picoSystem, &picoTaResource );
        picoTaResource = 0;
    }
terminate:
    if (picoSystem) {
        pico_terminate(&picoSystem);
        picoSystem = 0;
    }

    return -1;
}

void Pico::cleanup() 
{
    if ( sdOutFile ) {
        // close output wave file
        picoos_sdfCloseOut(common, &sdOutFile);
        sdOutFile = 0;
    }

    if (picoEngine) {
        pico_disposeEngine( picoSystem, &picoEngine );
        pico_releaseVoiceDefinition( picoSystem, (pico_Char *) PICO_VOICE_NAME );
        picoEngine = 0;
    }

    if (picoUtppResource) {
        pico_unloadResource( picoSystem, &picoUtppResource );
        picoUtppResource = 0;
    }

    if (picoSgResource) {
        pico_unloadResource( picoSystem, &picoSgResource );
        picoSgResource = 0;
    }

    if (picoTaResource) {
        pico_unloadResource( picoSystem, &picoTaResource );
        picoTaResource = 0;
    }

    if (picoSystem) {
        pico_terminate(&picoSystem);
        picoSystem = 0;
    }
}

void Pico::sendTextForProcessing( const char * words, int word_len )
{
    local_text      = (pico_Char *) words;
    text_remaining  = word_len;
}

int Pico::processText( char * pcm_buffer, int buf_len ) 
{
    const int       MAX_OUTBUF_SIZE     = 128;
    const int       bufferSize          = 256;
    pico_Char *     inp                 = 0;
    int             picoSynthAbort      = 0;
    pico_Int16      bytes_sent, bytes_recv, out_data_type;
    short           outbuf[MAX_OUTBUF_SIZE/2];
    pico_Retstring  outMessage;


    inp = (pico_Char *) local_text;
    unsigned int bufused = 0;


/*
    picoos_bool done = TRUE;
    if(TRUE != (done = picoos_sdfOpenOut(common, &sdOutFile,
        (picoos_char *) wavefile, SAMPLE_FREQ_16KHZ, PICOOS_ENC_LIN)))
    {   
        fprintf(stderr, "Cannot open output wave file\n");
        return -1;
    }
*/


    /* synthesis loop   */
    while (text_remaining) 
    {
        /* Feed the text into the engine.   */
        if((ret = pico_putTextUtf8( picoEngine, inp, text_remaining, &bytes_sent ))) {
            pico_getSystemStatusMessage(picoSystem, ret, outMessage);
            fprintf(stderr, "Cannot put Text (%i): %s\n", ret, outMessage);
            return -2;
        }

        text_remaining -= bytes_sent;
        inp += bytes_sent;

        do {
            if (picoSynthAbort) {
                return -3;
            }

            /* Retrieve the samples */ 
            getstatus = pico_getData( picoEngine, (void *) outbuf, MAX_OUTBUF_SIZE, &bytes_recv, &out_data_type );
            if ( (getstatus !=PICO_STEP_BUSY) && (getstatus !=PICO_STEP_IDLE) ) {
                pico_getSystemStatusMessage(picoSystem, getstatus, outMessage);
                fprintf(stderr, "Cannot get Data (%i): %s\n", getstatus, outMessage);
                return -4;
            }

            if ( bytes_recv > 0 ) 
            {
                memcpy( pcm_buffer+bufused, (int8_t *)outbuf, bytes_recv );
                bufused += bytes_recv;
            }

            if ( bufused >= buf_len ) 
            {
                return bufused;
            }

#if 0
            /* ...and add them to the buffer. */
            if (bytes_recv) {
                if ((bufused + bytes_recv) <= bufferSize) {
                    memcpy( pcm_buffer+bufused, (int8_t *)outbuf, bytes_recv );
                    bufused += bytes_recv;
                } else {
/*
                    done = picoos_sdfPutSamples(
                                        sdOutFile,
                                        bufused / 2,
                                        (picoos_int16*) (buffer));
*/
                    bufused = 0;
                    memcpy( pcm_buffer, (int8_t *)outbuf, bytes_recv );
                    bufused += bytes_recv;
                }
            }
#endif

        } while (PICO_STEP_BUSY == getstatus);


        /* This chunk of synthesis is finished; pass the remaining samples. */
        if (!picoSynthAbort) {
/*
                    done = picoos_sdfPutSamples(
                                        sdOutFile,
                                        bufused / 2,
                                        (picoos_int16*) (buffer));
*/
        }
        picoSynthAbort = 0;
    }

    return bufused;
}

void Pico::setVoice( const char * v ) {
    voices.setVoice( v );
}





int main( int argc, const char ** argv ) 
{
    Nano * nano = new Nano( argc, argv );

    // 
    if ( nano->check_args() < 0 ) {
        delete nano;
        exit( 0 );
    }

    // 
    if ( nano->setup_input_output() < 0 ) {
        delete nano;
        exit( 0 );
    }

    //
    Pico *pico = new Pico();
    pico->setup();
    pico->setVoice( nano->getVoice() );
    pico->setPath( nano->getPath() );


    char * words = 0;
    int length = 0;
    char pcm_buffer[ 8000 ];
    int bytes = 0;

    do {
        nano->getInput( &words, &length );
        if ( 0 == length )
            break;

        pico->sendTextForProcessing( words, length ); 

        do {
            bytes = pico->processText( pcm_buffer, 8000 );
            if ( bytes < 0 ) 
                goto speedy_exit;
            if ( 0 == bytes )
                break;
        }
        while(1)
    }
    while(1);

    //nano->sendOutput( pcm_buffer, bytes );

speedy_exit:
    delete pico;
    delete nano;
}
