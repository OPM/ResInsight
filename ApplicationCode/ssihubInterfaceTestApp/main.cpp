/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QApplication>
#include <QDir>

#include "ssihubInterface.h"
#include "TestTools.h"
#include "RiuWellImportWizard.h"
#include "RimWellPathImport.h"





int main(int argc, char *argv[])
{
    QApplication app(argc, argv);


    QString destinationFolder("c:/tmp/resinsight_ws");
    QString wsAddress("http://127.0.0.1:5000");
    
    ssihub::Interface wsInterface;
    wsInterface.setWebServiceAddress(wsAddress);
    wsInterface.setJsonDestinationFolder(destinationFolder);

    int north = 7500000;
    int south = 7000000;
    int east =   401000;
    int west =   400000;
    wsInterface.setRegion(north, south, east, west);

    //QStringList jsonWellPathFileNames = wsInterface.jsonWellPaths();

//     void setWebServiceAddress(const QString wsAdress);
//     void setJsonDestinationFolder(const QString folder);
//     void setRegion(int east, int west, int north, int south);
// 
//     QStringList jsonWellPaths();

    RimWellPathImport* wellPathImportObject = TestTools::createMockObject();
    wellPathImportObject->north = north;
    wellPathImportObject->south = south;
    wellPathImportObject->east = east;
    wellPathImportObject->west = west;

    RiuWellImportWizard wizard(wsAddress, destinationFolder, wellPathImportObject);

    wizard.show();

    return app.exec();
}
