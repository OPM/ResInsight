/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'sched_macros.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __SCHED_MACROS_H___
#define __SCHED_MACROS_H___



/***********************************
Macros for "voidifications" of the data handlers.

  - *_IMPL     - The implementation
  - *_HEADER   - Header
  - None.      - Name.

***********************************/

#define KW_FREE(KW)    sched_kw_## KW ##_free__
#define KW_FPRINTF(KW) sched_kw_## KW ##_fprintf__
#define KW_COPYC(KW)   sched_kw_## KW ##_copyc__
#define KW_ALLOC(KW)   sched_kw_## KW ##_alloc__

#define GET_DATA_HANDLERS(DH, KWNAME) \
DH.token_alloc  = KW_ALLOC(       KWNAME) ; \
DH.free         = KW_FREE(        KWNAME) ; \
DH.fprintf      = KW_FPRINTF(     KWNAME) ; \
DH.copyc        = NULL/*KW_ALLOC_COPY(KWNAME)*/   ;

/*******************************************************************/

#define KW_ALLOC_IMPL(KW) \
void * sched_kw_## KW ##_alloc__(const stringlist_type * tokens , int * token_index ) \
{                                                                                     \
  return (void *) sched_kw_## KW ##_alloc(tokens , token_index);                      \
}


#define KW_FPRINTF_IMPL(KW)                                                \
void   sched_kw_## KW ##_fprintf__(const void * kw, FILE * stream)         \
{                                                                          \
  sched_kw_## KW ##_fprintf((const sched_kw_## KW ##_type *) kw, stream);  \
}                                                                          \

#define KW_FREE_IMPL(KW)                                \
void   sched_kw_## KW ##_free__(void * kw)              \
{                                                       \
  sched_kw_## KW ##_free((sched_kw_## KW ##_type *) kw);\
}                                                       \


#define KW_COPYC_IMPL(KW)                             \
void * sched_kw_## KW ##_copyc__(const void * kw)  {  \
   return sched_kw_ ## KW ## _copyc(kw);              \
}

#define KW_IMPL(KW)      \
KW_FREE_IMPL(KW)         \
KW_FPRINTF_IMPL(KW)      \
KW_ALLOC_IMPL(KW)        \
KW_COPYC_IMPL(KW)



/*******************************************************************/

#define KW_ALLOC_HEADER(KW)                                                            \
void * sched_kw_## KW ##_alloc__(const stringlist_type * tokens , int * token_index) ; \

#define KW_COPYC_HEADER(KW)                                           \
void * sched_kw_## KW ##_copyc__(const void *);                  \

#define KW_FPRINTF_HEADER(KW)                              \
void   sched_kw_## KW ##_fprintf__(const void *, FILE * ); \

#define KW_FREE_HEADER(KW)               \
void   sched_kw_## KW ##_free__(void *); \

#endif


#define KW_HEADER(KW)      \
KW_FREE_HEADER(KW)         \
KW_FPRINTF_HEADER(KW)      \
KW_ALLOC_HEADER(KW)        \
KW_COPYC_HEADER(KW)
