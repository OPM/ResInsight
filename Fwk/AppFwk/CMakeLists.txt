cmake_minimum_required (VERSION 2.8)

project (CeeApp)


find_package (Qt4 COMPONENTS QtCore QtGui QtMain QtOpenGl REQUIRED)
include (${QT_USE_FILE})

#libraries
add_subdirectory(cafProjectDataModel)
add_subdirectory(cafUserInterface)

#executables
add_subdirectory(cafTests/cafProjectDataModel_UnitTests)
add_subdirectory(cafTests/cafTestApplication)

