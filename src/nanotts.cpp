
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

extern "C" {
#include "svoxpico/picoapi.h"
#include "svoxpico/picoapid.h"
#include "svoxpico/picoos.h"
}




/*
================================================
Nano
================================================
*/
class Nano 
{
private:
    enum inputMode_t {
        IN_STDIN,
        IN_ARG,
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
    

public:
    Nano( const int, const char ** );

    void PrintUsage();
    void check_args();
    void setup_input_output() ;

    int getInput( char ** data, unsigned int * bytes );
    void sendOutput( char * data, unsigned int size );
};

Nano::Nano( const int i, const char ** v ) : my_argc(i), my_argv(v) {
    strcpy( voice, "en-GB" );
    voicedir = 0;
    out_filename = 0;
    in_filename = 0;
    words = 0;
    in_fp = 0;
    out_fp = 0;
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
    out_mode = OUT_PLAYBACK;


    for ( int i = 0; i < my_argc; i++ )
    {
        if ( strcmp( my_argv[i], "-h" ) == 0 || strcmp( my_argv[i], "--help" ) == 0 ) {
            PrintUsage();
            return -1;
        }

        // INPUTS
        else if ( strcmp( my_argv[i], "-w" ) == 0 ) {
            in_mode = IN_ARG;
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
    case IN_ARG:
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
int Nano::getInput( char ** data, unsigned int * bytes ) {
}

// 
void Nano::sendOutput( char * data, unsigned int size ) {
}



/*
================================================
Pico
================================================
*/
class Pico {
private:
    pico_System     picoSystem;
    pico_Resource   picoTaResource;
    pico_Resource   picoSgResource;
    pico_Resource   picoUtppResource;
    pico_Engine     picoEngine;

public:
    Pico() ;

    int setup() ;
    void cleanup() ;
    void run() ;
};


Pico::Pico() {
    picoSystem          = 0;
    picoTaResource      = 0;
    picoSgResource      = 0;
    picoUtppResource    = 0;
    picoEngine          = 0;
}

void Pico::setup() 
{
    /* supported voices
       Pico does not seperately specify the voice and locale.   */
    const char * picoSupportedLangIso3[]        = { "eng",              "eng",              "deu",              "spa",              "fra",              "ita" };
    const char * picoSupportedCountryIso3[]     = { "USA",              "GBR",              "DEU",              "ESP",              "FRA",              "ITA" };
    const char * picoSupportedLang[]            = { "en-US",            "en-GB",            "de-DE",            "es-ES",            "fr-FR",            "it-IT" };
    const char * picoInternalLang[]             = { "en-US",            "en-GB",            "de-DE",            "es-ES",            "fr-FR",            "it-IT" };
    const char * picoInternalTaLingware[]       = { "en-US_ta.bin",     "en-GB_ta.bin",     "de-DE_ta.bin",     "es-ES_ta.bin",     "fr-FR_ta.bin",     "it-IT_ta.bin" };
    const char * picoInternalSgLingware[]       = { "en-US_lh0_sg.bin", "en-GB_kh0_sg.bin", "de-DE_gl0_sg.bin", "es-ES_zl0_sg.bin", "fr-FR_nk0_sg.bin", "it-IT_cm0_sg.bin" };
    const char * picoInternalUtppLingware[]     = { "en-US_utpp.bin",   "en-GB_utpp.bin",   "de-DE_utpp.bin",   "es-ES_utpp.bin",   "fr-FR_utpp.bin",   "it-IT_utpp.bin" };
    const int picoNumSupportedVocs              = 6;

    /* adapation layer global variables */
    void *          picoMemArea         = 0;

    pico_Char *     picoTaFileName      = 0;
    pico_Char *     picoSgFileName      = 0;
    pico_Char *     picoUtppFileName    = 0;
    pico_Char *     picoTaResourceName  = 0;
    pico_Char *     picoSgResourceName  = 0;
    pico_Char *     picoUtppResourceName = 0;
    const int       PICO_MEM_SIZE = 8000;
    int             ret, getstatus;
    pico_Retstring  outMessage;



    //
    picoMemArea = malloc( PICO_MEM_SIZE );

    if ( (ret = pico_initialize( picoMemArea, PICO_MEM_SIZE, &picoSystem )) ) {
        pico_getSystemStatusMessage(picoSystem, ret, outMessage);
        fprintf(stderr, "Cannot initialize pico (%i): %s\n", ret, outMessage);
        goto terminate;
    }

    /* Load the text analysis Lingware resource file.   */
    picoTaFileName      = (pico_Char *) malloc( PICO_MAX_DATAPATH_NAME_SIZE + PICO_MAX_FILE_NAME_SIZE );
    strcpy((char *) picoTaFileName,   PICO_LINGWARE_PATH);
    strcat((char *) picoTaFileName,   (const char *) picoInternalTaLingware[langIndex]);
    if((ret = pico_loadResource( picoSystem, picoTaFileName, &picoTaResource ))) {
        pico_getSystemStatusMessage(picoSystem, ret, outMessage);
        fprintf(stderr, "Cannot load text analysis resource file (%i): %s\n", ret, outMessage);
        goto unloadTaResource;
    }

    /* Load the signal generation Lingware resource file.   */
    picoSgFileName      = (pico_Char *) malloc( PICO_MAX_DATAPATH_NAME_SIZE + PICO_MAX_FILE_NAME_SIZE );
    strcpy((char *) picoSgFileName,   PICO_LINGWARE_PATH);
    strcat((char *) picoSgFileName,   (const char *) picoInternalSgLingware[langIndex]);
    if((ret = pico_loadResource( picoSystem, picoSgFileName, &picoSgResource ))) {
        pico_getSystemStatusMessage(picoSystem, ret, outMessage);
        fprintf(stderr, "Cannot load signal generation Lingware resource file (%i): %s\n", ret, outMessage);
        goto unloadSgResource;
    }

    /* Get the text analysis resource name.     */
    picoTaResourceName  = (pico_Char *) malloc( PICO_MAX_RESOURCE_NAME_SIZE );
    if((ret = pico_getResourceName( picoSystem, picoTaResource, (char *) picoTaResourceName ))) {
        pico_getSystemStatusMessage(picoSystem, ret, outMessage);
        fprintf(stderr, "Cannot get the text analysis resource name (%i): %s\n", ret, outMessage);
        goto unloadUtppResource;
    }

    /* Get the signal generation resource name. */
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
    picoos_sdfCloseOut(common, &sdOutFile);

disposeEngine:
    if (picoEngine) {
        pico_disposeEngine( picoSystem, &picoEngine );
        pico_releaseVoiceDefinition( picoSystem, (pico_Char *) PICO_VOICE_NAME );
        picoEngine = NULL;
    }
unloadUtppResource:
    if (picoUtppResource) {
        pico_unloadResource( picoSystem, &picoUtppResource );
        picoUtppResource = NULL;
    }
unloadSgResource:
    if (picoSgResource) {
        pico_unloadResource( picoSystem, &picoSgResource );
        picoSgResource = NULL;
    }
unloadTaResource:
    if (picoTaResource) {  
        pico_unloadResource( picoSystem, &picoTaResource );
        picoTaResource = NULL;
    }
terminate:
    if (picoSystem) {
        pico_terminate(&picoSystem);
        picoSystem = NULL;
    }

    return -1;
}

void Pico::cleanup() {

    // close output wave file
    picoos_sdfCloseOut(common, &sdOutFile);

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

void Pico::run( const char * words, unsigned int byte_len, void * buffer ) 
{
    pico_Char *     local_text = NULL;
    pico_Int16      bytes_sent, bytes_recv, text_remaining, out_data_type;
    pico_Char *     inp = NULL;
    short           outbuf[MAX_OUTBUF_SIZE/2];
    pico_Retstring  outMessage;
    int             picoSynthAbort      = 0;


    local_text = (pico_Char *) words ;
    text_remaining = strlen((const char *) local_text) + 1;

    inp = (pico_Char *) local_text;

    size_t bufused = 0;

    picoos_Common common = (picoos_Common) pico_sysGetCommon(picoSystem);

    picoos_SDFile sdOutFile = NULL;

    picoos_bool done = TRUE;
    if(TRUE != (done = picoos_sdfOpenOut(common, &sdOutFile,
        (picoos_char *) wavefile, SAMPLE_FREQ_16KHZ, PICOOS_ENC_LIN)))
    {   
        fprintf(stderr, "Cannot open output wave file\n");
        return -1;
    }

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

            /* Retrieve the samples and add them to the buffer. */
            getstatus = pico_getData( picoEngine, (void *) outbuf,
                      MAX_OUTBUF_SIZE, &bytes_recv, &out_data_type );

            if((getstatus !=PICO_STEP_BUSY) && (getstatus !=PICO_STEP_IDLE)){
                pico_getSystemStatusMessage(picoSystem, getstatus, outMessage);
                fprintf(stderr, "Cannot get Data (%i): %s\n", getstatus, outMessage);
                return -4;
            }

            if (bytes_recv) {
                if ((bufused + bytes_recv) <= bufferSize) {
                    memcpy(buffer+bufused, (int8_t *) outbuf, bytes_recv);
                    bufused += bytes_recv;
                } else {
                    done = picoos_sdfPutSamples(
                                        sdOutFile,
                                        bufused / 2,
                                        (picoos_int16*) (buffer));
                    bufused = 0;
                    memcpy(buffer, (int8_t *) outbuf, bytes_recv);
                    bufused += bytes_recv;
                }
            }

        } while (PICO_STEP_BUSY == getstatus);


        /* This chunk of synthesis is finished; pass the remaining samples. */
        if (!picoSynthAbort) {
                    done = picoos_sdfPutSamples(
                                        sdOutFile,
                                        bufused / 2,
                                        (picoos_int16*) (buffer));
        }
        picoSynthAbort = 0;
    }

    return 0;
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

    while(1)
    {
        char * words = 0;
        int length = 0;
        char pcm_buffer[ 8000 ];

        nano->getInput( &words, &length );
        if ( 0 == length )
            break;

        pico->run( words, length, pcm_buffer );

        nano->sendOutput( pcm_buffer );
    }

    //
    pico->cleanup();
    delete pico;
    delete nano;
}
