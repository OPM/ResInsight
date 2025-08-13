
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RimTools.h"

#include "RigFemPart.h"
#include "RigFemPartCollection.h"
#include "RigGeoMechCaseData.h"
#include "RigReservoirGridTools.h"

#include "Polygons/RimPolygon.h"
#include "Polygons/RimPolygonCollection.h"
#include "RimCase.h"
#include "RimColorLegend.h"
#include "RimColorLegendCollection.h"
#include "RimCompletionTemplateCollection.h"
#include "RimEclipseCase.h"
#include "RimFaultInView.h"
#include "RimFaultInViewCollection.h"
#include "RimGeoMechCase.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSeismicData.h"
#include "RimSeismicDataCollection.h"
#include "RimSeismicDifferenceData.h"
#include "RimSurfaceCollection.h"
#include "RimValveTemplateCollection.h"
#include "RimViewWindow.h"
#include "RimWellLogLasFile.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "cafPdmUiItem.h"
#include "cafUtils.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimTools::getCacheRootDirectoryPathFromProject()
{
    if ( !RimProject::current() )
    {
        return QString();
    }

    QString projectFileName = RimProject::current()->fileName();

    QString   cacheRootFolderPath;
    QFileInfo fileInfo( projectFileName );
    cacheRootFolderPath = fileInfo.canonicalPath();
    cacheRootFolderPath += "/" + fileInfo.completeBaseName();

    return cacheRootFolderPath;
}

