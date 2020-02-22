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


#include "QSRStdInclude.h"
#include "QSRMainWindow.h"
#include "QSRCommandLineArgs.h"

#include "SnippetFactoryBasis.h"

#include "cvfShaderSourceProvider.h"
#include "cvfqtUtils.h"

#include <QApplication>
#include <QMessageBox>


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // On Linux, Qt will use the system locale, force number formatting settings back to "C" locale
    setlocale(LC_NUMERIC,"C");

    // These directories are correct when running from within visual studio
    cvf::String testDataDir = "../../../Tests/TestData/";
    cvf::String shaderDir   = "../../../Tests/SnippetsBasis/Shaders/";

    {
        QSRCommandLineArgs cmdLineArgs;
        cmdLineArgs.parseCommandLine(app.arguments());
        if (cmdLineArgs.m_showHelpText)
        {
            QMessageBox::information(NULL, "Qt Snippet Runner", cmdLineArgs.getHelptext());
            return EXIT_SUCCESS;
        }

        if (!cmdLineArgs.m_testDataDir.isEmpty()) testDataDir = cvfqt::Utils::toString(cmdLineArgs.m_testDataDir);
        if (!cmdLineArgs.m_shaderDir.isEmpty())   shaderDir   = cvfqt::Utils::toString(cmdLineArgs.m_shaderDir);

        if (cmdLineArgs.m_echoArguments)
        {
            String txt;
            txt += "Active parameters:";
            txt += "\n  testDataDir: " + testDataDir;
            txt += "\n  shaderDir: " + shaderDir;
            QMessageBox::information(NULL, "Qt Snippet Runner", cvfqt::Utils::toQString(txt));
        }
    }

    cvfu::SnippetFactory* factoryBasis = new SnippetFactoryBasis;
    factoryBasis->setTestDataDir(testDataDir);
    cvfu::SnippetRegistry::instance()->addFactory(factoryBasis);

    cvf::ShaderSourceProvider::instance()->addFileSearchDirectory(shaderDir);

    // Comment in this line to be able to read the glsl directly from file
    //cvf::ShaderSourceProvider::instance()->setSourceRepository(new cvf::ShaderSourceRepositoryFile("../../../LibRender/glsl/"));


    QSRMainWindow window;
    window.resize(1000, 800);;
    window.show();

    return app.exec();
}
