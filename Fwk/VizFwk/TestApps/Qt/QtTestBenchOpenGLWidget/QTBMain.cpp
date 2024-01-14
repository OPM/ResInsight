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
#include "cvfLibRender.h"
#include "cvfLibGeometry.h"
#include "cvfLibViewing.h"

#include <QApplication>

#include "QTBMainWindow.h"



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    // Used to get registry settings in the right place
    QCoreApplication::setOrganizationName("CeetronSolutions");
    QCoreApplication::setApplicationName("QtTestBenchOpenGLWidget");

    // Note!
    // The Qt::AA_ShareOpenGLContexts setting is needed when we have multiple viz widgets in flight
    // and we have a setup where these widgets belong to different top-level windows, or end up
    // belonging to different top-level windows through re-parenting.
    // This is indeed the situation we encounter in this test application.
    // For example:
    // * Two viz widgets inside the same central widget => OK
    // * Two viz widgets docked inside two different dock widgets => OK
    // * One viz widget in docked dock widget and one viz widget in central widget => OK
    // * Two viz widgets as different top level windows => FAIL
    // * One viz widget in docked dock widget and one viz widget in floating dock widget => FAIL
    // * One viz widget in central widget and one widget as top level window => FAIL
    //
    // In the failing situations the viz widgets should really belong to different context groups (which
    // has it own implications), but as a workaround we can use the Qt::AA_ShareOpenGLContexts flag
    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    // May want to do some further experimentation with these as well
    //QApplication::setAttribute(Qt::AA_NativeWindows);
    //QApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

    //QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
    //QApplication::setAttribute(Qt::AA_UseSoftwareOpenGL);
    //QApplication::setAttribute(Qt::AA_UseOpenGLES);


    cvf::LogManager* logManager = cvf::LogManager::instance();
    logManager->logger("cee.cvf.OpenGL")->setLevel(cvf::Logger::LL_DEBUG);

    // To display logging from GLWidget::logOpenGLInfo() 
    logManager->logger("cee.cvf.qt")->setLevel(cvf::Logger::LL_INFO);

    // For even more output
    logManager->logger("cee.cvf.qt")->setLevel(cvf::Logger::LL_DEBUG);


    QApplication app(argc, argv);

    QTBMainWindow mainWindow;
    mainWindow.setWindowTitle(QString("QtTestBenchOpenGLWidget (qtVer=%1)").arg(qVersion()));
    mainWindow.resize(1000, 800);
    mainWindow.show();

    const int appRetCode = app.exec();
    return appRetCode;
}

