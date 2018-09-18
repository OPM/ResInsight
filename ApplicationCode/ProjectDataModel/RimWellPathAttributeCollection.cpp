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

CAF_PDM_SOURCE_INIT(RimWellPathAttributeCollection, "WellPathAttributes");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathAttributeCollection::RimWellPathAttributeCollection()
{
    CAF_PDM_InitObject("WellPathAttributes", ":/Well.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_attributes, "Attributes", "", "", "", "");
    m_attributes.uiCapability()->setUiEditorTypeName(caf::PdmUiTableViewEditor::uiEditorTypeName());
    m_attributes.uiCapability()->setUiTreeChildrenHidden(true);
    m_attributes.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathAttributeCollection::deleteAttribute(RimWellPathAttribute* attributeToDelete)
{
    m_attributes.removeChildObject(attributeToDelete);
    delete attributeToDelete;    
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
            tvAttribute->forceColumnWidthResize = true;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathAttributeCollection::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* attrGroup = uiOrdering.addNewGroup("Well Path Attributes");
    attrGroup->add(&m_attributes);
}
