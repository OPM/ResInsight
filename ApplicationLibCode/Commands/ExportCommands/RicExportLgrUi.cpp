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

#include "RicExportLgrUi.h"

#include "RicCellRangeUi.h"

#include "RimEclipseCase.h"
#include "RimTools.h"

#include "cafPdmUiFilePathEditor.h"
#include "cafPdmUiTextEditor.h"
#include "cafVecIjk.h"

CAF_PDM_SOURCE_INIT( RicExportLgrUi, "RicExportLgrUi" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
namespace caf
{
template <>
void Lgr::SplitTypeEnum::setUp()
{
    addItem( Lgr::LGR_PER_CELL, "LGR_PER_CELL", "LGR Per Cell" );
    addItem( Lgr::LGR_PER_COMPLETION, "LGR_PER_COMPLETION", "LGR Per Completion" );
    addItem( Lgr::LGR_PER_WELL, "LGR_PER_WELL", "LGR Per Well" );

    setDefault( Lgr::LGR_PER_COMPLETION );
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicExportLgrUi::RicExportLgrUi()
{
    CAF_PDM_InitObject( "Export CARFIN" );

    CAF_PDM_InitFieldNoDefault( &m_exportFolder, "ExportFolder", "Export Folder" );
    m_exportFolder.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_caseToApply, "CaseToApply", "Source Case" );
    CAF_PDM_InitFieldNoDefault( &m_timeStep, "TimeStepIndex", "Time Step" );

    CAF_PDM_InitField( &m_includePerforations, "IncludePerforations", true, "Perforations" );
    CAF_PDM_InitField( &m_includeFractures, "IncludeFractures", true, "Fractures" );
    CAF_PDM_InitField( &m_includeFishbones, "IncludeFishbones", true, "Fishbones" );

    QString ijkLabel = "Refinement I, J, K";
    CAF_PDM_InitField( &m_refinementI, "CellCountI", 2, ijkLabel );
    CAF_PDM_InitField( &m_refinementJ, "CellCountJ", 2, "" );
    CAF_PDM_InitField( &m_refinementK, "CellCountK", 2, "" );

    m_refinementJ.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_refinementK.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitField( &m_splitType, "SplitType", Lgr::SplitTypeEnum(), "Split Type" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportLgrUi::setCase( RimEclipseCase* rimCase )
{
    bool isDifferent = ( rimCase != m_caseToApply );

    if ( isDifferent )
    {
        m_caseToApply = rimCase;
        setDefaultValuesFromCase();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportLgrUi::setTimeStep( int timeStep )
{
    bool isDifferent = timeStep != m_timeStep;

    if ( isDifferent )
    {
        m_timeStep = timeStep;
        setDefaultValuesFromCase();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3st RicExportLgrUi::refinement() const
{
    return cvf::Vec3st( m_refinementI, m_refinementJ, m_refinementK );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicExportLgrUi::exportFolder() const
{
    return m_exportFolder();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RicExportLgrUi::caseToApply() const
{
    return m_caseToApply();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RicExportLgrUi::timeStep() const
{
    return m_timeStep;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RigCompletionData::CompletionType> RicExportLgrUi::completionTypes() const
{
    std::set<RigCompletionData::CompletionType> cts;
    if ( m_includePerforations() ) cts.insert( RigCompletionData::CompletionType::PERFORATION );
    if ( m_includeFractures() ) cts.insert( RigCompletionData::CompletionType::FRACTURE );
    if ( m_includeFishbones() ) cts.insert( RigCompletionData::CompletionType::FISHBONES );
    return cts;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Lgr::SplitType RicExportLgrUi::splitType() const
{
    return m_splitType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportLgrUi::hideExportFolderField( bool hide )
{
    m_exportFolder.uiCapability()->setUiHidden( hide );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportLgrUi::setExportFolder( const QString& folder )
{
    m_exportFolder = folder;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportLgrUi::setDefaultValuesFromCase()
{
    if ( m_caseToApply )
    {
        QString caseFolder = m_caseToApply->locationOnDisc();
        m_exportFolder     = caseFolder;
    }

    m_refinementI = 2;
    m_refinementJ = 2;
    m_refinementK = 2;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RicExportLgrUi::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_caseToApply )
    {
        RimTools::caseOptionItems( &options );
    }
    else if ( fieldNeedingOptions == &m_timeStep )
    {
        RimTools::timeStepsForCase( m_caseToApply, &options );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportLgrUi::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_caseToApply )
    {
        setDefaultValuesFromCase();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportLgrUi::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiOrdering::LayoutOptions layout = { .newRow = true, .totalColumnSpan = 6, .leftLabelColumnSpan = 1 };
    uiOrdering.add( &m_caseToApply, layout );
    uiOrdering.add( &m_timeStep, layout );
    uiOrdering.add( &m_exportFolder, layout );
    uiOrdering.add( &m_includeFractures, layout );
    uiOrdering.add( &m_includeFishbones, layout );
    uiOrdering.add( &m_includePerforations, layout );
    uiOrdering.add( &m_splitType, { .newRow = true, .totalColumnSpan = 6, .leftLabelColumnSpan = 1 } );

    caf::PdmUiGroup* gridRefinement = uiOrdering.addNewGroup( "Grid Refinement" );
    gridRefinement->add( &m_refinementI, { .newRow = true, .totalColumnSpan = 2, .leftLabelColumnSpan = 1 } );
    gridRefinement->appendToRow( &m_refinementJ );
    gridRefinement->appendToRow( &m_refinementK );

    //    uiOrdering.add(&m_wellPathsInfo);
    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportLgrUi::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_exportFolder )
    {
        caf::PdmUiFilePathEditorAttribute* myAttr = dynamic_cast<caf::PdmUiFilePathEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->m_selectDirectory = true;
        }
    }
}
