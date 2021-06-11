# Check for various compiler extensions

# HAVE_ATTRIBUTE_ALWAYS_INLINE     True if attribute always inline is supported
# HAS_ATTRIBUTE_UNUSED             True if attribute unused is supported
# HAS_ATTRIBUTE_DEPRECATED         True if attribute deprecated is supported
# HAS_ATTRIBUTE_DEPRECATED_MSG     True if attribute deprecated("msg") is supported

include(CheckCXXSourceCompiles)

# __attribute__((always_inline))
CHECK_CXX_SOURCE_COMPILES("
   void __attribute__((always_inline)) foo(void) {}
   int main(void)
   {
     foo();
     return 0;
   };
"  HAVE_ATTRIBUTE_ALWAYS_INLINE
)

# __attribute__((unused))
CHECK_CXX_SOURCE_COMPILES("
   int main(void)
   {
     int __attribute__((unused)) foo;
     return 0;
   };
"  HAS_ATTRIBUTE_UNUSED
)

# __attribute__((deprecated))
CHECK_CXX_SOURCE_COMPILES("
#define DEP __attribute__((deprecated))
   class bar
   {
     bar() DEP;
   };

   class peng { } DEP;

   template <class T>
   class t_bar
   {
     t_bar() DEP;
   };

   template <class T>
   class t_peng {
     t_peng() {};
   } DEP;

   void foo() DEP;

   void foo() {};

   int main(void)
   {
     return 0;
   };
"  HAS_ATTRIBUTE_DEPRECATED
)

# __attribute__((deprecated("msg")))
CHECK_CXX_SOURCE_COMPILES("
#define DEP __attribute__((deprecated(\"message\")))
   class bar {
     bar() DEP;
   };

   class peng { } DEP;

   template <class T>
   class t_bar
   {
     t_bar() DEP;
   };

   template <class T>
   class t_peng
   {
     t_peng() {};
   } DEP;

   void foo() DEP;

   void foo() {};

   int main(void)
   {
     return 0;
   };
"  HAS_ATTRIBUTE_DEPRECATED_MSG
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    list(APPEND ${project}_LIBRARIES stdc++fs)
  endif()
endif()

option(OPM_CLANG_WITH_STDC++FS "Using libstdc++ with clang and we want to link to stdc++fs" OFF)
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND OPM_CLANG_WITH_STDC++FS)
  list(APPEND ${project}_LIBRARIES stdc++fs)
endif()
