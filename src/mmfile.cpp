
#include "mmfile.h"


mmfile_t::mmfile_t( const char * _filename ) 
{
    open( _filename );
}

mmfile_t::~mmfile_t()
{
    close();
}

void mmfile_t::open( const char * _filename ) 
{
#ifdef _WIN /* Windows */
	LPSTR lpfilename = (LPSTR) _filename;
	hFile = CreateFile(lpfilename, GENERIC_READ, FILE_SHARE_READ, NULL,
		       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if ( hFile == INVALID_HANDLE_VALUE ) {
		fprintf( stderr, "mmfile: couldn't open file: \"%s\" for reading\n", _filename );
	}
	hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (hFileMapping == 0) {
		CloseHandle( hFile );
		fprintf( stderr, "mmfile: couldn't map file: \"%s\" \n", _filename );
	}
	
	lpFileBase = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);
    if (lpFileBase == 0) {
		CloseHandle( hFileMapping );
		CloseHandle( hFile );
		fprintf( stderr, "mmfile: couldn't map file: \"%s\"\n", _filename );
	}

	// filesize
	size = GetFileSize( hFile, NULL );

	// save 
	data = (unsigned char *) lpFileBase;

#else /* Unix */
    filename = realpath( _filename, NULL );

	// open
	if ( !(fp = fopen( filename, "rb" )) ) {
		fprintf( stderr, "mmfile: couldn't open input: \"%s\" for reading\n", filename );
	}

	// fileno
	this->fileno = ::fileno( fp );

	// filesize
	struct stat st;
	if ( fstat( this->fileno, &st ) == -1 || st.st_size == 0 ) {
		fprintf( stderr, "mmfile: couldn't stat input file\n" );
	}
	size = st.st_size;

	// mmap the file
	data = (unsigned char *) mmap( 0, st.st_size, PROT_READ, MAP_SHARED, this->fileno, 0 );
#endif /* Windows or Unix mmap methods */
}

void mmfile_t::close()
{
#ifdef _WIN
    UnmapViewOfFile(lpFileBase);
    CloseHandle(hFileMapping);
    CloseHandle(hFile);

#else /* Unix|Mac */
    if ( data )
        if ( -1 == munmap( data, size ) )
            fprintf( stderr, "mmfile: failed to unmap file: %s\n", filename );
    if ( fp )
        fclose( fp );
    if ( filename )
        free( filename );
#endif
}

