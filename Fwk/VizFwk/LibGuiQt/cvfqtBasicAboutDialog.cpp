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


#include "cvfBase.h"
#include "cvfSystem.h"

#include "cvfqtBasicAboutDialog.h"

#include <QtCore/QVariant>
#include <QtCore/QString>

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
namespace cvfqt {



//==================================================================================================
///
/// \class cvfqt::BasicAboutDialog
/// \ingroup GuiQt
///
/// 
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
BasicAboutDialog::BasicAboutDialog(QWidget* parent)
    : QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
    m_isCreated = false;

    //m_appName;
    //m_appVersion;
    //m_appCopyright;

    m_showLibraryVersion = true;
    m_showQtVersion = true;

    m_isDebugBuild = false;
}



//--------------------------------------------------------------------------------------------------
/// Set application name to show in the dialog. Must be specified if any other app info is to be displayed
//--------------------------------------------------------------------------------------------------
void BasicAboutDialog::setApplicationName(const QString& appName)	
{ 
    CVF_ASSERT(!m_isCreated); 
    m_appName = appName; 
}			


//--------------------------------------------------------------------------------------------------
/// Set application version info to display
//--------------------------------------------------------------------------------------------------
void BasicAboutDialog::setApplicationVersion(const QString& ver)
{
    CVF_ASSERT(!m_isCreated); 
    m_appVersion = ver; 
}			


//--------------------------------------------------------------------------------------------------
/// Set copyright info to display
//--------------------------------------------------------------------------------------------------
void BasicAboutDialog::setCopyright(const QString& copyright)
{ 
    CVF_ASSERT(!m_isCreated); 
    m_appCopyright = copyright; 
}		


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void BasicAboutDialog::showLibraryVersion(bool show)
{ 
    CVF_ASSERT(!m_isCreated); 
    m_showLibraryVersion = show; 
}		


//--------------------------------------------------------------------------------------------------
/// Enable display of Qt version
//--------------------------------------------------------------------------------------------------
void BasicAboutDialog::showQtVersion(bool show)
{ 
    CVF_ASSERT(!m_isCreated); 
    m_showQtVersion = show; 
}		


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void BasicAboutDialog::addVersionEntry(const QString& verLabel, const QString& verText)
{
    CVF_ASSERT(!m_isCreated); 

    m_verLabels.push_back(verLabel);
    m_verTexts.push_back(verText);

    CVF_ASSERT(m_verLabels.size() == m_verTexts.size());
}


