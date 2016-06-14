#
# Module that checks for supported C++11 (former C++0x) features.
#
# Sets the follwing variable:
#
# HAVE_FINAL                       True if the compiler supports the "final" quantifier
# HAVE_TYPE_TRAITS                 True if the <type_traits> header is available and implements sufficient functionality
# HAVE_SHARED_PTR                  True if std::shared_ptr is available
# HAVE_UNIQUE_PTR                  True if std::unique_ptr is available
# HAVE_NULLPTR                     True if nullptr is available
# HAVE_REGEX                       True if std::regex available and sufficiently usable
# HAVE_ARRAY                       True if header <array> and fill() are available
# HAVE_ATTRIBUTE_ALWAYS_INLINE     True if attribute always inline is supported
# HAS_ATTRIBUTE_UNUSED             True if attribute unused is supported
# HAS_ATTRIBUTE_DEPRECATED         True if attribute deprecated is supported
# HAS_ATTRIBUTE_DEPRECATED_MSG     True if attribute deprecated("msg") is supported
# HAVE_CONSTEXPR                   True if constexpr attribute is available
# HAVE_INTEGRAL_CONSTANT           True if compiler supports integral_constant
# HAVE_STATIC_ASSERT               True if static_assert is available
# HAVE_AUTO                        True if the compiler supports the auto keyword
# HAVE_VARIADIC_TEMPLATES          True if variadic templates are supported
# HAVE_VARIADIC_CONSTRUCTOR_SFINAE True if variadic constructor sfinae is supported
# HAVE_RVALUE_REFERENCES           True if rvalue references are supported
# HAVE_TUPLE                       True if std::tuple is available
# HAVE_TR1_TUPLE                   True if std::tr1::tuple is available

include(CheckCXXSourceCompiles)
include(CheckCXXSourceRuns)

# test for C++11 flags
include(TestCXXAcceptsFlag)
include(CheckIncludeFileCXX)

# macro to only add option once
include(AddOptions)

if(NOT MSVC)
  # try to use compiler flag -std=c++11
  CHECK_CXX_ACCEPTS_FLAG("-std=c++11" CXX_FLAG_CXX11)
  if(CXX_FLAG_CXX11)
    add_options (CXX ALL_BUILDS "-std=c++11")
    set(CXX_STD0X_FLAGS "-std=c++11")
  else()
    # try to use compiler flag -std=c++0x for older compilers
    CHECK_CXX_ACCEPTS_FLAG("-std=c++0x" CXX_FLAG_CXX0X)
    if(CXX_FLAG_CXX0X)
      add_options (CXX ALL_BUILDS "-std=c++0x")
      set(CXX_STD0X_FLAGS "-std=c++0x")
    endif(CXX_FLAG_CXX0X)
  endif(CXX_FLAG_CXX11)
endif(NOT MSVC)

# if we are building with an Apple toolchain in MacOS X,
# we cannot use the old GCC 4.2 fork, but must use the
# new runtime library
set (CXX_STDLIB_FLAGS)
string (TOUPPER "${CMAKE_CXX_COMPILER_ID}" _comp_id)
if (APPLE AND (_comp_id MATCHES "CLANG"))
  CHECK_CXX_ACCEPTS_FLAG ("-stdlib=libc++" CXX_FLAG_STDLIB_LIBCXX)
  if (CXX_FLAG_STDLIB_LIBCXX)
	add_options (CXX ALL_BUILDS "-stdlib=libc++")
	set (CXX_STDLIB_FLAGS "-stdlib=libc++")
  endif (CXX_FLAG_STDLIB_LIBCXX)
endif (APPLE AND (_comp_id MATCHES "CLANG"))

# to format the command-line options pretty, we have an optional space
if (CXX_STD0X_FLAGS AND CXX_STDLIB_FLAGS)
  set (CXX_SPACE " ")
else (CXX_STD0X_FLAGS AND CXX_STDLIB_FLAGS)
  set (CXX_SPACE)
endif (CXX_STD0X_FLAGS AND CXX_STDLIB_FLAGS)

# perform tests
include(CheckCXXSourceCompiles)

# the "final" method specifier
CHECK_CXX_SOURCE_COMPILES("
struct Base {
  virtual void foo() = 0;
};
struct Derived : public Base {
  virtual void foo() final {};
};

int main()
{
    return 0;
}
"  HAVE_FINAL
)

