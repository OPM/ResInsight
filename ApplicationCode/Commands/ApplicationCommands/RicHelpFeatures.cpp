/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RicHelpFeatures.h"

#include "RiaBaseDefs.h"
#include "RiaApplication.h"

#include "RiuMainWindow.h"

#include "cafAboutDialog.h"
#include "cafViewer.h"

#include <QAction>
#include <QDesktopServices>
#include <QErrorMessage>
#include <QUrl>

CAF_CMD_SOURCE_INIT(RicHelpAboutFeature,            "RicHelpAboutFeature");
CAF_CMD_SOURCE_INIT(RicHelpCommandLineFeature,      "RicHelpCommandLineFeature");
CAF_CMD_SOURCE_INIT(RicHelpOpenUsersGuideFeature,   "RicHelpOpenUsersGuideFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicHelpAboutFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicHelpAboutFeature::onActionTriggered(bool isChecked)
{
    this->disableModelChangeContribution();

    caf::AboutDialog dlg(nullptr);

    dlg.setApplicationName(RI_APPLICATION_NAME);
    dlg.setApplicationVersion(RiaApplication::getVersionStringApp(true));
    dlg.setCopyright("Copyright Statoil ASA, Ceetron Solutions AS, Ceetron AS");
    dlg.showQtVersion(false);
#ifdef _DEBUG
    dlg.setIsDebugBuild(true);
#endif

    dlg.addVersionEntry(" ", "ResInsight is made available under the GNU General Public License v. 3");
    dlg.addVersionEntry(" ", "See http://www.gnu.org/licenses/gpl.html");
    dlg.addVersionEntry(" ", " ");

    QStringList activeFeatures;
#ifdef USE_ODB_API
    activeFeatures += "  Geomech";
#endif
#ifdef USE_HDF5
    activeFeatures += "  Souring";
#endif

    if (!activeFeatures.isEmpty())
    {
        dlg.addVersionEntry(" ", "Features");

        for (auto feature : activeFeatures)
        {
            dlg.addVersionEntry(" ", feature);
        }
    }

    dlg.addVersionEntry(" ", " ");
    dlg.addVersionEntry(" ", "Technical Information");
    dlg.addVersionEntry(" ", QString("   Qt ") + qVersion());
    dlg.addVersionEntry(" ", QString("   ") + caf::AboutDialog::versionStringForcurrentOpenGLContext());
    dlg.addVersionEntry(" ", caf::Viewer::isShadersSupported() ? "   Hardware OpenGL" : "   Software OpenGL");

    if (RiaApplication::enableDevelopmentFeatures())
    {
        QString vendor("Unknown");
        QString render("Unknown");

        {
            char* str = (char*)glGetString(GL_VENDOR);

            if (str)
            {
                vendor = str;
            }
        }

        {
            char* str = (char*)glGetString(GL_RENDERER);

            if (str)
            {
                render = str;
            }
        }
        
        dlg.addVersionEntry(" ", QString("   ") + vendor + " : " + render);
    }

    dlg.create();
    dlg.resize(300, 200);

    dlg.exec();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicHelpAboutFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("&About");
}







//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicHelpCommandLineFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicHelpCommandLineFeature::onActionTriggered(bool isChecked)
{
    this->disableModelChangeContribution();

    RiaApplication* app = RiaApplication::instance();
    QString text = app->commandLineParameterHelp();
    app->showFormattedTextInMessageBox(text);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicHelpCommandLineFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("&Command Line Help");
}






//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicHelpOpenUsersGuideFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicHelpOpenUsersGuideFeature::onActionTriggered(bool isChecked)
{
    this->disableModelChangeContribution();

    QString usersGuideUrl = "http://resinsight.org/docs/home";

    if (!QDesktopServices::openUrl(usersGuideUrl))
    {
        QErrorMessage* errorHandler = QErrorMessage::qtHandler();
        errorHandler->showMessage("Failed open browser with the following url\n\n" + usersGuideUrl);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicHelpOpenUsersGuideFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("&Users Guide");
}


