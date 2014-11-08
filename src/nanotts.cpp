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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h> // mmap

#include <ao/ao.h>


extern "C" {
#include "svoxpico/picoapi.h"
#include "svoxpico/picoapid.h"
#include "svoxpico/picoos.h"
}

#include "PicoVoices.h"
#include "mmfile.h"

class Nano;


/*
================================================
Listener

stream class, for exchanging byte-streams between producer/consumer
================================================
*/
template <typename type>
class Listener {
    type * data;
    unsigned int read_p;
    void (Nano::*consume)( short *, unsigned int );
    Nano* nano_class;
public:
    Listener() : data(0), read_p(0), consume(0), nano_class(0) {
    }
    virtual ~Listener() {
    }

    void writeData( type * data, unsigned int byte_size );
    void setCallback( void (Nano::*con_f)( short *, unsigned int ), Nano* ) ;
};

template <typename type>
void Listener<type>::writeData( type * data, unsigned int byte_size ) {
    if ( this->consume ) {
        void (Nano::*pointer)(short *, unsigned int) = this->consume;
        (*nano_class.*pointer)( data, byte_size );
    }
}

template <typename type>
void Listener<type>::setCallback( void (Nano::*con_f)( short *, unsigned int ), Nano* nano_class ) {
    this->consume = con_f;
    this->nano_class = nano_class;
}
//////////////////////////////////////////////////////////////////



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
        IN_SINGLE_FILE,
        IN_MULTIPLE_FILES
    };

    enum outputMode_t {
        OUT_STDOUT,
        OUT_SINGLE_FILE,
        OUT_MULTIPLE_FILES
    };

    inputMode_t     in_mode;
    outputMode_t    out_mode;

    const int       my_argc;
    const char **   my_argv;
    const char *    exename;

    char *          voice;
    char *          voicedir;
    char *          prefix;
    char *          out_filename;
    char *          in_filename;
    char *          words;
    FILE *          in_fp;
    FILE *          out_fp;

    char *          copy_arg( int );
    
    ao_device *         pcm_device;
    ao_sample_format    pcm_format;
    int                 pcm_driver;

    unsigned char * input_buffer;
    unsigned int    input_size;

    mmfile_t *      mmfile;

    Listener<short> stdout_processor;
    void write_short_to_stdout( short *, unsigned int );

public:
    bool            silence_output;

    Nano( const int, const char ** );
    ~Nano();

    void PrintUsage();
    int check_args();
    int setup_input_output() ;

    int getInput( unsigned char ** data, unsigned int * bytes );
    void playOutput();

    int  pcmSetup();
    void pcmPlay( char * , unsigned int );
    void pcmShutdown();

    const char * getVoice();
    const char * getPath();

    const char * outFilename() const { return out_filename; }

    Listener<short> * getListener() ; 
};

Nano::Nano( const int i, const char ** v ) : my_argc(i), my_argv(v) {
    voice = 0;
    voicedir = 0;
    prefix = 0;
    out_filename = 0;
    in_filename = 0;
    words = 0;
    in_fp = 0;
    out_fp = 0;
    pcm_device = 0;
    input_buffer = 0;
    input_size = 0;

    silence_output = false;
    stdout_processor.setCallback( &Nano::write_short_to_stdout, this );
}

Nano::~Nano() {
    if ( voice )
        delete[] voice;
    if ( voicedir )
        delete[] voicedir;
    if ( prefix )
        delete[] prefix;
    if ( out_filename )
        delete[] out_filename;
    if ( in_filename )
        delete[] in_filename;
    if ( words )
        delete[] words;

    if ( pcm_device ) {
        pcmShutdown();
    }

    if ( input_buffer ) {
        switch ( in_mode ) {
        case IN_STDIN: 
            delete[] input_buffer;
            break;
        case IN_SINGLE_FILE:
            delete mmfile;
            mmfile = 0;
        default:
            break;
        }
        input_buffer = 0;
    }

    if ( in_fp != 0 && in_fp != stdin ) {
        fclose( in_fp );
        in_fp = 0;
    }
    if ( out_fp != 0 && out_fp != stdout ) {
        fclose( out_fp );
        out_fp = 0;
    }

}

