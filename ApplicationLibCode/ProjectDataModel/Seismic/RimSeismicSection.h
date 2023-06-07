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

#include "RiaSeismicDefines.h"

#include "RimCheckableNamedObject.h"
#include "RimIntersectionEnums.h"
#include "RimLegendConfigChangeType.h"
#include "RimPolylinePickerInterface.h"
#include "RimPolylinesDataInterface.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmFieldCvfColor.h"
#include "cafPdmFieldCvfVec3d.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"

#include "cvfArray.h"
#include "cvfColor3.h"
#include "cvfObject.h"

#include <QList>
#include <QString>

#include <vector>

class RicPolylineTargetsPickEventHandler;
class RimPolylineTarget;
class RigPolylinesData;
class RigWellPath;
class RigTexturedSection;
class RivSeismicSectionPartMgr;
class Rim3dView;
class RimSeismicDataInterface;
class RimRegularLegendConfig;
class RimSeismicAlphaMapper;
class RimWellPath;
class QMenu;
class QWidget;

class RimSeismicSection : public RimCheckableNamedObject, public RimPolylinePickerInterface, public RimPolylinesDataInterface
{
    CAF_PDM_HEADER_INIT;

public:
    RimSeismicSection();
    ~RimSeismicSection() override;

    QString userDescription();
    void    setUserDescription( QString description );
    QString fullName() const;

    void updateVisualization() override;
    void updateEditorsAndVisualization() override;
    void addTargetNoUpdate( RimPolylineTarget* target );
    void insertTarget( const RimPolylineTarget* targetToInsertBefore, RimPolylineTarget* targetToInsert ) override;
    void deleteTarget( RimPolylineTarget* targetToDelete ) override;
    void enablePicking( bool enable );

    void setSectionType( RiaDefines::SeismicSectionType sectionType );

    std::vector<RimPolylineTarget*> activeTargets() const override;
    bool                            pickingEnabled() const override;
    caf::PickEventHandler*          pickEventHandler() const override;

    cvf::ref<RigPolyLinesData> polyLinesData() const override;

    cvf::ref<RigTexturedSection> texturedSection();

    RivSeismicSectionPartMgr* partMgr();

    RimSeismicDataInterface* seismicData() const;
    void                     setSeismicData( RimSeismicDataInterface* seisData );

    RimRegularLegendConfig* legendConfig() const;
    RimSeismicAlphaMapper*  alphaValueMapper() const;

    bool isTransparent() const;

    int                       upperFilterZ( int upperGridLimit ) const;
    int                       lowerFilterZ( int lowerGridLimit ) const;
    RimIntersectionFilterEnum zFilterType() const;

    void setDepthFilter( RimIntersectionFilterEnum filterType, int upperValue, int lowerValue );

    QString resultInfoText( cvf::Vec3d worldCoord, int partIndex );

    void setWellPath( RimWellPath* wellPath );

protected:
    void                 initAfterRead() override;
    caf::PdmFieldHandle* userDescriptionField() override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    void defineCustomContextMenu( const caf::PdmFieldHandle* fieldNeedingMenu, QMenu* menu, QWidget* fieldEditorWidget ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    void onLegendConfigChanged( const caf::SignalEmitter* emitter, RimLegendConfigChangeType changeType );

    int alignZValue( int z ) const;

    void initSliceRanges();

    void scheduleViewUpdate();

    QPixmap getImage();

    void                    updateTextureSectionFromPoints( std::vector<cvf::Vec3d> points, double zmin, double zmax );
    std::vector<cvf::Vec3d> wellPathToSectionPoints( RigWellPath* wellpath, double zmin );

    caf::PdmField<QString>                     m_userDescription;
    caf::PdmPtrField<RimSeismicDataInterface*> m_seismicData;
    caf::PdmPtrField<RimWellPath*>             m_wellPath;
    caf::PdmProxyValueField<QString>           m_nameProxy;

    caf::PdmField<caf::AppEnum<RiaDefines::SeismicSectionType>> m_type;
    caf::PdmChildArrayField<RimPolylineTarget*>                 m_targets;

    caf::PdmField<double>       m_muteDataLimit;
    caf::PdmField<bool>         m_showSeismicOutline;
    caf::PdmField<bool>         m_enablePicking;
    caf::PdmField<bool>         m_showSectionLine;
    caf::PdmField<int>          m_lineThickness;
    caf::PdmField<cvf::Color3f> m_lineColor;
    caf::PdmField<bool>         m_transparent;
    caf::PdmField<bool>         m_showImage;
    caf::PdmField<int>          m_inlineIndex;
    caf::PdmField<int>          m_xlineIndex;
    caf::PdmField<int>          m_depthIndex;

    caf::PdmField<caf::AppEnum<RimIntersectionFilterEnum>> m_zFilterType;
    caf::PdmField<int>                                     m_zUpperThreshold;
    caf::PdmField<int>                                     m_zLowerThreshold;

    std::shared_ptr<RicPolylineTargetsPickEventHandler> m_pickTargetsEventHandler;
    cvf::ref<RivSeismicSectionPartMgr>                  m_sectionPartMgr;
    cvf::ref<RigTexturedSection>                        m_texturedSection;

    std::vector<cvf::Vec3d> m_wellPathPoints;
};
