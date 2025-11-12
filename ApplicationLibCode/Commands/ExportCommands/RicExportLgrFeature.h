/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#pragma once

#include "RigCompletionData.h"
#include "RigCompletionDataGridCell.h"

#include "RicExportLgrUi.h"

#include "cafCmdFeature.h"
#include "cafVecIjk.h"

#include <limits>
#include <memory>

class LgrNameFactory;
class RigMainGrid;
class RigGridBase;
class RimEclipseCase;
class RimSimWellInView;
class RimWellPath;
class QFile;
class QTextStream;
class IjkBoundingBox;

//==================================================================================================
/// Candidate for refactoring
//==================================================================================================

//==================================================================================================
///
//==================================================================================================
class LgrInfo
{
public:
    LgrInfo( int                 id,
             const QString&      name,
             const QString&      associatedWellPathName,
             const cvf::Vec3st&  refinement,
             const caf::VecIjk0& mainGridStartCell,
             const caf::VecIjk0& mainGridEndCell )
        : id( id )
        , name( name )
        , associatedWellPathName( associatedWellPathName )
        , refinement( refinement )
        , mainGridStartCell( mainGridStartCell )
        , mainGridEndCell( mainGridEndCell )
    {
    }

    cvf::Vec3st lgrCellCounts() const
    {
        auto lgrI = ( mainGridEndCell.i() - mainGridStartCell.i() + 1 ) * refinement.i();
        auto lgrJ = ( mainGridEndCell.j() - mainGridStartCell.j() + 1 ) * refinement.j();
        auto lgrK = ( mainGridEndCell.k() - mainGridStartCell.k() + 1 ) * refinement.k();

        lgrI = lgrI > 0 ? lgrI : 1;
        lgrJ = lgrJ > 0 ? lgrJ : 1;
        lgrK = lgrK > 0 ? lgrK : 1;

        return { lgrI, lgrJ, lgrK };
    }

    size_t lgrCellCount() const
    {
        auto size = lgrCellCounts();
        return size.i() * size.j() * size.k();
    }

    int     id;
    QString name;
    QString associatedWellPathName;

    cvf::Vec3st refinement;

    caf::VecIjk0 mainGridStartCell;
    caf::VecIjk0 mainGridEndCell;
};

//==================================================================================================
///
//==================================================================================================
class CompletionInfo
{
public:
    CompletionInfo()
        : type( RigCompletionData::CompletionType::CT_UNDEFINED )
        , name( "" )
        , wellPathName( "" )
    {
    }
    CompletionInfo( RigCompletionData::CompletionType type, const QString& name, const QString& wellPathName )
        : type( type )
        , name( name )
        , wellPathName( wellPathName )
    {
    }

    CompletionInfo( RigCompletionData::CompletionType type, const QString& name )
        : type( type )
        , name( name )
    {
    }
    RigCompletionData::CompletionType type;
    QString                           name;
    QString                           wellPathName;

    bool isValid() const { return type != RigCompletionData::CompletionType::CT_UNDEFINED && !name.isEmpty() && !wellPathName.isEmpty(); }

    int priority() const
    {
        return type == RigCompletionData::CompletionType::FRACTURE      ? 1
               : type == RigCompletionData::CompletionType::FISHBONES   ? 2
               : type == RigCompletionData::CompletionType::PERFORATION ? 3
                                                                        : 4;
    }

    // Sort by priority, then name, then number
    bool operator<( const CompletionInfo& other ) const
    {
        if ( priority() == other.priority() )
        {
            if ( wellPathName == other.wellPathName ) return name < other.name;
            return wellPathName < other.wellPathName;
        }
        return priority() < other.priority();
    }

    bool operator==( const CompletionInfo& other ) const
    {
        return type == other.type && name == other.name && wellPathName == other.wellPathName;
    }

    bool operator!=( const CompletionInfo& other ) const { return !operator==( other ); }
};

