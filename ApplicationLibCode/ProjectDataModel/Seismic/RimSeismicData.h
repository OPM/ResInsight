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

#include "RiaSeismicDefines.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "cvfVector3.h"

#include <QString>

#include <memory>
#include <utility>

class RimGenericParameter;
class RifSeismicZGYReader;

namespace cvf
{
class BoundingBox;
}

namespace ZGYAccess
{
class SeismicSliceData;
}

class RimSeismicData : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimSeismicData();
    ~RimSeismicData() override;

    void    setFileName( QString filename );
    QString fileName() const;

    QString userDescription();
    void    setUserDescription( QString description );

    void updateMetaData();

    double zMin() const;
    double zMax() const;
    double zStep() const;

    int inlineMin() const;
    int inlineMax() const;
    int inlineStep() const;

    int xlineMin() const;
    int xlineMax() const;
    int xlineStep() const;

    std::vector<double> histogramXvalues() const;
    std::vector<double> histogramYvalues() const;

    std::vector<cvf::Vec3d> worldOutline() const;

    cvf::Vec3d convertToWorldCoords( int iLine, int xLine, double depth );

    std::shared_ptr<ZGYAccess::SeismicSliceData> sliceData( RiaDefines::SeismicSliceDirection direction, int sliceNumber );

    std::pair<double, double> dataRangeMinMax() const;

protected:
    void                 initAfterRead() override;
    caf::PdmFieldHandle* userDescriptionField() override;
    void                 defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    cvf::BoundingBox* boundingBox() const;

private:
    void updateDataRange( bool updatePlot );
    bool openFileIfNotOpen();
    void logError( QString msg );

private:
    caf::PdmField<QString>                        m_filename;
    caf::PdmField<QString>                        m_userDescription;
    caf::PdmChildArrayField<RimGenericParameter*> m_metadata;

    caf::PdmField<bool>   m_overrideDataRange;
    caf::PdmField<double> m_maxAbsDataValue;

    double                            m_zStep;
    cvf::Vec3i                        m_inlineInfo;
    cvf::Vec3i                        m_xlineInfo;
    std::shared_ptr<cvf::BoundingBox> m_boundingBox;
    std::vector<double>               m_histogramXvalues;
    std::vector<double>               m_histogramYvalues;
    std::vector<double>               m_clippedHistogramXvalues;
    std::vector<double>               m_clippedHistogramYvalues;
    std::vector<cvf::Vec3d>           m_worldOutline;
    std::pair<double, double>         m_activeDataRange;
    std::pair<double, double>         m_fileDataRange;

    std::shared_ptr<RifSeismicZGYReader> m_filereader;
    int                                  m_nErrorsLogged;
};
