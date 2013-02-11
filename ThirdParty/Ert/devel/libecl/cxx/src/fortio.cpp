#include <string.h>
#include <fortio.hpp>

static FortIO reader( const char * filename , bool endian_flip_header , bool fmt_file) {
  c_ptr = fortio_open_reader( filename , endian_flip_header , fmt_file );
}


static FortIO writer( const std::string & filename , bool endian_flip_header , bool fmt_file) {
  c_ptr = fortio_open_reader( filename.c_str() , endian_flip_header , fmt_file );
}
