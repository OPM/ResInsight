#ifndef ERT_TYPE_MACROS_H
#define ERT_TYPE_MACROS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ert/util/util.h>


/*****************************************************************/
/**

   The four macros UTIL_IS_INSTANCE_FUNCTION, UTIL_SAFE_CAST_FUNTION,
   UTIL_TYPE_ID_DECLARATION and UTIL_TYPE_ID_INIT can be used to
   implement a simple system for type checking (void *) at
   runtime. The system is based on a unique integer for each class,
   this must be provided by the user.

   The motivation for these functions is to be able to to type-check
   the arguments to callback functions like pthread_create.

    UTIL_TYPE_ID_DECLARATION: Adds a field "int __type_id;" to the
      struct defintion.

    UTIL_TYPE_ID_INIT: Should be added to the allocation routine,
      inserts a "->__type_id = magic_int;" code line in the alloc
      routine.

    UTIL_IS_INSTANCE_FUNCTION: This macro will generate a function
      <type>_is_instance(void *) which will cast the (void *) input to
      (type *), and check the value of __type_id. If this is the
      correct value true is returned, otherwise the function will
      return false. Observe that the function will accept NULL as
      input; in which case it will return false.

    UTIL_SAFE_CAST_FUNCTION: This is similar to
      UTIL_IS_INSTANCE_FUNCTION, but it will return (type *) if the
      cast succeeds, and fail hard if it fails. There is also a _CONST
      variety of this function.

*/




#define UTIL_IS_INSTANCE_FUNCTION(type , TYPE_ID)                 \
bool type ## _is_instance( const void * __arg ) {                 \
   if (__arg == NULL)                                             \
      return false;                                               \
   else {                                                         \
     const type ## _type * arg =  (const type ## _type * ) __arg; \
      if ( arg->__type_id == TYPE_ID)                             \
         return true;                                             \
      else                                                        \
         return false;                                            \
   }                                                              \
}


#define UTIL_IS_INSTANCE_HEADER(type) bool type ## _is_instance( const void * __arg )


#define UTIL_SAFE_CAST_FUNCTION(type , TYPE_ID)                                          \
type ## _type * type ## _safe_cast( void * __arg ) {                                     \
   if (__arg == NULL) {                                                                  \
      util_abort("%s: runtime cast failed - tried to dereference NULL\n",__func__);      \
      return NULL;                                                                       \
   }                                                                                     \
   {                                                                                     \
      type ## _type * arg = (type ## _type *) __arg;                                     \
      if ( arg->__type_id == TYPE_ID)                                                       \
         return arg;                                                                        \
      else {                                                                                \
         util_abort("%s: runtime cast failed: Got ID:%d  Expected ID:%d \n", __func__ , arg->__type_id , TYPE_ID); \
         return NULL;                                                                       \
      }                                                                                   \
   }                                                                                     \
}
#define UTIL_SAFE_CAST_HEADER( type ) type ## _type * type ## _safe_cast( void * __arg )


#define UTIL_SAFE_CAST_FUNCTION_CONST(type , TYPE_ID)                                    \
const type ## _type * type ## _safe_cast_const( const void * __arg ) {                   \
   if (__arg == NULL) {                                                                  \
      util_abort("%s: runtime cast failed - tried to dereference NULL\n",__func__);      \
      return NULL;                                                                       \
   }                                                                                     \
   {                                                                                     \
   const type ## _type * arg = (const type ## _type *) __arg;                            \
   if ( arg->__type_id == TYPE_ID)                                                       \
      return arg;                                                                        \
   else {                                                                                \
      util_abort("%s: runtime cast failed: Got ID:%d  Expected ID:%d \n", __func__ , arg->__type_id , TYPE_ID); \
      return NULL;                                                                       \
   }                                                                                     \
   }                                                                                     \
}
#define UTIL_SAFE_CAST_HEADER_CONST( type ) const type ## _type * type ## _safe_cast_const( const void * __arg )




#define UTIL_TRY_CAST_FUNCTION(type , TYPE_ID)                                           \
type ## _type * type ## _try_cast( void * __arg ) {                                      \
   if (__arg == NULL)                                                                    \
      return NULL;                                                                       \
   {                                                                                     \
      type ## _type * arg = (type ## _type *) __arg;                                     \
      if ( arg->__type_id == TYPE_ID)                                                    \
         return arg;                                                                     \
      else                                                                               \
         return NULL;                                                                    \
   }                                                                                     \
}
#define UTIL_TRY_CAST_HEADER( type ) type ## _type * type ## _try_cast( void * __arg )


#define UTIL_TRY_CAST_FUNCTION_CONST(type , TYPE_ID)                                     \
const type ## _type * type ## _try_cast_const( const void * __arg ) {                          \
   if (__arg == NULL)                                                                    \
      return NULL;                                                                       \
   {                                                                                     \
      const type ## _type * arg = (type ## _type *) __arg;                               \
      if ( arg->__type_id == TYPE_ID)                                                    \
         return arg;                                                                     \
      else                                                                               \
         return NULL;                                                                    \
   }                                                                                     \
}
#define UTIL_TRY_CAST_HEADER_CONST( type ) const type ## _type * type ## _try_cast_const( const void * __arg )




#define UTIL_TYPE_ID_DECLARATION           int   __type_id
#define UTIL_TYPE_ID_INIT(var , TYPE_ID)   var->__type_id = TYPE_ID;



#ifdef __cplusplus
}
#endif
#endif
