#include <iostream>

#include <ecl_kw.h>
#include <ecl_kw.hpp>


EclKW EclKW::wrap( ecl_kw_type * c_ptr , bool owner) {
  return EclKW( c_ptr , owner );
}


EclKW EclKW::wrap_data(const char * name ,  int size , ecl_type_enum type, void * data) {
  ecl_kw_type * c_ptr = ecl_kw_alloc_new_shared( name , size , type , data );
  return EclKW( c_ptr , true );
}


EclKW EclKW::create( const char * name , int size , ecl_type_enum type , void * data) {
  ecl_kw_type * c_ptr = ecl_kw_alloc( name , size , type );
  if (data != NULL)
    ecl_kw_set_memcpy_data( c_ptr , data );
  return EclKW( c_ptr , true );
}

/*EclKW::EclKW( const std::string& name , size_t size , ecl_type_enum type , void * data) {
  c_ptr = ecl_kw_alloc( name.c_str() , (int) size , type );
  if (data != NULL)
    ecl_kw_set_memcpy_data( c_ptr , data );
}
*/

EclKW::~EclKW() {
  std::cout << "Calling destructor: " << ecl_kw_get_header( c_ptr ) << "\n";
  if (owner) 
    ecl_kw_free( c_ptr );
}

/*EclKW::EclKW( EclKW & src ) {
  owner = src.owner;
  src.owner = false;
  
  c_ptr = src.c_ptr;
}
*/

/*EclKW::EclKW( EclKW src ) {
  owner = sr.>owner;
  src.owner = false;
  
  c_ptr = src.c_ptr;
}
*/

void EclKW::fwrite(FortIO& fortio) {
  ecl_kw_fwrite( c_ptr , fortio.C_PTR());
}


