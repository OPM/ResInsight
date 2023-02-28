/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023     Equinor ASA
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

#include "RimCheckableNamedObject.h"

#include "RimPolylinePickerInterface.h"
#include "RimPolylinesDataInterface.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmFieldCvfColor.h"
#include "cafPdmFieldCvfVec3d.h"
#include "cafPdmPtrField.h"

#include "cvfArray.h"
#include "cvfColor3.h"
#include "cvfObject.h"

#include <QList>
#include <QString>

class RicPolylineTargetsPickEventHandler;
class RimPolylineTarget;
class RigPolylinesData;
class RigTexturedSection;
class RivSeismicSectionPartMgr;
class Rim3dView;
class RimSeismicData;
class RimRegularLegendConfig;

class RimSeismicSection : public RimCheckableNamedObject, public RimPolylinePickerInterface, public RimPolylinesDataInterface
{
    CAF_PDM_HEADER_INIT;

public:
    enum class CrossSectionEnum
    {
        CS_INLINE,
        CS_XLINE,
        CS_DEPTHSLICE,
        CS_POLYLINE
    };

public:
    RimSeismicSection();
    ~RimSeismicSection() override;

    QString userDescription();
    void    setUserDescription( QString description );

    void updateVisualization() override;
    void updateEditorsAndVisualization() override;
    void insertTarget( const RimPolylineTarget* targetToInsertBefore, RimPolylineTarget* targetToInsert ) override;
    void deleteTarget( RimPolylineTarget* targetToDelete ) override;
    void enablePicking( bool enable );

    std::vector<RimPolylineTarget*> activeTargets() const override;
    bool                            pickingEnabled() const override;
    caf::PickEventHandler*          pickEventHandler() const override;

    cvf::ref<RigPolyLinesData> polyLinesData() const override;

    cvf::ref<RigTexturedSection> texturedSection() const;

    RivSeismicSectionPartMgr* partMgr();
    void                      rebuildGeometry();

    RimSeismicData* seismicData() const;

    RimRegularLegendConfig* legendConfig() const;

protected:
    void                 initAfterRead() override;
    caf::PdmFieldHandle* userDescriptionField() override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    void initSliceRanges();

    caf::PdmField<QString>            m_userDescription;
    caf::PdmPtrField<RimSeismicData*> m_seismicData;

    caf::PdmField<bool>                           m_showSeismicOutline;
    caf::PdmField<bool>                           m_enablePicking;
    caf::PdmChildArrayField<RimPolylineTarget*>   m_targets;
    caf::PdmField<int>                            m_lineThickness;
    caf::PdmField<cvf::Color3f>                   m_lineColor;
    caf::PdmChildField<RimRegularLegendConfig*>   m_legendConfig;
    caf::PdmField<caf::AppEnum<CrossSectionEnum>> m_type;

    caf::PdmField<int> m_inlineIndex;
    caf::PdmField<int> m_xlineIndex;
    caf::PdmField<int> m_depthIndex;

    std::shared_ptr<RicPolylineTargetsPickEventHandler> m_pickTargetsEventHandler;
    cvf::ref<RivSeismicSectionPartMgr>                  m_sectionPartMgr;
};
