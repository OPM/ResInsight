cmake_minimum_required (VERSION 2.8)

project ( QtPropertyBrowser )


if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Wno-overloaded-virtual")  # do not display warning related to hiding of overloaded virtual method
endif()


find_package (Qt4 COMPONENTS QtCore QtGui QtMain REQUIRED)
include (${QT_USE_FILE})
include( ${QtPropertyBrowser_SOURCE_DIR}/qtadditions_cmake.txt )

# Qt MOC
# ==============================================================================
file( GLOB_RECURSE HEADER_FILES *.h )
set ( QT_MOC_HEADERS ${HEADER_FILES} )

qt4_wrap_cpp( MOC_FILES_CPP ${QT_MOC_HEADERS} )

# Special handling of cpp files with internal Qt classes that must be MOCed. 
# The file base name is given, and .moc is appended
set( EXTRA_MOC_FILES qteditorfactory qtpropertymanager qttreepropertybrowser )
foreach (it ${EXTRA_MOC_FILES})
    qt4_generate_moc(${it}.cpp ${it}.moc)
    set( MOC_FILES_CPP ${MOC_FILES_CPP} ${it}.moc )
endforeach(it)


INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

set_source_files_properties(${MOC_FILES_CPP} PROPERTIES HEADER_FILE_ONLY TRUE)

add_library ( ${PROJECT_NAME}
    qtbuttonpropertybrowser.cpp
    qteditorfactory.cpp
    qtgroupboxpropertybrowser.cpp
    qtpropertybrowser.cpp
    qtpropertybrowserutils.cpp
    qtpropertymanager.cpp
    qttreepropertybrowser.cpp
    qtvariantproperty.cpp

    ${MOC_FILES_CPP}
)
