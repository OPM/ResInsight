/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'util_endian.c' is part of ERT - Ensemble based Reservoir Tool. 
    
   ERT is free software: you can redistribute it and/or modify 
   it under the terms of the GNU General Public License as published by 
   the Free Software Foundation, either version 3 of the License, or 
   (at your option) any later version. 
    
   ERT is distributed in the hope that it will be useful, but WITHOUT ANY 
   WARRANTY; without even the implied warranty of MERCHANTABILITY or 
   FITNESS FOR A PARTICULAR PURPOSE.   
    
   See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
   for more details. 
*/
#include <stdint.h>
#include <stdio.h>


#if UINTPTR_MAX == 0xFFFFFFFF
#define ARCH32
#elif UINTPTR_MAX == 0xFFFFFFFFFFFFFFFF
#define ARCH64
#else
#error "Could not determine if this is a 32 bit or 64 bit computer?"
#endif


/*
   Macros for endian flipping. The macros create a new endian-flipped
   value, and should be used as: 

     flipped_value = FLIP32( value )
 
   The macros are not exported and only available through the function
   util_endian_flip_vector().  
*/


#define FLIP16(var) (((var >> 8) & 0x00ff) | ((var << 8) & 0xff00))

#define FLIP32(var) (( (var >> 24) & 0x000000ff) | \
                      ((var >>  8) & 0x0000ff00) | \
                      ((var <<  8) & 0x00ff0000) | \
                      ((var << 24) & 0xff000000))

#define FLIP64(var)  (((var >> 56) & 0x00000000000000ff) | \
                      ((var >> 40) & 0x000000000000ff00) | \
                      ((var >> 24) & 0x0000000000ff0000) | \
                      ((var >>  8) & 0x00000000ff000000) | \
                      ((var <<  8) & 0x000000ff00000000) | \
                      ((var << 24) & 0x0000ff0000000000) | \
                      ((var << 40) & 0x00ff000000000000) | \
                      ((var << 56) & 0xff00000000000000))




static uint16_t util_endian_convert16( uint16_t u ) {
  return (( u >> 8U ) & 0xFFU) | (( u & 0xFFU) >> 8U);
}


static uint32_t util_endian_convert32( uint32_t u ) {
  const uint32_t m8  = (uint32_t) 0x00FF00FFUL;
  const uint32_t m16 = (uint32_t) 0x0000FFFFUL;
  
  u = (( u >> 8U ) & m8)   | ((u & m8) << 8U);
  u = (( u >> 16U ) & m16) | ((u & m16) << 16U);
  return u;
}


static uint64_t util_endian_convert64( uint64_t u ) {
  const uint64_t m8  = (uint64_t) 0x00FF00FF00FF00FFULL;
  const uint64_t m16 = (uint64_t) 0x0000FFFF0000FFFFULL;
  const uint64_t m32 = (uint64_t) 0x00000000FFFFFFFFULL;

  
  u = (( u >> 8U ) & m8)   | ((u & m8) << 8U);
  u = (( u >> 16U ) & m16) | ((u & m16) << 16U);
  u = (( u >> 32U ) & m32) | ((u & m32) << 32U);
  return u;
}


static uint64_t util_endian_convert32_64( uint64_t u ) {
  const uint64_t m8  = (uint64_t) 0x00FF00FF00FF00FFULL;
  const uint64_t m16 = (uint64_t) 0x0000FFFF0000FFFFULL;

  
  u = (( u >> 8U ) & m8)   | ((u & m8) << 8U);
  u = (( u >> 16U ) & m16) | ((u & m16) << 16U);
  return u;
}



void util_endian_flip_vector(void *data, int element_size , int elements) {
  int i;
  switch (element_size) {
  case(1):
    break;
  case(2): 
    {
      uint16_t *tmp16 = (uint16_t *) data;

      for (i = 0; i <elements; i++)
        tmp16[i] = util_endian_convert16(tmp16[i]);
      break;
    }
  case(4):
    {
#ifdef ARCH64
      /*
        In the case of a 64 bit CPU the fastest way to swap 32 bit
        variables will be by swapping two elements in one operation;
        this is provided by the util_endian_convert32_64() function. In the case
        of binary ECLIPSE files this case is quite common, and
        therefore worth supporting as a special case.
      */
      uint64_t *tmp64 = (uint64_t *) data;

      for (i = 0; i <elements/2; i++)
        tmp64[i] = util_endian_convert32_64(tmp64[i]);

      if ( elements & 1 ) {
        // Odd number of elements - flip the last element as an ordinary 32 bit swap.
        uint32_t *tmp32 = (uint32_t *) data;
        tmp32[ elements - 1] = util_endian_convert32( tmp32[elements - 1] );
      }
      break;
#else
      uint32_t *tmp32 = (uint32_t *) data;
      
      for (i = 0; i <elements; i++)
        tmp32[i] = util_endian_convert32(tmp32[i]);
      
      break;
#endif
    }
  case(8):
    {
      uint64_t *tmp64 = (uint64_t *) data;

      for (i = 0; i <elements; i++)
        tmp64[i] = util_endian_convert64(tmp64[i]);
      break;
    }
  default:
    fprintf(stderr,"%s: current element size: %d \n",__func__ , element_size);
    util_abort("%s: can only endian flip 1/2/4/8 byte variables - aborting \n",__func__);
  }
}

