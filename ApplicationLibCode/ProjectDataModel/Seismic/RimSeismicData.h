/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

class RimGenericParameter;
class RifSeismicReader;

class RimSeismicData : public RimSeismicDataInterface
{
    CAF_PDM_HEADER_INIT;

public:
    RimSeismicData();
    ~RimSeismicData() override;

    void    setFileName( QString filename );
    QString fileName() const;

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

    void ensureFileReaderIsInitialized();

protected:
    void updateMetaData();

    caf::PdmFieldHandle* userDescriptionField() override;

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    bool openFileIfNotOpen();
    void logError( QString msg );

    int toInlineIndex( int inLine ) const;
    int toXlineIndex( int xLine ) const;
    int toZIndex( double z ) const;

private:
    caf::PdmField<caf::FilePath>                  m_filename;
    caf::PdmField<QString>                        m_userDescription;
    caf::PdmChildArrayField<RimGenericParameter*> m_metadata;

    double                    m_zStep;
    cvf::Vec3i                m_inlineInfo;
    cvf::Vec3i                m_xlineInfo;
    std::vector<cvf::Vec3d>   m_worldOutline;
    std::pair<double, double> m_fileDataRange;

    std::shared_ptr<RifSeismicReader> m_filereader;
    int                               m_nErrorsLogged;
};