//==================================================================================================
///
//==================================================================================================
class RicExportLgrFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

    using Range = std::pair<size_t, size_t>;
    static Range initRange() { return std::make_pair( std::numeric_limits<size_t>::max(), 0 ); }

    static RicExportLgrUi* openDialog( const QString&  dialogTitle,
                                       RimEclipseCase* defaultCase           = nullptr,
                                       int             defaultTimeStep       = 0,
                                       bool            hideExportFolderField = false );
    static bool            openFileForExport( const QString& folderName, const QString& fileName, QFile* exportFile );
    static void            exportLgrsForWellPaths( const QString&                                     exportFolder,
                                                   std::vector<RimWellPath*>                          wellPaths,
                                                   RimEclipseCase*                                    eclipseCase,
                                                   size_t                                             timeStep,
                                                   const cvf::Vec3st&                                 refinement,
                                                   Lgr::SplitType                                     splitType,
                                                   const std::set<RigCompletionData::CompletionType>& completionTypes,
                                                   QStringList*                                       wellsIntersectingOtherLgrs );

    static void exportLgrs( const QString& exportFolder, const QString& wellName, const std::vector<LgrInfo>& lgrInfos );

    static std::vector<LgrInfo> buildLgrsForWellPaths( std::vector<RimWellPath*>                          wellPaths,
                                                       RimEclipseCase*                                    eclipseCase,
                                                       size_t                                             timeStep,
                                                       const cvf::Vec3st&                                 refinement,
                                                       Lgr::SplitType                                     splitType,
                                                       const std::set<RigCompletionData::CompletionType>& completionTypes,
                                                       QStringList*                                       wellsIntersectingOtherLgrs );

    static std::vector<RimWellPath*> selectedWellPaths();

    static std::map<QString, std::vector<LgrInfo>> createLgrInfoListForTemporaryLgrs( const RigMainGrid* mainGrid );

protected:
    bool isCommandEnabled() const override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;

private:
    static void writeLgrs( QTextStream& stream, const std::vector<LgrInfo>& lgrInfos );

    static std::vector<LgrInfo> buildLgrsPerMainCell( int                                           firstLgrId,
                                                      RimEclipseCase*                               eclipseCase,
                                                      RimWellPath*                                  wellPath,
                                                      const std::vector<RigCompletionDataGridCell>& intersectingCells,
                                                      const cvf::Vec3st&                            refinement,
                                                      LgrNameFactory&                               lgrNameFactory );
    static std::vector<LgrInfo> buildLgrsPerCompletion( int             firstLgrId,
                                                        RimEclipseCase* eclipseCase,
                                                        const std::map<CompletionInfo, std::vector<RigCompletionDataGridCell>>& completionInfo,
                                                        const cvf::Vec3st& refinement,
                                                        LgrNameFactory&    lgrNameFactory );
    static LgrInfo              buildLgr( int                                           lgrId,
                                          const QString&                                lgrName,
                                          const QString&                                wellPathName,
                                          const std::vector<RigCompletionDataGridCell>& intersectingCells,
                                          const cvf::Vec3st&                            refinement );

    static LgrInfo buildLgr( int                   lgrId,
                             const QString&        lgrName,
                             const QString&        wellPathName,
                             const IjkBoundingBox& boundingBox,
                             const cvf::Vec3st&    refinement );

    static std::vector<RigCompletionDataGridCell> cellsIntersectingCompletions( RimEclipseCase*    eclipseCase,
                                                                                const RimWellPath* wellPath,
                                                                                size_t             timeStep,
                                                                                const std::set<RigCompletionData::CompletionType>& completionTypes,
                                                                                bool* isIntersectingOtherLgrs );

    static std::map<CompletionInfo, std::vector<RigCompletionDataGridCell>>
        cellsIntersectingCompletions_PerCompletion( RimEclipseCase*                                    eclipseCase,
                                                    const std::vector<RimWellPath*>&                   wellPaths,
                                                    size_t                                             timeStep,
                                                    const std::set<RigCompletionData::CompletionType>& completionTypes,
                                                    QStringList*                                       wellsIntersectingOtherLgrs );

    static std::vector<RigCompletionDataGridCell> allIntersectedCells( RimEclipseCase* eclipseCase, const RimWellPath* wellPath );

    static int                firstAvailableLgrId( const RigMainGrid* mainGrid );
    static const RigGridBase* hostGrid( const RigMainGrid* mainGrid, size_t reservoirCellIndex );
};
