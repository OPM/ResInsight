cmake_minimum_required(VERSION 2.8)

project(freetype)


add_definitions(-D_LIB)
add_definitions(-DFT2_BUILD_LIBRARY)

if (MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)

    # Disable some warnings that appear in 64bit compile
    set(CMAKE_C_FLAGS "/wd4244 /wd4267 ${CMAKE_C_FLAGS}")
endif()


include_directories(include)


set(CEE_SOURCE_FILES
src/autofit/autofit.c
src/base/ftbase.c
src/base/ftbbox.c
src/base/ftbitmap.c
src/base/ftfstype.c
src/base/ftgasp.c
src/base/ftgxval.c
src/base/ftglyph.c
src/base/ftinit.c
src/base/ftlcdfil.c
src/base/ftmm.c
src/base/ftotval.c
src/base/ftpatent.c
src/base/ftpfr.c
src/base/ftstroke.c
src/base/ftsynth.c
src/base/ftsystem.c
src/base/fttype1.c
src/base/ftwinfnt.c
src/base/ftxf86.c
src/bdf/bdf.c
src/cache/ftcache.c
src/cff/cff.c
src/cid/type1cid.c
src/gzip/ftgzip.c
src/lzw/ftlzw.c
src/pcf/pcf.c
src/pfr/pfr.c
src/psaux/psaux.c
src/pshinter/pshinter.c
src/psnames/psmodule.c
src/raster/raster.c
src/sfnt/sfnt.c
src/smooth/smooth.c
src/truetype/truetype.c
src/type1/type1.c
src/type42/type42.c
src/winfonts/winfnt.c
)

if (MSVC)
  set(CEE_SOURCE_FILES ${CEE_SOURCE_FILES} builds/win32/ftdebug.c)
endif()


add_library(${PROJECT_NAME} ${CEE_SOURCE_FILES})

