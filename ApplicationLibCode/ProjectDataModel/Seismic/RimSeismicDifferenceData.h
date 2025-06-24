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

#include "RimSeismicDataInterface.h"

#include "RiaSeismicDefines.h"

#include "cafFilePath.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"

class RimGenericParameter;

class RimSeismicDifferenceData : public RimSeismicDataInterface
{
    CAF_PDM_HEADER_INIT;

public:
    RimSeismicDifferenceData();
    ~RimSeismicDifferenceData() override;

    void setInputData( RimSeismicDataInterface* data1, RimSeismicDataInterface* data2 );

    std::string userDescription() const override;
    void        setUserDescription( QString description );

    double zMin() const override;
    double zMax() const override;
    double zStep() const override;

    int inlineMin() const override;
    int inlineMax() const override;
    int inlineStep() const override;

    int xlineMin() const override;
    int xlineMax() const override;
    int xlineStep() const override;

    std::vector<cvf::Vec3d> worldOutline() const override;

    cvf::Vec3d          convertToWorldCoords( int iLine, int xLine, double depth ) override;
    std::pair<int, int> convertToInlineXline( cvf::Vec3d worldCoords ) override;

    std::shared_ptr<ZGYAccess::SeismicSliceData>
        sliceData( RiaDefines::SeismicSliceDirection direction, int sliceNumber, double zMin, double zMax ) override;
    std::shared_ptr<ZGYAccess::SeismicSliceData>
        sliceData( double worldX1, double worldY1, double worldX2, double worldY2, double zMin, double zMax ) override;

    float valueAt( cvf::Vec3d worldCoord ) override;

    std::pair<double, double> sourceDataRangeMinMax() const override;

    QString fullName() const;

    bool hasValidData() const override;

protected:
    void                          initAfterRead() override;
    caf::PdmFieldHandle*          userDescriptionField() override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void                          defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    std::shared_ptr<ZGYAccess::SeismicSliceData> difference( ZGYAccess::SeismicSliceData* data1, ZGYAccess::SeismicSliceData* data2 );

private:
    void updateMetaData();
    bool isInputDataOK() const;
    void generateHistogram();

    caf::PdmField<QString>           m_userDescription;
    caf::PdmProxyValueField<QString> m_nameProxy;

    caf::PdmPtrField<RimSeismicDataInterface*> m_seismicData1;
    caf::PdmPtrField<RimSeismicDataInterface*> m_seismicData2;

    std::pair<double, double> m_fileDataRange;

    bool m_inputDataOK;
};
