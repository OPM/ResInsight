#include <fortio.hpp>


void FortIO::close() {
  if (c_ptr)
    fortio_fclose( c_ptr );
  c_ptr = NULL;
}


FortIO FortIO::reader( const char * filename , bool fmt_file , bool endian_flip_header) {
  fortio_type * c_ptr = fortio_open_reader( filename ,  fmt_file , endian_flip_header);
  return FortIO( c_ptr );
}


FortIO FortIO::writer( const std::string & filename , bool fmt_file , bool endian_flip_header ) {
  fortio_type * c_ptr = fortio_open_writer( filename.c_str()  , fmt_file , endian_flip_header);
  return FortIO( c_ptr );
}


FortIO FortIO::readwrite( const char * filename , bool fmt_file , bool endian_flip_header) {
  fortio_type * c_ptr = fortio_open_readwrite( filename , fmt_file  , endian_flip_header);
  return FortIO( c_ptr );
}