# std::is_convertible, std::is_base_of
CHECK_CXX_SOURCE_COMPILES("
#include <type_traits>

class Base {};
class Derived : public Base {};

int main()
{
    bool foo = std::is_convertible<int, double>::value;
    bool bar = std::is_base_of<Base, Derived>::value;
    bool foobar = std::is_integral<double>::value;
    return 0;
}
"  HAVE_TYPE_TRAITS
)

# nullptr
CHECK_CXX_SOURCE_COMPILES("
    #include <memory>

    int main(void)
    {
      std::shared_ptr<int> foo(new int(123));
      return 0;
    }
"  HAVE_SHARED_PTR
)

# this is required by dune-common to avoid linker errors. "fun"!
if (HAVE_SHARED_PTR)
  set(HAVE_MAKE_SHARED 1)
  set(SHARED_PTR_HEADER "<memory>")
  set(SHARED_PTR_NAMESPACE "std")
endif()

# nullptr
CHECK_CXX_SOURCE_COMPILES("
    #include <memory>

    int main(void)
    {
      std::unique_ptr<int> foo(new int(123));
      return 0;
    }
"  HAVE_UNIQUE_PTR
)

# nullptr
CHECK_CXX_SOURCE_COMPILES("
    int main(void)
    {
      char* ch = nullptr;
      return 0;
    }
"  HAVE_NULLPTR
)