void Nano::PrintUsage() {

    const char * program = strrchr( my_argv[0], '/' );
    program = !program ? my_argv[0] : program + 1;
    this->exename = program;

    printf( "usage: %s [options]\n", exename );

    char line1[ 80 ];
    char line2[ 80 ];
    char line3[ 80 ];
    memset( line1, 0, 80 );
    memset( line2, 0, 80 );
    sprintf( line1, "  %s -f <filename> --no-play -o file1.wav", exename );
    sprintf( line2, "  echo \"Mary had a little lamb\" | %s", exename );
    sprintf( line3, "  %s -w \"Once upon a midnight dreary\" -v en-US", exename );

    struct help {
        const char *arg;
        const char *desc;
    } help_lines[] = {
        { "-h, --help", "displays this" },
        { "-v <voice>", "select voice. Default: en-GB" },
        { "-l <directory>", "Lingware voices directory. Default: \"./lang\"" },
        { "-w <words>", "words. must be correctly quoted" },
        { "-f <filename>", "filename to read input from" },
//        { "-p <prefix>", "write output to multiple numbered files with prefix" },
        { "-o <filename>", "write output to single file; overrides prefix" },
        { "--no-play|-m", "do NOT play output on PC's soundcard" },
        { "-c ", "send PCM output to stdout" },
//        { "--files", "set multiple input files" },
        { " ", " " },
        { "Examples: ", " " },
        { line1, " " },
        { line2, " " },
        { line3, " " },
    };

    unsigned long long int size = *(&help_lines + 1) - help_lines;

    for ( unsigned int i = 0; i < size; i++ ) {
        printf( " %-14s   %s\n", help_lines[i].arg, help_lines[i].desc );
    }
}

// get argument at index, make a copy and return it
char * Nano::copy_arg( int index ) 
{
    if ( index >= my_argc ) 
        return 0;

    int len = strlen( my_argv[index] );
    char * buf = new char[len+1];
    memset( buf, 0, len + 1 );
    strcpy( buf, my_argv[index] );
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
    out_mode = OUT_SINGLE_FILE;


    for ( int i = 1; i < my_argc; i++ )
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
            in_mode = IN_SINGLE_FILE;
            if ( (in_filename = copy_arg( i + 1 )) == 0 )
                return -1;
        }
        else if ( strcmp( my_argv[i], "--files" ) == 0 ) {
            in_mode = IN_MULTIPLE_FILES;
            // FIXME: get array of char*filename
            if ( (in_filename = copy_arg( i + 1 )) == 0 )
                return -1;
        }

        // OUTPUTS
        else if ( strcmp( my_argv[i], "-o" ) == 0 ) {
            out_mode = OUT_SINGLE_FILE;
            if ( (out_filename = copy_arg( i + 1 )) == 0 )
                return -1;
        }
        else if ( strcmp( my_argv[i], "-p" ) == 0 ) {
            out_mode = OUT_MULTIPLE_FILES;
            if ( (prefix = copy_arg( i + 1 )) == 0 )
                return -1;
        }
        else if ( strcmp( my_argv[i], "-c" ) == 0 ) {
            out_mode = OUT_STDOUT;
        }

        // SVOX
        else if ( strcmp( my_argv[i], "-v" ) == 0 ) {
            if ( (voice = copy_arg( i + 1 )) == 0 )
                return -1;
        }
        else if ( strcmp( my_argv[i], "-l" ) == 0 ) {
            if ( (voicedir = copy_arg( i + 1 )) == 0 )
                return -1;
        }

        // OTHER
        else if ( strcmp( my_argv[i], "-m" ) == 0 || strcmp( my_argv[i], "--no-play" ) == 0 ) {
            silence_output = true;
        } 
    }

    // post-DEFAULTS
    if ( !voice ) {
        voice = new char[6];
        strcpy( voice, "en-GB" );
    }
    if ( !voicedir ) {
        voicedir = new char[8];
        strcpy( voicedir, "./lang/" );
    }

    // FIXME: temporary
    if ( !out_filename ) {
        out_filename = new char[20];
        memset( out_filename, 0, 20 );
        strcpy( out_filename, "nanotts-001.wav" );
    }
    if ( !prefix ) {
        prefix = new char[20];
        memset( prefix, 0, 20 );
        strcpy( prefix, "nanotts-001" );
    }

    return 0;
}

