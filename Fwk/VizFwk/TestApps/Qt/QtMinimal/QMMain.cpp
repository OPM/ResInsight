//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################

#include "cvfLibCore.h"

#include "QMMainWindow.h"

#include <QApplication>

#include <locale.h>


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    cvf::LogManager* logManager = cvf::LogManager::instance();
    logManager->logger("cee.cvf.OpenGL")->setLevel(cvf::Logger::LL_DEBUG);

    QApplication app(argc, argv);

    // On Linux, Qt will use the system locale, force number formatting settings back to "C" locale
    setlocale(LC_NUMERIC,"C");

    QMMainWindow window;
    QString platform = cvf::System::is64Bit() ? "(64bit)" : "(32bit)";
    window.setWindowTitle("Qt Minimal " + platform);
    window.resize(1000, 800);;
    window.show();

    return app.exec();
}
