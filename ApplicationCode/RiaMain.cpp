/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
// 
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
// 
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RiaStdInclude.h"
#include "RiaApplication.h"
#include "RiuMainWindow.h"


// Cmake is able to control subsystem on Windows using the following method http://www.cmake.org/Wiki/VSConfigSpecificSettings
//
// if(WIN32)
//     set_target_properties(WindowApplicationExample PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
//     set_target_properties(WindowApplicationExample PROPERTIES COMPILE_DEFINITIONS_DEBUG "_CONSOLE")
//     set_target_properties(WindowApplicationExample PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
//     set_target_properties(WindowApplicationExample PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO "_CONSOLE")
//     set_target_properties(WindowApplicationExample PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
//     set_target_properties(WindowApplicationExample PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS")
// endif(WIN32)
//
//
// Due to a bug in Cmake, use workaround described here http://public.kitware.com/Bug/view.php?id=12566
#if defined(_MSC_VER) && defined(_WIN32)

    #ifdef _DEBUG
        #pragma comment(linker, "/SUBSYSTEM:CONSOLE")
    #else
        #pragma comment(linker, "/SUBSYSTEM:WINDOWS")
    #endif // _DEBUG

#endif // defined(_MSC_VER) && defined(_WIN32)




int main(int argc, char *argv[])
{
    RiaApplication app(argc, argv);

    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

    RiuMainWindow window;
    QString platform = cvf::System::is64Bit() ? "(64bit)" : "(32bit)";
    window.setWindowTitle("ResInsight " + platform);
    window.resize(1000, 800);
    window.show();

    if (app.parseArguments())
    {
        return app.exec();
    }

    return 0;
}