//--------------------------------------------------------------------------------------------------
///  Relocate the supplied file name, based on the search path as follows:
///  fileName, newProjectPath/fileNameWoPath, relocatedPath/fileNameWoPath
///  If the file is not found in any of the positions, the fileName is returned but converted to Qt Style path
///  separators: "/"
///
///  The relocatedPath is found in this way:
///  use the start of newProjectPath
///  plus the end of the path to m_gridFileName
///  such that the common start of oldProjectPath and m_gridFileName is removed from m_gridFileName
///  and replaced with the start of newProjectPath up to where newProjectPath starts to be equal to oldProjectPath
//--------------------------------------------------------------------------------------------------
QString RimTools::relocateFile( const QString& originalFileName, const QString& currentProjectPath, const QString& previousProjectPath )
{
    // Make sure we have a Qt formatted path ( using "/" not "\")
    QString fileName       = QDir::fromNativeSeparators( originalFileName );
    QString newProjectPath = QDir::fromNativeSeparators( currentProjectPath );
    QString oldProjectPath = QDir::fromNativeSeparators( previousProjectPath );

    // If we from a file or whatever gets a real windows path on linux, we need to manually convert it
    // because Qt will not. QDir::fromNativeSeparators does nothing on linux.

    bool isWindowsPath = false;
    if ( originalFileName.count( "/" ) )
        isWindowsPath = false; // "/" are not allowed in a windows path
    else if ( originalFileName.count( "\\" ) && !caf::Utils::fileExists( originalFileName ) ) // To make sure we do not
                                                                                              // convert single linux
                                                                                              // files containing "\"
    {
        isWindowsPath = true;
    }

    if ( isWindowsPath )
    {
        // Windows absolute path detected. transform.
        fileName.replace( QString( "\\" ), QString( "/" ) );
    }

    if ( caf::Utils::fileExists( fileName ) )
    {
        return fileName;
    }

    // First check in the new project file directory
    {
        QString fileNameWithoutPath = QFileInfo( fileName ).fileName();
        QString candidate           = QDir::fromNativeSeparators( newProjectPath + QDir::separator() + fileNameWithoutPath );

        if ( caf::Utils::fileExists( candidate ) )
        {
            return candidate;
        }
    }

    // Then find the possible move of a directory structure where projects and files referenced are moved in "paralell"

    QFileInfo   fileNameFileInfo( QDir::fromNativeSeparators( fileName ) );
    QString     fileNamePath         = fileNameFileInfo.path();
    QString     fileNameWithoutPath  = fileNameFileInfo.fileName();
    QStringList fileNamePathElements = fileNamePath.split( "/" );

    QString     oldProjPath         = QDir::fromNativeSeparators( oldProjectPath );
    QStringList oldProjPathElements = oldProjPath.split( "/" );

    QString     newProjPath         = QDir::fromNativeSeparators( newProjectPath );
    QStringList newProjPathElements = newProjPath.split( "/" );

    // Find the possible equal start of the old project path, and the referenced file

    bool pathStartsAreEqual = false;
    bool pathEndsDiffer     = false;
    int  firstDiffIdx       = 0;
    for ( firstDiffIdx = 0; firstDiffIdx < fileNamePathElements.size() && firstDiffIdx < oldProjPathElements.size(); ++firstDiffIdx )
    {
#ifdef WIN32
        // When comparing parts of a file path, the drive letter has been seen to be a mix of
        // upper and lower cases. Always use case insensitive compare on Windows, as this is a valid approach
        // for all parts for a file path
        Qt::CaseSensitivity cs = Qt::CaseInsensitive;
#else
        Qt::CaseSensitivity cs = Qt::CaseSensitive;
#endif
        if ( fileNamePathElements[firstDiffIdx].compare( oldProjPathElements[firstDiffIdx], cs ) == 0 )
        {
            pathStartsAreEqual = pathStartsAreEqual || !fileNamePathElements[firstDiffIdx].isEmpty();
        }
        else
        {
            pathEndsDiffer = true;
            break;
        }
    }

    if ( ( !pathEndsDiffer && firstDiffIdx < fileNamePathElements.size() ) || firstDiffIdx < oldProjPathElements.size() )
    {
        pathEndsDiffer = true;
    }

    // If the path starts are equal, try to substitute it in the referenced file, with the corresponding new project
    // path start

    if ( pathStartsAreEqual )
    {
        if ( pathEndsDiffer )
        {
            QString oldFileNamePathEnd;
            for ( int i = firstDiffIdx; i < fileNamePathElements.size(); ++i )
            {
                oldFileNamePathEnd += fileNamePathElements[i];
                oldFileNamePathEnd += "/";
            }

            // Find the new Project File Start Path

            QStringList oldProjectFilePathEndElements;
            for ( int i = firstDiffIdx; i < oldProjPathElements.size(); ++i )
            {
                oldProjectFilePathEndElements.push_back( oldProjPathElements[i] );
            }

            int lastProjectDiffIdx = newProjPathElements.size() - 1;
            {
                int ppIdx = oldProjectFilePathEndElements.size() - 1;

                for ( ; lastProjectDiffIdx >= 0 && ppIdx >= 0; --lastProjectDiffIdx, --ppIdx )
                {
                    if ( oldProjectFilePathEndElements[ppIdx] != newProjPathElements[lastProjectDiffIdx] )
                    {
                        break;
                    }
                }
            }

            QString newProjectFileStartPath;
            for ( int i = 0; i <= lastProjectDiffIdx; ++i )
            {
                newProjectFileStartPath += newProjPathElements[i];
                newProjectFileStartPath += "/";
            }

            QString relocationPath = newProjectFileStartPath + oldFileNamePathEnd;

            QString relocatedFileName = relocationPath + fileNameWithoutPath;

            if ( caf::Utils::fileExists( relocatedFileName ) )
            {
                return relocatedFileName;
            }
        }
        else
        {
            // The Grid file was located in the same dir as the Project file. This is supposed to be handled above.
            // So we did not find it
        }
    }

    return fileName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimTools::relocatePathPattern( const QString& originalPattern, const QString& currentProjectPath, const QString& previousProjectPath )
{
    auto previousProjectPathStd = previousProjectPath.toStdString();
    auto currentProjectPathStd  = currentProjectPath.toStdString();

    auto originalPatternStd = originalPattern.toStdString();

    size_t equalIndex = 0;
    for ( size_t i = 0; i < std::min( previousProjectPathStd.size(), originalPatternStd.size() ); ++i )
    {
        if ( originalPatternStd[i] != previousProjectPathStd[i] )
        {
            equalIndex = i;
            break;
        }
    }

    if ( equalIndex > 0 )
    {
        auto theRest = previousProjectPathStd.substr( equalIndex );

        // remove theRest from the back of newProjectPath
        auto newProjectPathWithoutTheRest = currentProjectPathStd.substr( 0, currentProjectPathStd.size() - theRest.size() );
        auto newPattern                   = newProjectPathWithoutTheRest + originalPatternStd.substr( equalIndex );

        return QString::fromStdString( newPattern );
    }

    return originalPattern;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTools::wellPathOptionItemsSubset( const std::vector<RimWellPath*>& wellPathsToExclude, QList<caf::PdmOptionItemInfo>* options )
{
    if ( !options ) return;

    auto wellPathColl = RimTools::wellPathCollection();
    if ( wellPathColl )
    {
        std::vector<RimWellPath*> wellPathsToInclude;

        auto all = wellPathColl->allWellPaths();
        for ( auto w : all )
        {
            bool include = true;
            for ( auto exclude : wellPathsToExclude )
            {
                if ( w == exclude ) include = false;
            }

            if ( include ) wellPathsToInclude.push_back( w );
        }

        optionItemsForSpecifiedWellPaths( wellPathsToInclude, options );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTools::wellPathOptionItems( QList<caf::PdmOptionItemInfo>* options )
{
    if ( !options ) return;

    auto wellPathColl = RimTools::wellPathCollection();
    if ( wellPathColl )
    {
        optionItemsForSpecifiedWellPaths( wellPathColl->allWellPaths(), options );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTools::wellPathWithFormationsOptionItems( QList<caf::PdmOptionItemInfo>* options )
{
    if ( !options ) return;

    std::vector<RimWellPath*> wellPaths;
    RimTools::wellPathWithFormations( &wellPaths );

    optionItemsForSpecifiedWellPaths( wellPaths, options );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTools::wellPathWithFormations( std::vector<RimWellPath*>* wellPaths )
{
    auto wellPathColl = RimTools::wellPathCollection();
    if ( wellPathColl )
    {
        for ( RimWellPath* wellPath : wellPathColl->allWellPaths() )
        {
            if ( wellPath->hasFormations() )
            {
                wellPaths->push_back( wellPath );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTools::caseOptionItems( QList<caf::PdmOptionItemInfo>* options )
{
    if ( !options ) return;

    RimProject* proj = RimProject::current();
    if ( proj )
    {
        std::vector<RimCase*> cases = proj->allGridCases();
        for ( RimCase* c : cases )
        {
            options->push_back( caf::PdmOptionItemInfo( c->caseUserDescription(), c, false, c->uiIconProvider() ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTools::eclipseCaseOptionItems( QList<caf::PdmOptionItemInfo>* options )
{
    if ( !options ) return;

    RimProject* proj = RimProject::current();
    if ( proj )
    {
        std::vector<RimCase*> cases = proj->allGridCases();
        for ( RimCase* c : cases )
        {
            RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( c );
            if ( eclipseCase )
            {
                options->push_back( caf::PdmOptionItemInfo( c->caseUserDescription(), c, false, c->uiIconProvider() ) );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTools::geoMechCaseOptionItems( QList<caf::PdmOptionItemInfo>* options )
{
    if ( !options ) return;

    RimProject* proj = RimProject::current();
    if ( proj )
    {
        std::vector<RimCase*> cases = proj->allGridCases();
        for ( RimCase* c : cases )
        {
            RimGeoMechCase* geoMechCase = dynamic_cast<RimGeoMechCase*>( c );
            if ( geoMechCase )
            {
                options->push_back( caf::PdmOptionItemInfo( c->caseUserDescription(), c, false, c->uiIconProvider() ) );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTools::eclipseGridOptionItems( QList<caf::PdmOptionItemInfo>* options, RimEclipseCase* eCase )
{
    if ( !options ) return;

    for ( int gIdx = 0; gIdx < RigReservoirGridTools::gridCount( eCase ); gIdx++ )
    {
        QString gridName = RigReservoirGridTools::gridName( eCase, gIdx );
        if ( gIdx == 0 )
        {
            if ( gridName.isEmpty() )
                gridName += "Main Grid";
            else
                gridName += " (Main Grid)";
        }

        options->push_back( caf::PdmOptionItemInfo( gridName, gIdx ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTools::geoMechPartOptionItems( QList<caf::PdmOptionItemInfo>* options, RimGeoMechCase* gCase )
{
    if ( !options ) return;

    if ( !gCase || !gCase->geoMechData() || !gCase->geoMechData()->femParts() ) return;

    const auto parts = gCase->geoMechData()->femParts();

    for ( int i = 0; i < parts->partCount(); i++ )
    {
        auto part = parts->part( i );
        if ( part != nullptr )
        {
            options->push_back( caf::PdmOptionItemInfo( QString::fromStdString( part->name() ), i ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTools::geoMechElementSetOptionItems( QList<caf::PdmOptionItemInfo>* options, RimGeoMechCase* gCase, int partId )
{
    if ( !options ) return;

    if ( !gCase || !gCase->geoMechData() || !gCase->geoMechData()->femParts() ) return;

    const auto parts = gCase->geoMechData()->femParts();

    if ( partId >= parts->partCount() ) return;

    auto part = parts->part( partId );
    if ( part != nullptr )
    {
        auto names = part->elementSetNames();

        for ( int i = 0; i < (int)names.size(); i++ )
        {
            options->push_back( caf::PdmOptionItemInfo( QString::fromStdString( names[i] ), i ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTools::seismicDataOptionItems( QList<caf::PdmOptionItemInfo>* options )
{
    if ( !options ) return;

    RimProject* proj = RimProject::current();
    if ( proj )
    {
        const auto& coll = proj->activeOilField()->seismicDataCollection().p();

        for ( auto* c : coll->seismicData() )
        {
            options->push_back( caf::PdmOptionItemInfo( QString::fromStdString( c->userDescription() ), c, false, c->uiIconProvider() ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTools::seismicDataOptionItems( QList<caf::PdmOptionItemInfo>* options, cvf::BoundingBox worldBBox, bool basicDataOnly )
{
    if ( !options ) return;

    RimProject* proj = RimProject::current();
    if ( proj )
    {
        const auto& coll = proj->activeOilField()->seismicDataCollection().p();

        for ( auto* c : coll->seismicData() )
        {
            if ( c->boundingBox()->intersects( worldBBox ) )
                options->push_back( caf::PdmOptionItemInfo( QString::fromStdString( c->userDescription() ), c, false, c->uiIconProvider() ) );
        }

        if ( !basicDataOnly )
        {
            for ( auto* c : coll->differenceData() )
            {
                if ( c->boundingBox()->intersects( worldBBox ) )
                    options->push_back(
                        caf::PdmOptionItemInfo( QString::fromStdString( c->userDescription() ), c, false, c->uiIconProvider() ) );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTools::polygonOptionItems( QList<caf::PdmOptionItemInfo>* options )
{
    auto project = RimProject::current();
    auto coll    = project->activeOilField()->polygonCollection();

    for ( auto* p : coll->allPolygons() )
    {
        options->push_back( caf::PdmOptionItemInfo( p->name(), p, false, p->uiIconProvider() ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTools::colorLegendOptionItems( QList<caf::PdmOptionItemInfo>* options )
{
    if ( !options ) return;

    RimProject*                  project               = RimProject::current();
    RimColorLegendCollection*    colorLegendCollection = project->colorLegendCollection();
    std::vector<RimColorLegend*> colorLegends          = colorLegendCollection->allColorLegends();

    for ( RimColorLegend* colorLegend : colorLegends )
    {
        options->push_back( caf::PdmOptionItemInfo( colorLegend->colorLegendName(), colorLegend, false, colorLegend->paletteIconProvider() ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathCollection* RimTools::wellPathCollection()
{
    RimProject* proj = RimProject::current();
    if ( proj && proj->activeOilField() )
    {
        return proj->activeOilField()->wellPathCollection();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* RimTools::firstWellPath()
{
    auto wellpathcoll = wellPathCollection();
    auto wellpaths    = wellpathcoll->allWellPaths();

    if ( !wellpaths.empty() ) return wellpaths[0];

    return nullptr;
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimValveTemplateCollection* RimTools::valveTemplateCollection()
{
    auto proj = RimProject::current();
    if ( !proj ) return nullptr;

    auto oilField = proj->activeOilField();
    if ( !oilField ) return nullptr;

    auto compColl = oilField->completionTemplateCollection();
    return compColl ? compColl->valveTemplateCollection() : nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceCollection* RimTools::surfaceCollection()
{
    return RimProject::current()->activeOilField()->surfaceCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimJobCollection* RimTools::jobCollection()
{
    return RimProject::current()->jobCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonCollection* RimTools::polygonCollection()
{
    return RimProject::current()->activeOilField()->polygonCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAutomationSettings* RimTools::automationSettings()
{
    return RimProject::current()->automationSettings();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTools::timeStepsForCase( RimCase* gridCase, QList<caf::PdmOptionItemInfo>* options )
{
    QStringList timeStepNames;

    if ( gridCase )
    {
        timeStepNames = gridCase->timeStepStrings();
    }

    for ( int i = 0; i < timeStepNames.size(); i++ )
    {
        options->push_back( caf::PdmOptionItemInfo( timeStepNames[i], i ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTools::optionItemsForSpecifiedWellPaths( const std::vector<RimWellPath*>& wellPaths, QList<caf::PdmOptionItemInfo>* options )
{
    if ( !options ) return;

    caf::IconProvider wellIcon( ":/Well.svg" );
    for ( auto wellPath : wellPaths )
    {
        options->push_back( caf::PdmOptionItemInfo( wellPath->name(), wellPath, false, wellIcon ) );
    }
}

//--------------------------------------------------------------------------------------------------
/// This function is intended to be called when multiple views are toggled or displayed. Can be called from a plot collection in
/// onChildrenUpdated()
//--------------------------------------------------------------------------------------------------
void RimTools::updateViewWindowContent( std::vector<caf::PdmObjectHandle*>& objects )
{
    for ( auto& obj : objects )
    {
        if ( auto viewWindow = dynamic_cast<RimViewWindow*>( obj ) )
        {
            if ( viewWindow->showWindow() )
            {
                viewWindow->loadDataAndUpdate();
            }
            else
            {
                viewWindow->updateMdiWindowVisibility();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTools::faultOptionItems( QList<caf::PdmOptionItemInfo>* options, RimFaultInViewCollection* coll )
{
    if ( !options ) return;
    if ( !coll ) return;

    for ( auto& f : coll->faults() )
    {
        options->push_back( caf::PdmOptionItemInfo( f->name(), f, false, f->uiIconProvider() ) );
    }
}
