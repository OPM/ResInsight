/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "cafPdmObject.h"

#include "cafPdmField.h"

#include "cvfColor3.h"

#include <QDate>
#include <QString>

class RimWellPath;

//==================================================================================================
///
//==================================================================================================
class RimWellMeasurement : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellMeasurement();
    ~RimWellMeasurement() override;

    QString wellName() const;
    void    setWellName( const QString& wellName );
    double  MD() const;
    void    setMD( double md );
    QDate   date() const;
    void    setDate( const QDate& date );
    double  value() const;
    void    setValue( double value );
    QString kind() const;
    void    setKind( const QString& kind );
    int     quality() const;
    void    setQuality( int quality );
    QString remark() const;
    void    setRemark( const QString& remark );
    QString filePath() const;
    void    setFilePath( const QString& filePath );

    static bool                 kindHasValue( const QString& measurementKind );
    static cvf::Color3f         mapToColor( const QString& measurementKind );
    static std::vector<QString> measurementKindsForWellBoreStability();

private:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    caf::PdmField<QString> m_wellName;
    caf::PdmField<double>  m_MD;
    caf::PdmField<QDate>   m_date;
    caf::PdmField<double>  m_value;
    caf::PdmField<QString> m_kind;
    caf::PdmField<int>     m_quality;
    caf::PdmField<QString> m_remark;
    caf::PdmField<QString> m_filePath;
};
