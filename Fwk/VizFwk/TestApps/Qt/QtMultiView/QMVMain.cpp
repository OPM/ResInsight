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

#include "QMVMainWindow.h"

#include <QApplication>

#include <clocale>



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    // Enables resource sharing between QOpenGLWidget instances that belong to different top-level windows
    // Must be enabled if we want to create multiple OpenGL widgets in the same context group and they're not sitting as direct children of the same widget.
    // Enable here so we can experiment with creating widgets as floating dialogs
    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    // Can be used as a work-around if OpenGL scene appears as black in combination with remote desktop for Win7 clients
    // See: https://bugreports.qt.io/browse/QTBUG-47975
    //QSurfaceFormat surface;
    //surface.setAlphaBufferSize(8);
    //QSurfaceFormat::setDefaultFormat(surface);


    QApplication app(argc, argv);

    cvf::LogManager* logManager = cvf::LogManager::instance();
    logManager->logger("cee.cvf.OpenGL")->setLevel(4);

    // On Linux, Qt will use the system locale, force number formatting settings back to "C" locale
    setlocale(LC_NUMERIC,"C");

    QMVMainWindow window;
    QString platform = cvf::System::is64Bit() ? "(64bit)" : "(32bit)";
    window.setWindowTitle("Qt MultiView " + platform);
    window.resize(1000, 800);
    window.show();

    return app.exec();
}
