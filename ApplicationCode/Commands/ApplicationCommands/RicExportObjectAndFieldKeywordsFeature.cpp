/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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

#include "RicExportObjectAndFieldKeywordsFeature.h"

#include "RiaVersionInfo.h"

#include "RiuFileDialogTools.h"
#include "RiuMainWindow.h"

#include "cafClassTypeName.h"
#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmXmlFieldHandle.h"

#include <QAction>
#include <QApplication>
#include <QDateTime>
#include <QFile>
#include <QTextStream>

CAF_CMD_SOURCE_INIT( RicExportObjectAndFieldKeywordsFeature, "RicExportObjectAndFieldKeywordsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportObjectAndFieldKeywordsFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportObjectAndFieldKeywordsFeature::onActionTriggered( bool isChecked )
{
    QString dir = RiuFileDialogTools::getExistingDirectory( RiuMainWindow::instance(),
                                                            tr( "Select Directory For Export" ),
                                                            "c:/temp" );

    if ( !dir.isEmpty() )
    {
        exportObjectKeywords( dir + "/ri-objectKeywords.txt" );
        exportObjectAndFieldKeywords( dir + "/ri-fieldKeywords.txt" );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportObjectAndFieldKeywordsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Export Object and Field Keywords" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportObjectAndFieldKeywordsFeature::exportObjectKeywords( const QString& filePath )
{
    auto instance = caf::PdmDefaultObjectFactory::instance();

    QString     textString;
    QTextStream stream( &textString );

    textString = versionHeaderText();

    std::vector<QString> classKeywords = instance->classKeywords();
    for ( const auto& keyword : classKeywords )
    {
        stream << keyword << "\n";
    }

    writeTextToFile( filePath, textString );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportObjectAndFieldKeywordsFeature::exportObjectAndFieldKeywords( const QString& filePath )
{
    auto instance = caf::PdmDefaultObjectFactory::instance();

    QString     textString;
    QTextStream stream( &textString );

    bool includeClassName = true;

    textString = versionHeaderText();

    std::vector<QString> classKeywords = instance->classKeywords();
    for ( const auto& keyword : classKeywords )
    {
        caf::PdmObjectHandle* myClass = instance->create( keyword );

        stream << keyword;

        if ( includeClassName )
        {
            QString className = qStringTypeName( *myClass );

            stream << " - " << className;
        }

        stream << "\n";

        std::vector<caf::PdmFieldHandle*> fields;
        myClass->fields( fields );

        for ( auto f : fields )
        {
            if ( !f->xmlCapability()->isIOReadable() ) continue;

            stream << "  " << f->keyword() << "\n";

            for ( auto alias : f->keywordAliases() )
            {
                stream << "    (A)" << alias << "\n";
            }
        }

        stream << "\n";

        delete myClass;
    }

    writeTextToFile( filePath, textString );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportObjectAndFieldKeywordsFeature::writeTextToFile( const QString& filePath, const QString& text )
{
    QFile exportFile( filePath );
    if ( exportFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
        QTextStream stream( &exportFile );

        stream << text;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicExportObjectAndFieldKeywordsFeature::versionHeaderText()
{
    QString text;

    text += QString( "// ResInsight version string : %1\n" ).arg( STRPRODUCTVER );
    text += QString( "// Report generated : %1\n" ).arg( QDateTime::currentDateTime().toString() );
    text += "//\n";
    text += "//\n";
    text += "\n";

    return text;
}
