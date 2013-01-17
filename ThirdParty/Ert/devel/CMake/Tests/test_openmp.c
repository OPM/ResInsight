#include <stdlib.h>
#include <omp.h>

int main(int argc, char ** argv) {
  int sum = 0;
#pragma omp parallel for
  for (int i=0; i < 100; i++)
    sum += i;
  
}

