#include <stdlib.h>

void  dgemv_(char * , int * , int * , double * , double * , int * , const double * , int * , double * , double * , int * );


/*
  This test is only to check if seperate linking with BLAS is
  necessary; on the RedHat linux computers only explicit linking with
  lapack is sufficient, whereas in other cases also blas must be
  linked in specifically. 
*/


int main(int argc , char ** argv) {
  /* Wildly invalid input - but the signature is satisfies so it should compile. */
  dgemv_( "A" , NULL , NULL , NULL , NULL , NULL , NULL , NULL , NULL , NULL , NULL);
}
