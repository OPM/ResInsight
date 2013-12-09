#ifndef __FORTIO_HPP__
#define __FORTIO_HPP__
#include <string>

#include <fortio.h>

#ifndef BYTE_ORDER
#define BYTE_ORDER  __LITTLE_ENDIAN
#endif
#include <ecl_endian_flip.h>

class FortIO {

private:
  fortio_type * c_ptr;
  
  FortIO(fortio_type * c_ptr) {
    this->c_ptr = c_ptr;
  }
  
    
public:
  ~FortIO() {  close();  }
  
  fortio_type  * C_PTR( ) { return c_ptr; } 

  static FortIO writer(const std::string & filename    , bool fmt_file = false , bool endian_flip_header = ECL_ENDIAN_FLIP );
  static FortIO reader(const char * filename     , bool fmt_file = false , bool endian_flip_header = ECL_ENDIAN_FLIP );
  static FortIO readwrite( const char * filename , bool fmt_file = false , bool endian_flip_header = ECL_ENDIAN_FLIP );
  void close();
};
  
#endif
