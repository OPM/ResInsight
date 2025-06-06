/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RicSelectPlotTemplateUi.h"

#include "Summary/RiaSummaryTools.h"

#include "PlotTemplates/RimPlotTemplateFileItem.h"
#include "PlotTemplates/RimPlotTemplateFolderItem.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimSummaryPlot.h"

#include "cafPdmUiTreeSelectionEditor.h"

CAF_PDM_SOURCE_INIT( RicSelectPlotTemplateUi, "RicSelectPlotTemplateUi" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicSelectPlotTemplateUi::RicSelectPlotTemplateUi()
    : m_useMultiSelect( false )
{
    CAF_PDM_InitObject( "RicSelectPlotTemplateUi" );

    CAF_PDM_InitFieldNoDefault( &m_selectedPlotTemplates, "SelectedPlotTemplates", "Plot Templates" );
    m_selectedPlotTemplates.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
    m_selectedPlotTemplates.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSelectPlotTemplateUi::setMultiSelectMode( bool multiSelect )
{
    m_useMultiSelect = multiSelect;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSelectPlotTemplateUi::setInitialSelection( const std::vector<QString>& selectedTemplates )
{
    auto plotTemplateRoot = RimProject::current()->rootPlotTemplateItem();

    std::vector<RimPlotTemplateFileItem*> fileItems;
    RimPlotTemplateFolderItem::allPlotTemplates( fileItems, plotTemplateRoot );

    for ( const auto& selectedTemplate : selectedTemplates )
    {
        for ( const auto& fileItem : fileItems )
        {
            if ( fileItem->absoluteFilePath() == selectedTemplate )
            {
                m_selectedPlotTemplates.push_back( fileItem );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlotTemplateFileItem*> RicSelectPlotTemplateUi::selectedPlotTemplates()
{
    std::vector<RimPlotTemplateFileItem*> objs;

    for ( const auto& a : m_selectedPlotTemplates() )
    {
        if ( a )
        {
            objs.push_back( a );
        }
    }

    return objs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RicSelectPlotTemplateUi::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_selectedPlotTemplates )
    {
        auto plotTemplateRoot = RimProject::current()->rootPlotTemplateItem();

        RimPlotTemplateFolderItem::appendOptionItemsForPlotTemplates( options, plotTemplateRoot );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSelectPlotTemplateUi::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( &m_selectedPlotTemplates == field )
    {
        auto a = dynamic_cast<caf::PdmUiTreeSelectionEditorAttribute*>( attribute );
        if ( a )
        {
            a->singleSelectionMode = !m_useMultiSelect;
        }
    }
}
