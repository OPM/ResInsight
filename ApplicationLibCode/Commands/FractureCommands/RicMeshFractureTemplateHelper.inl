/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022-     Equinor ASA
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

#include "RiaApplication.h"

#include "RimEclipseView.h"
#include "RimFractureTemplateCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimThermalFractureTemplate.h"
#include "RimWellPathFracture.h"

#include "Riu3DMainWindowTools.h"
#include "RiuFileDialogTools.h"

#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>
#include <QFileInfo>
#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <class T>
void RicMeshFractureTemplateHelper<T>::createNewTemplateForFractureAndUpdate( RimFracture*   fracture,
                                                                              const QString& title,
                                                                              const QString& lastUsedDialogFallback,
                                                                              const QString& fileFilter,
                                                                              const QString& defaultTemplateName )
{
    std::vector<T*> newTemplates = createNewTemplates( title, lastUsedDialogFallback, fileFilter, defaultTemplateName );
    if ( !newTemplates.empty() )
    {
        T* lastTemplateCreated = newTemplates.back();
        fracture->setFractureTemplate( lastTemplateCreated );

        selectFractureTemplateAndUpdate( lastTemplateCreated );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <class T>
void RicMeshFractureTemplateHelper<T>::selectFractureTemplateAndUpdate( RimFractureTemplate* fractureTemplate )
{
    fractureTemplate->loadDataAndUpdate();

    RimFractureTemplateCollection* templateCollection = nullptr;
    fractureTemplate->firstAncestorOrThisOfTypeAsserted( templateCollection );
    templateCollection->updateConnectedEditors();

    RimProject* project = RimProject::current();

    project->scheduleCreateDisplayModelAndRedrawAllViews();
    Riu3DMainWindowTools::selectAsCurrentItem( fractureTemplate );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <class T>
std::vector<T*> RicMeshFractureTemplateHelper<T>::createNewTemplates( const QString& title,
                                                                      const QString& lastUsedDialogFallback,
                                                                      const QString& fileFilter,
                                                                      const QString& defaultTemplateName )
{
    RiaApplication* app        = RiaApplication::instance();
    QString         defaultDir = app->lastUsedDialogDirectoryWithFallbackToProjectFolder( lastUsedDialogFallback );
    QStringList     fileNames  = RiuFileDialogTools::getOpenFileNames( nullptr, title, defaultDir, fileFilter );

    auto templates = createNewTemplatesFromFiles( std::vector<QString>( fileNames.begin(), fileNames.end() ), defaultTemplateName );

    if ( !fileNames.isEmpty() )
    {
        app->setLastUsedDialogDirectory( lastUsedDialogFallback, QFileInfo( fileNames.last() ).absolutePath() );
    }

    return templates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <class T>
std::vector<T*> RicMeshFractureTemplateHelper<T>::createNewTemplatesFromFiles( const std::vector<QString>& fileNames,
                                                                               const QString&              defaultTemplateName,
                                                                               bool reuseExistingTemplatesWithMatchingNames )
{
    if ( fileNames.empty() ) return std::vector<T*>();

    RimProject* project = RimProject::current();
    CVF_ASSERT( project );

    RimOilField* oilfield = project->activeOilField();
    if ( oilfield == nullptr ) return std::vector<T*>();

    RimFractureTemplateCollection* fracDefColl = oilfield->fractureDefinitionCollection();
    if ( !fracDefColl ) return std::vector<T*>();

    auto findTemplateByName = []( RimFractureTemplateCollection* coll, const QString& name ) -> T*
    {
        for ( auto t : coll->fractureTemplates() )
            if ( t->name() == name ) return dynamic_cast<T*>( t );

        return nullptr;
    };

    std::vector<T*> newFractures;
    for ( auto fileName : fileNames )
    {
        if ( fileName.isEmpty() ) continue;

        QFileInfo stimplanfileFileInfo( fileName );
        QString   name = stimplanfileFileInfo.baseName();
        if ( name.isEmpty() )
        {
            name = defaultTemplateName;
        }

        T* fractureDef = nullptr;
        if ( reuseExistingTemplatesWithMatchingNames ) fractureDef = findTemplateByName( fracDefColl, name );

        if ( fractureDef == nullptr )
        {
            fractureDef = new T();
            fracDefColl->addFractureTemplate( fractureDef );
            fractureDef->setName( name );
        }

        fractureDef->setFileName( fileName );
        fractureDef->loadDataAndUpdate();
        fractureDef->setDefaultsBasedOnFile();
        fractureDef->setDefaultWellDiameterFromUnit();
        newFractures.push_back( fractureDef );
    }

    return newFractures;
}
