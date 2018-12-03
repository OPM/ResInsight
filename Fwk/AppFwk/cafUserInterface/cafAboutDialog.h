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


#pragma once

#include <QDialog>

class QGridLayout;

namespace caf {


//==================================================================================================
//
// 
//
//==================================================================================================
class AboutDialog : public QDialog
{
public:
    explicit AboutDialog(QWidget* parent);

    void    setApplicationName(const QString& appName);
    void    setApplicationVersion(const QString& ver);
    void    setCopyright(const QString& copyright);

    void    showQtVersion(bool show);
    void    addVersionEntry(const QString& verLabel, const QString& verText);
    void    setIsDebugBuild(bool isDebugBuild);

    void    create();

    static QString  versionStringForcurrentOpenGLContext();

private:
    void    addStringPairToVerInfoLayout(const QString& labelStr, const QString& infoStr, QGridLayout* verInfoLayout, int insertRow);

private:
    bool            m_isCreated;            // Indicates if the create() function has been called

    QString         m_appName;                // Application name, appears in bold at the top of the dialog. 
    QString         m_appVersion;            // Application version info. Can be empty
    QString            m_appCopyright;            // Application copyright string. Can be empty

    bool            m_showQtVersion;        // Flags whether Qt version info should be shown
    QStringList     m_verLabels;            // Labels for user specified version entries
    QStringList     m_verTexts;                // The actual version text for user specified version entries

    bool            m_isDebugBuild;            // If set to true, will show info in dlg to indicate that this is a debug build
};

}
