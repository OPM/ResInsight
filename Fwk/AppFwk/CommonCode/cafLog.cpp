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
#include "cvfTrace.h"

#include "cafLog.h"
#include "cafMessagePanel.h"
#include "cafUtils.h"


namespace caf {

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Log::info(const QString& msg)
{
    infoMultiLine(msg, "");
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Log::warning(const QString& msg)
{
    warningMultiLine(msg, "");
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Log::error(const QString& err)
{
    return errorMultiLine(err, "");
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Log::infoMultiLine(const QString& line1, const QString& line2Etc)
{
    MessagePanel* messagePanel = MessagePanel::instance();

    bool generateTrace = true;

    if (messagePanel)
    {
        QString msg = line1;
        if (!line2Etc.isEmpty())
        {
            msg += "\n";
            msg += Utils::indentString(2, line2Etc);
        }

        messagePanel->showInfo(msg);
    }

    if (generateTrace)
    {
        cvf::Trace::show("INF: %s", (const char*)line1.toLatin1());
        if (!line2Etc.isEmpty())
        {
            cvf::Trace::show((const char*)Utils::indentString(5, line2Etc).toLatin1());
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Log::warningMultiLine(const QString& line1, const QString& line2Etc)
{
    MessagePanel* messagePanel = MessagePanel::instance();

    bool generateTrace = true;

    if (messagePanel)
    {
        QString msg = line1;
        if (!line2Etc.isEmpty())
        {
            msg += "\n";
            msg += Utils::indentString(2, line2Etc);
        }

        messagePanel->showWarning(msg);
    }

    if (generateTrace)
    {
        cvf::Trace::show("WRN: %s", (const char*)line1.toLatin1());
        if (!line2Etc.isEmpty())
        {
            cvf::Trace::show((const char*)Utils::indentString(5, line2Etc).toLatin1());
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Log::errorMultiLine(const QString& line1, const QString& line2Etc)
{
    MessagePanel* messagePanel = MessagePanel::instance();

    bool generateTrace = true;

    if (messagePanel)
    {
        QString msg = line1;
        if (!line2Etc.isEmpty())
        {
            msg += "\n";
            msg += Utils::indentString(2, line2Etc);
        }

        messagePanel->showError(msg);
    }

    bool messagePanelVisible = messagePanel ? messagePanel->isVisibleToUser() : false;
    if (!messagePanelVisible)
    {
//         if (mainWindow)
//         {
//             QString capt = QString(PD_APPLICATION_NAME) + " Error";
// 
//             QString msg = line1;
//             if (!line2Etc.isEmpty())
//             {
//                 msg += "\n";
//                 msg += line2Etc;
//             }
// 
//             QMessageBox msgBox(mainWindow);
//             msgBox.setIcon(QMessageBox::Critical);
//             msgBox.setWindowTitle(capt);
//             msgBox.setText(msg);
// 
//             msgBox.exec();
//         }
//         else
//         {
//             generateTrace = true;
//         }
    }

    if (generateTrace)
    {
        cvf::Trace::show("\nERR: %s", (const char*)line1.toLatin1());
        if (!line2Etc.isEmpty())
        {
            cvf::Trace::show((const char*)Utils::indentString(5, line2Etc).toLatin1());
        }
    }

    return false;
}


// //--------------------------------------------------------------------------------------------------
// /// 
// //--------------------------------------------------------------------------------------------------
// void PDLog::pumpMessages()
// {
//     qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
// }
// 
// }


}
