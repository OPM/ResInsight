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
#include "QSRCommandLineArgs.h"



//==================================================================================================
//
// QSRCommandLineArgs
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QSRCommandLineArgs::QSRCommandLineArgs()
:   m_echoArguments(false),
    m_showHelpText(false)
{

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool QSRCommandLineArgs::parseCommandLine(const QStringList& argList)
{
    bool parseError = false;

    QStringListIterator it(argList);

    // Skip first one which is app name
    if (!it.hasNext()) return true;
    it.next();

    while (it.hasNext())
    {
        QString arg = it.next();
        arg = arg.trimmed();
        if (arg.isEmpty())
        {
            continue;
        }

        bool assumeOption = arg.startsWith("-");

        if (assumeOption)
        {
            CVF_ASSERT(arg.length() > 1);
            QString opt = arg.mid(1, -1);


            // Do simple switches/options first
            // ------------------------------------------------
            if (opt.compare("echo", Qt::CaseInsensitive) == 0)
            {
                m_echoArguments = true;
                continue;
            }
            else if (opt.compare("help", Qt::CaseInsensitive) == 0)
            {
                m_showHelpText = true;
                continue;
            }


            // All of the following options require a parameter
            // ------------------------------------------------

            // Hence, if we're out of arguments or the next string is empty we will get an error
            QString par = it.next();
            par = par.trimmed();
            if (par.isEmpty())
            {
                parseError = true;
                break;
            }

            if (opt.compare("testDataDir", Qt::CaseInsensitive) == 0)
            {
                QString dir = par;
                dir.replace("\\", "/");
                if (!dir.isEmpty() && !dir.endsWith("/"))
                {
                    dir.append("/");
                }

                m_testDataDir = dir;
                continue;
            }

            if (opt.compare("shaderDir", Qt::CaseInsensitive) == 0)
            {
                QString dir = par;
                dir.replace("\\", "/");
                if (!dir.isEmpty() && !dir.endsWith("/"))
                {
                    dir.append("/");
                }

                m_shaderDir = dir;
                continue;
            }
        }

//         // For now the only non option if project file name
//         if (sStartupProjectFile.IsBlank())
//         {
//             sStartupProjectFile = sArg;
//         }

    }

    // cvf::Trace::show("m_testDataDir:   %s", m_testDataDir.toAscii().data());
    // cvf::Trace::show("m_shaderDir:     %s", m_shaderDir.toAscii().data());
    // cvf::Trace::show("m_echoArguments: %d", m_echoArguments);
    // cvf::Trace::show("m_showHelpText:  %d", m_showHelpText);

    if (parseError)
    {
        return false;
    }
    else
    {
        return true;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString QSRCommandLineArgs::getHelptext()
{
    QString txt;
    txt += "Valid command line arguments:\n";
    txt += "  -testDataDir <dir> \n";
    txt += "  -shaderDir <dir> \n";
    txt += "  -echo \n";
    txt += "  -help \n";

    return txt;
}