//--------------------------------------------------------------------------------------------------
/// Set to true to show text in dialog to indicate that we're running a debug build of our app
//--------------------------------------------------------------------------------------------------
void BasicAboutDialog::setIsDebugBuild(bool isDebugBuild)
{ 
    CVF_ASSERT(!m_isCreated); 
    m_isDebugBuild = isDebugBuild; 
}		


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void BasicAboutDialog::create()
{
    // Only allowed to call once
    CVF_ASSERT(!m_isCreated);

    // Only show app info if app name is non-empty
    bool showAppInfo = !m_appName.isEmpty();

    // Do an initial resize, dialog will resize itself later based on the widgets we have added
    resize(10, 10); 


    // Set caption, different text depending on whether we're showing app info or not
    QString dlgCaption = "Version Information Details";
    if (showAppInfo)
    {
        dlgCaption = "About " + m_appName;
        if (m_isDebugBuild) dlgCaption += " (DEBUG)";
    }

    setWindowTitle(dlgCaption);


    // Create the dialog's main layout
    QVBoxLayout* dlgMainLayout = new QVBoxLayout(this); 

    // The the top layout
    QVBoxLayout* topLayout = new QVBoxLayout; 
    topLayout->setSpacing(3);


    // Possibly create and set text for widgets with app info
    if (showAppInfo)
    {
        QVBoxLayout* appInfoLayout = new QVBoxLayout; 
        appInfoLayout->setSpacing(3);

        // Always do app name
        CVF_ASSERT(!m_appName.isEmpty());
        QLabel* appNameLabel = new QLabel(this);

        QFont appNameFont(appNameLabel->font());
        appNameFont.setPointSize(14);
        appNameFont.setBold(true);
        appNameLabel->setFont(appNameFont); 
        appNameLabel->setText(m_appName);
        appInfoLayout->addWidget(appNameLabel);

        // Application version if specified
        if (!m_appVersion.isEmpty())
        {
            QString appVer = m_appVersion;
			appVer += cvf::System::is64Bit() ? "  (64-bit)" : "  (32-bit)";

            QLabel* appVersionLabel = new QLabel(this);
            QFont appVersionFont(appVersionLabel->font());
            appVersionFont.setPointSize(8);
            appVersionFont.setBold(true);
            appVersionLabel->setFont(appVersionFont); 
            appVersionLabel->setText(appVer);
            appInfoLayout->addWidget(appVersionLabel);
        }

        // Application copyright if specified
        if (!m_appCopyright.isEmpty())
        {
            QLabel* appCopyrightLabel = new QLabel(this);
            appCopyrightLabel->setTextFormat(Qt::RichText);
            QFont appCopyrightFont(appCopyrightLabel->font());
            appCopyrightFont.setPointSize(8);
            appCopyrightFont.setBold(true);
            appCopyrightLabel->setFont(appCopyrightFont); 
            appCopyrightLabel->setText(m_appCopyright);
            appInfoLayout->addWidget(appCopyrightLabel);
        }

        QFrame* line = new QFrame(this);
        line->setProperty("frameShape", (int)QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        line->setFrameShape(QFrame::HLine);
        appInfoLayout->addWidget(line);

        topLayout->addLayout(appInfoLayout);
    }


    // Possibly show extend version info
    if (m_showLibraryVersion  ||
        m_showQtVersion	      ||
        m_verLabels.size() > 0)
    {
        QGridLayout* verInfoLayout = new QGridLayout; 
        verInfoLayout->setSpacing(0);

        int insertRow = 0;

        // Library version
        if (m_showLibraryVersion)
        {
            QString ver;
            ver.sprintf("%s.%s%s-%s", CVF_MAJOR_VERSION, CVF_MINOR_VERSION, CVF_SPECIAL_BUILD, CVF_BUILD_NUMBER);

            addStringPairToVerInfoLayout("Library ver.:  ", ver, verInfoLayout, insertRow++);
        }

        // Qt version
        if (m_showQtVersion)
        {
            addStringPairToVerInfoLayout("Qt ver.:  ", qVersion(), verInfoLayout, insertRow++);
        }

        // Custom specified labels
        if (m_verLabels.size() > 0)
        {
            CVF_ASSERT(m_verLabels.size() == m_verTexts.size());

            int i;
            for (i = 0; i < m_verLabels.size(); i++)
            {
                addStringPairToVerInfoLayout(m_verLabels[i], m_verTexts[i], verInfoLayout, insertRow++);
            }
        }

        topLayout->addLayout(verInfoLayout);
    }


    dlgMainLayout->addLayout(topLayout);

    QSpacerItem* spacer1 = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    dlgMainLayout->addItem(spacer1);


    // The bottom part with the OK button and
    // possibly text label indicating that we're running a debug build
    QHBoxLayout* bottomLayout = new QHBoxLayout;

    // Indicate that this is a debug build
    if (m_isDebugBuild)
    {
        QLabel* debugLabel = new QLabel(this);
        debugLabel->setText("<font color='brown'><b>This is a DEBUG build...</b></font>");
        bottomLayout->addWidget(debugLabel);
    }


    // Add OK button
    QSpacerItem* spacer2 = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
    bottomLayout->addItem(spacer2);

    QPushButton* buttonOk = new QPushButton("&OK", this);
    buttonOk->setAutoDefault(true);
    buttonOk->setDefault(true);
    buttonOk->setFocus();
    connect(buttonOk, SIGNAL(clicked()), this, SLOT(accept()) );
    bottomLayout->addWidget(buttonOk);

    dlgMainLayout->addLayout(bottomLayout);

    m_isCreated = true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void BasicAboutDialog::addStringPairToVerInfoLayout(const QString& labelStr, const QString& infoStr, QGridLayout* verInfoLayout, int insertRow)
{
    QLabel* label = new QLabel(this);
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    label->setText(labelStr);
    verInfoLayout->addWidget(label, insertRow, 0);

    QLabel* info = new QLabel(this);
    info->setText(infoStr);
    verInfoLayout->addWidget(info, insertRow, 1 );

}


} // namespace cvfqt