int Nano::setup_input_output() 
{
#define __NOT_IMPL__ do{fprintf(stderr," ** not implemented ** \n");return -1;}while(0);

    switch ( in_mode ) {
    case IN_STDIN:
        // detect if stdin is coming from a pipe
        if ( ! isatty(fileno(stdin)) ) { // On windows prefix with underscores: _isatty, _fileno
            in_fp = stdin;
        } else {
            fprintf( stderr, " **error: reading from stdin.  (try -h for help)\n" );
            return -1;
        }
        break;
    case IN_SINGLE_FILE:
/*
        if ( (in_fp = fopen( in_filename, "rb" )) == 0 ) {
            fprintf( stderr, " **error: opening file: \"%s\"\n", in_filename );
            return -1;
        }
*/
        break;

    case IN_CMDLINE_ARG:
        break;
    case IN_MULTIPLE_FILES:
    default:
        __NOT_IMPL__
        break;
    };

    switch ( out_mode ) {
    case OUT_SINGLE_FILE:
        break;

    case OUT_STDOUT:
        out_fp = stdout;
        fprintf( stderr, "writing pcm stream to stdout\n" );
        break;
    case OUT_MULTIPLE_FILES:
    default:
        __NOT_IMPL__
        break;
    }

    return 0;
}

// puts input into *data, and number_bytes into bytes
// returns 0 on no more data
int Nano::getInput( unsigned char ** data, unsigned int * bytes ) 
{
    switch( in_mode ) {
    case IN_STDIN:
        input_buffer = new unsigned char[ 1000000 ];
        memset( input_buffer, 0, 1000000 );
        input_size = fread( input_buffer, 1, 1000000, stdin );
 
        *data = input_buffer;
        *bytes = input_size + 1;
        fprintf( stderr, "read: %u bytes from stdin\n", input_size );
        break;
    case IN_SINGLE_FILE:
        mmfile = new mmfile_t( in_filename );

        *data = mmfile->data;
        *bytes = mmfile->size + 1;
        fprintf( stderr, "read: %u bytes from file:\"%s\"\n", mmfile->size, in_filename );
        break;
    case IN_CMDLINE_ARG:
        *data = (unsigned char *)words;
        *bytes = strlen(words) + 1;
        fprintf( stderr, "read: %u bytes from command line\n", *bytes );
        break;
    default:
        fprintf( stderr, "unknown input\n" );
        return -1;
    }
    
    return 0;
}

// 
void Nano::playOutput() 
{
    if ( silence_output )
        return;
    if ( out_mode == OUT_STDOUT )
        return;

    pcmSetup();

    mmfile_t * mmap_wav = new mmfile_t( out_filename );

    if ( !mmap_wav->data ) {
        fprintf( stderr, "error mmap'ing .wav\n" );
        pcmShutdown();
        return;
    }

    pcmPlay( (char*)mmap_wav->data, mmap_wav->size );

    delete mmap_wav;

    pcmShutdown();
}

int Nano::pcmSetup() 
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

void Nano::pcmPlay( char * buffer, unsigned int bytes ) 
{
    ao_play( pcm_device, buffer, bytes );
    fprintf( stderr, "finished playback\n" );
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

void Nano::write_short_to_stdout( short * data, unsigned int shorts ) {
    if ( out_mode == OUT_STDOUT )
        fwrite( data, 2, shorts, out_fp );
}

Listener<short> * Nano::getListener() { 
    if ( out_mode != OUT_STDOUT )
        return 0;
    return &stdout_processor; 
}
//////////////////////////////////////////////////////////////////




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
    pico_Engine     picoEngine;
    picoos_SDFile   sdOutFile;
    char *          out_filename;

    pico_Char *     local_text;
    pico_Int16      text_remaining;
    long long int   total_text_length;
    char *          picoLingwarePath;

    char            picoVoiceName[10];
    Listener<short> * listener;
public:
    Pico() ;
    ~Pico() ;

    void setPath( const char * path =0 );
    int setup() ;
    void cleanup() ;
    void sendTextForProcessing( unsigned char *, long long int ) ;
    int process() ;

    int setVoice( const char * );
    void setOutFilename( const char * fn ) { out_filename = const_cast<char*>(fn); }

    int fileSize( const char * filename ) ;
    void setListener( Listener<short> * );
};