# <regex>
CHECK_CXX_SOURCE_RUNS("
    #include <regex>
    int main(void)
    {
      std::regex r(\"AB.*|BC+|DE.+\", std::regex::extended);
      if (!std::regex_match(\"AB\", r))
           return 1;
      if (!std::regex_match(\"ABC\", r))
           return 2;
      if (!std::regex_match(\"ABC!#\", r))
           return 3;
      if (std::regex_match(\"B\", r))
           return 4;
      if (!std::regex_match(\"BC\", r))
           return 5;
      if (std::regex_match(\"BCE\", r))
           return 6;
      if (std::regex_match(\"DE\", r))
           return 7;
      if (!std::regex_match(\"DEF\", r))
           return 8;
      return 0;
    }
"  HAVE_REGEX
)

# constexpr
CHECK_CXX_SOURCE_COMPILES("
    template <class T>
    inline constexpr int foo(T bar) { return bar*2; }
    int main(void)
    {
      constexpr int foobar = foo(100);
      return 0;
    }
"  HAVE_CONSTEXPR
)

# array and fill
CHECK_CXX_SOURCE_COMPILES("
    #include <array>
    
    int main(void)
    {
      std::array<int,2> a;
      a.fill(9);
      return 0;
    }
" HAVE_ARRAY
)

# Check whether if std::integral_constant< T, v > is supported and casts into T
CHECK_CXX_SOURCE_COMPILES("
    #include <type_traits>
    void f( int ){}

    int main(void){
      f( std::integral_constant< int, 42 >() );
    }
" HAVE_INTEGRAL_CONSTANT
)

# Check whether if <tuple> is available
check_include_file_cxx("tuple" HAVE_TUPLE)

# Check whether if <tr1/tuple> is available
check_include_file_cxx("tr1/tuple" HAVE_TR1_TUPLE)

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

# static assert
CHECK_CXX_SOURCE_COMPILES("
   int main(void)
   {
     static_assert(true,\"MSG\");
     return 0;
   }
"  HAVE_STATIC_ASSERT
)

# auto keyword
CHECK_CXX_SOURCE_COMPILES("
   int main(void)
   {
     auto foo = 1.23;
     return 0;
   }
"  HAVE_AUTO
)

# variadic template support
CHECK_CXX_SOURCE_COMPILES("
   #include <cassert>

   template<typename... T>
   int addints(T... x);

   int add_ints()
   {
     return 0;
   }

   template<typename T1, typename... T>
   int add_ints(T1 t1, T... t)
   {
     return t1 + add_ints(t...);
   }

   int main(void)
   {
     assert( 5 == add_ints(9,3,-5,-2) );
     return 0;
   }
" HAVE_VARIADIC_TEMPLATES
)

# SFINAE on variadic template constructors within template classes
CHECK_CXX_SOURCE_COMPILES("
  #include <functional>

  template<typename... U>
  struct A
  {
    template<typename... T,
             typename = typename std::enable_if<(sizeof...(T) < 2)>::type
            >
    A(T... t)
    : i(1)
    {}

    template<typename... T,
             typename = typename std::enable_if<(sizeof...(T) >= 2)>::type,
             typename = void
            >
    A(T... t)
    : i(-1)
    {}

    A()
    : i(1)
    {}

    int i;
  };

  int main(void)
  {
    return (A<int>().i + A<int>(2).i + A<int>(\"foo\",3.4).i + A<int>(8,'a',A<int>()).i == 0 ? 0 : 1);
  }
" HAVE_VARIADIC_CONSTRUCTOR_SFINAE
)

# rvalue references
CHECK_CXX_SOURCE_COMPILES("
  #include <cassert>
  #include <utility>
  int foo(int&& x) { return 1; }
  int foo(const int& x) { return -1; }

  template<typename T>
  int forward(T&& x)
  {
    return foo(std::forward<T>(x));
  }

  int main(void)
  {
    int i = 0;
    assert( forward(i) + forward(int(2)) == 0);
    return 0;
  }
" HAVE_RVALUE_REFERENCES
)
include(CheckIncludeFile)
include(CheckIncludeFileCXX)
# Search for some tr1 headers
foreach(_HEADER tuple tr1/tuple type_traits tr1/type_traits)
  string(REPLACE "/" "_" _HEADER_VAR ${_HEADER})
  string(TOUPPER ${_HEADER_VAR} _HEADER_VAR )
  check_include_file_cxx(${_HEADER} "HAVE_${_HEADER_VAR}")
endforeach(_HEADER tuple tr1/tuple tr1/type_traits)

# make sure that the C++-11 features implemented by the compiler are a
# superset of those provided by GCC 4.4. This makes the test fail on
# all GCC compilers before 4.4.
set(CXX_FEATURES_MISSING "")
if (NOT HAVE_TYPE_TRAITS)
  set(CXX_FEATURES_MISSING
      "${CXX_FEATURES_MISSING} - Sufficiently conformant type traits (defined by the 'type_traits' header file)\n")
endif()
if (NOT HAVE_SHARED_PTR)
  set(CXX_FEATURES_MISSING
      "${CXX_FEATURES_MISSING} - Shared pointers (the std::shared_ptr class)\n")
endif()
if (NOT HAVE_UNIQUE_PTR)
  set(CXX_FEATURES_MISSING
      "${CXX_FEATURES_MISSING} - Unique pointers (the std::unique_ptr class)\n")
endif()
if (NOT HAVE_ARRAY)
  set(CXX_FEATURES_MISSING
      "${CXX_FEATURES_MISSING} - Statically sized arrays (the std::array class)\n")
endif()
if (NOT HAVE_STATIC_ASSERT)
  set(CXX_FEATURES_MISSING
      "${CXX_FEATURES_MISSING} - Static assertations (the static_assert() mechanism)\n")
endif()
if (NOT HAVE_AUTO)
  set(CXX_FEATURES_MISSING
      "${CXX_FEATURES_MISSING} - Automatically typed variables (the 'auto' keyword)\n")
endif()
if (NOT HAVE_VARIADIC_TEMPLATES)
  set(CXX_FEATURES_MISSING
      "${CXX_FEATURES_MISSING} - Variable number of template arguments\n")
endif()
if (NOT HAVE_VARIADIC_CONSTRUCTOR_SFINAE)
  set(CXX_FEATURES_MISSING
      "${CXX_FEATURES_MISSING} - Constructors with variable number of template arguments obeying the SFINAE (specialization failure is not an error) rule\n")
endif()
if (NOT HAVE_RVALUE_REFERENCES)
  set(CXX_FEATURES_MISSING
      "${CXX_FEATURES_MISSING} - References to rvalue objects\n")
endif()
if (NOT HAVE_TUPLE)
  set(CXX_FEATURES_MISSING
      "${CXX_FEATURES_MISSING} - Tuples (the std::tuple class)\n")
endif()

if(CXX_FEATURES_MISSING)
  set (CXX11FEATURES_FOUND FALSE)
  if (CXX11Features_FIND_REQUIRED)
	message(FATAL_ERROR
      "Your C++ compiler does not support the minimum set of C++-2011 features required. "
      "Make sure to use a compiler which implements all C++-2011 features provided by GCC 4.4. "
      "Your compiler does not seem to implement the following features:\n"
      "${CXX_FEATURES_MISSING}")
  endif()
else ()
  set (CXX11FEATURES_FOUND TRUE)
endif()
