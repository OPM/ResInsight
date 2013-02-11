#include <stdarg.h>
#include <stdlib.h>

int func(int arg1 , ...) {
  va_list ap;
  va_list copy;
  va_start(ap , arg1);
  va_copy( copy , ap );
  
  va_end(ap);
  return 1;
}

int main(int argc, char ** argv) {
  func(10 , NULL , "String" );
}