Pico::Pico() {
    picoSystem          = 0;
    picoTaResource      = 0;
    picoSgResource      = 0;
    picoEngine          = 0;
    sdOutFile           = 0;
    picoLingwarePath    = 0;
    out_filename        = 0;

    strcpy( picoVoiceName, "PicoVoice" );

    total_text_length   = 0;
    text_remaining      = 0;
    listener            = 0;
}

Pico::~Pico() {
    if ( picoLingwarePath ) {
        delete[] picoLingwarePath;
    }

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
    pico_Char *     picoTaResourceName      = 0;
    pico_Char *     picoSgResourceName      = 0;
    const int       PICO_MEM_SIZE           = 2500000;
    pico_Retstring  outMessage;
    int             ret;



    // FIXME: does pico free memArea?  We need to profile: valgrind or gprof
    picoMemArea = malloc( PICO_MEM_SIZE );

    if ( (ret = pico_initialize( picoMemArea, PICO_MEM_SIZE, &picoSystem )) ) {
        pico_getSystemStatusMessage(picoSystem, ret, outMessage);
        fprintf(stderr, "Cannot initialize pico (%i): %s\n", ret, outMessage);

        pico_terminate(&picoSystem);
        picoSystem = 0;
        return -1;
    }

    /* Load the text analysis Lingware resource file.   */
    // FIXME: free this?
    picoTaFileName = (pico_Char *) malloc( PICO_MAX_DATAPATH_NAME_SIZE + PICO_MAX_FILE_NAME_SIZE );

    // path
    if ( !picoLingwarePath )
        setPath();
    strcpy((char *) picoTaFileName, picoLingwarePath);
    
    // check for connecting slash
    unsigned int len = strlen( (const char*)picoTaFileName );
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
    picoSgFileName = (pico_Char *) malloc( PICO_MAX_DATAPATH_NAME_SIZE + PICO_MAX_FILE_NAME_SIZE );

    strcpy((char *) picoSgFileName,   picoLingwarePath );
    strcat((char *) picoSgFileName,   voices.getSgName() );

    if ( (ret = pico_loadResource(picoSystem, picoSgFileName, &picoSgResource)) ) {
        pico_getSystemStatusMessage(picoSystem, ret, outMessage);
        fprintf(stderr, "Cannot load signal generation Lingware resource file (%i): %s\n", ret, outMessage);
        goto unloadSgResource;
    }

    /* Get the text analysis resource name.     */
    // FIXME: free?
    picoTaResourceName = (pico_Char *) malloc( PICO_MAX_RESOURCE_NAME_SIZE );
    if((ret = pico_getResourceName( picoSystem, picoTaResource, (char *) picoTaResourceName ))) {
        pico_getSystemStatusMessage(picoSystem, ret, outMessage);
        fprintf(stderr, "Cannot get the text analysis resource name (%i): %s\n", ret, outMessage);
        goto unloadSgResource;
    }

    /* Get the signal generation resource name. */
    // FIXME: free?
    picoSgResourceName = (pico_Char *) malloc( PICO_MAX_RESOURCE_NAME_SIZE );
    if((ret = pico_getResourceName( picoSystem, picoSgResource, (char *) picoSgResourceName ))) {
        pico_getSystemStatusMessage(picoSystem, ret, outMessage);
        fprintf(stderr, "Cannot get the signal generation resource name (%i): %s\n", ret, outMessage);
        goto unloadSgResource;
    }

    /* Create a voice definition.   */
    if((ret = pico_createVoiceDefinition( picoSystem, (const pico_Char *) picoVoiceName ))) {
        pico_getSystemStatusMessage(picoSystem, ret, outMessage);
        fprintf(stderr, "Cannot create voice definition (%i): %s\n", ret, outMessage);
        goto unloadSgResource;
    }

    /* Add the text analysis resource to the voice. */
    if((ret = pico_addResourceToVoiceDefinition( picoSystem, (const pico_Char *) picoVoiceName, picoTaResourceName ))) {
        pico_getSystemStatusMessage(picoSystem, ret, outMessage);
        fprintf(stderr, "Cannot add the text analysis resource to the voice (%i): %s\n", ret, outMessage);
        goto unloadSgResource;
    }

    /* Add the signal generation resource to the voice. */
    if((ret = pico_addResourceToVoiceDefinition( picoSystem, (const pico_Char *) picoVoiceName, picoSgResourceName ))) {
        pico_getSystemStatusMessage(picoSystem, ret, outMessage);
        fprintf(stderr, "Cannot add the signal generation resource to the voice (%i): %s\n", ret, outMessage);
        goto unloadSgResource;
    }

    /* Create a new Pico engine. */
    if((ret = pico_newEngine( picoSystem, (const pico_Char *) picoVoiceName, &picoEngine ))) { 
        pico_getSystemStatusMessage(picoSystem, ret, outMessage);
        fprintf(stderr, "Cannot create a new pico engine (%i): %s\n", ret, outMessage);
        goto disposeEngine;
    }

    /* success */
    return 0;


    //
    // partial shutdowns below this line
    //  for pico cleanup in case of startup abort
    //
disposeEngine:
    if (picoEngine) {
        pico_disposeEngine( picoSystem, &picoEngine );
        pico_releaseVoiceDefinition( picoSystem, (pico_Char *) picoVoiceName );
        picoEngine = 0;
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

    if (picoSystem) {
        pico_terminate(&picoSystem);
        picoSystem = 0;
    }

    return -1;
}

void Pico::cleanup() 
{
    if ( sdOutFile ) {
        picoos_Common common = (picoos_Common) pico_sysGetCommon(picoSystem);
        picoos_sdfCloseOut(common, &sdOutFile);
        sdOutFile = 0;
    }

    if (picoEngine) {
        pico_disposeEngine( picoSystem, &picoEngine );
        pico_releaseVoiceDefinition( picoSystem, (pico_Char *) picoVoiceName );
        picoEngine = 0;
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

void Pico::sendTextForProcessing( unsigned char * words, long long int word_len )
{
    local_text          = (pico_Char *) words;
    total_text_length   = word_len;
}

int Pico::process()
{
    const int       MAX_OUTBUF_SIZE     = 128;
    const int       PCM_BUFFER_SIZE     = 256;
    pico_Char *     inp                 = 0;
    pico_Int16      bytes_sent, bytes_recv, out_data_type;
    short           outbuf[MAX_OUTBUF_SIZE/2];
    pico_Retstring  outMessage;
    char            pcm_buffer[ PCM_BUFFER_SIZE ];
    int             ret, getstatus;
    picoos_bool     done = TRUE;

    inp = (pico_Char *) local_text;
    unsigned int bufused = 0;
    memset( pcm_buffer, 0, PCM_BUFFER_SIZE );

    // open output WAVE/PCM for writing
    if ( 0 == listener ) {
        picoos_Common common = (picoos_Common) pico_sysGetCommon(picoSystem);
        if ( TRUE != (done=picoos_sdfOpenOut(common, &sdOutFile, (picoos_char *)out_filename, SAMPLE_FREQ_16KHZ, PICOOS_ENC_LIN)) ) {   
            fprintf(stderr, "Cannot open output wave file\n");
            return -1;
        }
    }

    long long int text_length = total_text_length;

    /* synthesis loop   */
    while(1) 
    {
        //−32,768 to 32,767.
        if (text_remaining <= 0) 
        {
            if ( text_length <= 0 )
                break;

            int increment = text_length >= 32767 ? 32767 : text_length;
            text_length -= increment;
            text_remaining = increment;
        }

        /* Feed the text into the engine.   */
        if ( (ret = pico_putTextUtf8(picoEngine, inp, text_remaining, &bytes_sent)) ) {
            pico_getSystemStatusMessage(picoSystem, ret, outMessage);
            fprintf(stderr, "Cannot put Text (%i): %s\n", ret, outMessage);
            return -2;
        }

        text_remaining -= bytes_sent;
        inp += bytes_sent;

        do {

            /* Retrieve the samples */ 
            getstatus = pico_getData( picoEngine, (void *) outbuf, MAX_OUTBUF_SIZE, &bytes_recv, &out_data_type );
            if ( (getstatus !=PICO_STEP_BUSY) && (getstatus !=PICO_STEP_IDLE) ) {
                pico_getSystemStatusMessage(picoSystem, getstatus, outMessage);
                fprintf(stderr, "Cannot get Data (%i): %s\n", getstatus, outMessage);
                return -4;
            }

            /* copy partial encoding and get more bytes */
            if ( bytes_recv > 0 ) 
            {
                if ( (bufused + bytes_recv) <= PCM_BUFFER_SIZE ) {
                    memcpy( pcm_buffer+bufused, (int8_t *)outbuf, bytes_recv );
                    bufused += bytes_recv;
                }

                /* or write the buffer to wavefile, and retrieve any leftover decoding bytes */
                else
                {
                    if ( 0 == listener ) {
                        done = picoos_sdfPutSamples( sdOutFile, bufused / 2, (picoos_int16*) pcm_buffer );
                    } else {
                        listener->writeData( (short*)pcm_buffer, bufused/2 );
                    }

                    bufused = 0;
                    memcpy( pcm_buffer, (int8_t *)outbuf, bytes_recv );
                    bufused += bytes_recv;
                }
            }

        } while (PICO_STEP_BUSY == getstatus);

        /* This chunk of synthesis is finished; pass the remaining samples. */
        if ( 0 == listener ) {
            done = picoos_sdfPutSamples( sdOutFile, bufused / 2, (picoos_int16*) pcm_buffer );
        } else {
            listener->writeData( (short*)pcm_buffer, bufused/2 );
        }
    } 

    // close output wave file, so it can be opened elsewhere
    if ( sdOutFile ) {
        picoos_Common common = (picoos_Common) pico_sysGetCommon(picoSystem);
        picoos_sdfCloseOut(common, &sdOutFile);
        sdOutFile = 0;

        // report
        if ( !strstr( out_filename, "nanotts" ) ) {
            int bytes = fileSize( out_filename );
            fprintf( stderr, "wrote \"%s\" (%d bytes)\n", out_filename, bytes );
        }
    }

    return bufused;
}

int Pico::setVoice( const char * v ) {
    int r = voices.setVoice( v ) ;
    fprintf( stderr, "using lang: %s\n", voices.getVoice() );
    return r;
}

int Pico::fileSize( const char * filename ) {
    FILE * file_p = fopen( filename, "rb" );
    if ( !file_p )
        return -1;
    fseek( file_p, 0L, SEEK_SET );
    int beginning = ftell( file_p );
    fseek( file_p, 0L, SEEK_END );
    int end = ftell( file_p );
    return end - beginning;
}

void Pico::setListener( Listener<short> * listener ) {
    this->listener = listener;
}
//////////////////////////////////////////////////////////////////




int main( int argc, const char ** argv ) 
{
    Nano * nano = new Nano( argc, argv );

    // 
    if ( nano->check_args() < 0 ) {
        delete nano;
        return -1;
    }

    // 
    if ( nano->setup_input_output() < 0 ) {
        delete nano;
        return -2;
    }

    // 
    unsigned char * words   = 0;
    unsigned int    length  = 0;
    if ( nano->getInput( &words, &length ) < 0 ) {
        delete nano;
        return -3;
    }

    //
    Pico *pico = new Pico();
    pico->setPath( nano->getPath() );
    pico->setOutFilename( nano->outFilename() );
    if ( pico->setVoice( nano->getVoice() ) < 0 ) {
        fprintf( stderr, "set voice failed, with: \"%s\n\"", nano->getVoice() );
        goto fast_exit;
    }
    pico->setListener( nano->getListener() );

    //
    pico->setup();

    //
    pico->sendTextForProcessing( words, length ); 

    //
    pico->process();

    // 
    pico->cleanup();

    //
    nano->playOutput();

fast_exit:
    delete pico;
    delete nano;
}
