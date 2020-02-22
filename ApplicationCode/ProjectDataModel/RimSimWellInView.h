/////////////////////////////////////////////////////////////////////////////////
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

#pragma once

#include "RigWellDiskData.h"

#include "cafAppEnum.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"

// Include to make Pdm work for cvf::Color
#include "cafPdmFieldCvfColor.h"

#include "cvfObject.h"
#include "cvfVector3.h"

class RigSimWellData;
class RigWellResultFrame;
struct RigWellResultPoint;

class RimSimWellFractureCollection;
class RigWellPath;
class RimWellDiskConfig;

//==================================================================================================
///
///
//==================================================================================================
class RimSimWellInView : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimSimWellInView();
    ~RimSimWellInView() override;

    void                  setSimWellData( RigSimWellData* simWellData, size_t resultWellIndex );
    RigSimWellData*       simWellData();
    const RigSimWellData* simWellData() const;
    size_t                resultWellIndex() const;

    bool isWellCellsVisible() const;
    bool isWellPipeVisible( size_t frameIndex ) const;
    bool isWellSpheresVisible( size_t frameIndex ) const;
    bool isUsingCellCenterForPipe() const;

    RigWellDiskData wellDiskData() const;
    bool            isValidDisk() const;
    double          diskScale() const;

    caf::PdmFieldHandle* userDescriptionField() override;
    caf::PdmFieldHandle* objectToggleField() override;

    std::vector<const RigWellPath*> wellPipeBranches() const;

    void calculateWellPipeStaticCenterLine( std::vector<std::vector<cvf::Vec3d>>&         pipeBranchesCLCoords,
                                            std::vector<std::vector<RigWellResultPoint>>& pipeBranchesCellIds );

    void   wellHeadTopBottomPosition( int frameIndex, cvf::Vec3d* top, cvf::Vec3d* bottom );
    double pipeRadius();
    int    pipeCrossSectionVertexCount();

    void schedule2dIntersectionViewUpdate();

    caf::PdmField<bool> showWell;

    caf::PdmField<QString> name;

    caf::PdmField<bool> showWellLabel;
    caf::PdmField<bool> showWellHead;
    caf::PdmField<bool> showWellPipe;
    caf::PdmField<bool> showWellSpheres;
    caf::PdmField<bool> showWellDisks;

    caf::PdmField<double> wellHeadScaleFactor;
    caf::PdmField<double> pipeScaleFactor;

    caf::PdmField<cvf::Color3f> wellPipeColor;

    caf::PdmField<bool> showWellCells;
    caf::PdmField<bool> showWellCellFence;

    caf::PdmChildField<RimSimWellFractureCollection*> simwellFractureCollection;

    double calculateInjectionProductionFractions( const RimWellDiskConfig& wellDiskConfig, bool* isOk );
    void   scaleDisk( double minValue, double maxValue );

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;

private:
    bool intersectsDynamicWellCellsFilteredCells( size_t frameIndex ) const;
    bool intersectsStaticWellCellsFilteredCells() const;

    bool intersectsWellCellsFilteredCells( const RigWellResultFrame& wrsf, size_t frameIndex ) const;

private:
    cvf::ref<RigSimWellData> m_simWellData;
    size_t                   m_resultWellIndex;
    bool                     m_isInjector;

    RigWellDiskData m_wellDiskData;
    bool            m_isValidDisk;
    double          m_diskScale;
};
