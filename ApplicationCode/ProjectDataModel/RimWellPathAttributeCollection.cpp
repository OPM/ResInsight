/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 -    Equinor ASA
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
#include "RimWellPathAttributeCollection.h"

#include "RimWellPathAttribute.h"
#include "RimWellLogTrack.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiTableViewEditor.h"
#include "cafPdmUiTreeOrdering.h"

CAF_PDM_SOURCE_INIT(RimWellPathAttributeCollection, "WellPathAttributes");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathAttributeCollection::RimWellPathAttributeCollection()
{
    CAF_PDM_InitObject("Casing Design", ":/CompletionsSymbol16x16", "", "");

    CAF_PDM_InitFieldNoDefault(&m_attributes, "Attributes", "Casing Design Attributes", "", "", "");
    m_attributes.uiCapability()->setUiEditorTypeName(caf::PdmUiTableViewEditor::uiEditorTypeName());
    m_attributes.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::TOP);
    m_attributes.uiCapability()->setCustomContextMenuEnabled(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathAttributeCollection::~RimWellPathAttributeCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathAttributeCollection::updateAllReferringTracks()
{
    std::vector<RimWellLogTrack*> wellLogTracks;

    this->objectsWithReferringPtrFieldsOfType(wellLogTracks);
    for (RimWellLogTrack* track : wellLogTracks)
    {
        track->loadDataAndUpdate();
    }
    this->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPathAttribute*> RimWellPathAttributeCollection::attributes() const
{
    std::vector<RimWellPathAttribute*> attrs;

    for (auto attr : m_attributes)
    {
        attrs.push_back(attr.p());
    }
    return attrs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathAttributeCollection::insertAttribute(RimWellPathAttribute* insertBefore, RimWellPathAttribute* attribute)
{
    size_t index = m_attributes.index(insertBefore);
    if (index < m_attributes.size())
        m_attributes.insert(index, attribute);
    else
        m_attributes.push_back(attribute);

    this->updateAllReferringTracks();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathAttributeCollection::deleteAttribute(RimWellPathAttribute* attributeToDelete)
{
    m_attributes.removeChildObject(attributeToDelete);
    delete attributeToDelete;    

    this->updateAllReferringTracks();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathAttributeCollection::deleteAllAttributes()
{
    m_attributes.deleteAllChildObjects();
    this->updateAllReferringTracks();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathAttributeCollection::defineCustomContextMenu(const caf::PdmFieldHandle* fieldNeedingMenu,
                                                    QMenu*                     menu,
                                                    QWidget*                   fieldEditorWidget)
{
    caf::CmdFeatureMenuBuilder menuBuilder;

    menuBuilder << "RicNewWellPathAttributeFeature";
    menuBuilder << "Separator";
    menuBuilder << "RicDeleteWellPathAttributeFeature";

    menuBuilder.appendToMenu(menu);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathAttributeCollection::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    if (field == &m_attributes)
    {
        auto tvAttribute = dynamic_cast<caf::PdmUiTableViewEditorAttribute*>(attribute);
        if (tvAttribute)
        {
            tvAttribute->autoResizeColumnsToFillContainer = true;
            tvAttribute->minimumHeight = 300;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathAttributeCollection::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_attributes);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathAttributeCollection::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    uiTreeOrdering.skipRemainingChildren(true);
}
