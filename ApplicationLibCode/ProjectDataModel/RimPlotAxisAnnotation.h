/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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

#include "RiaPlotDefines.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include "cvfColor3.h"

#include <QString>

//==================================================================================================
///
///
//==================================================================================================
class RimPlotAxisAnnotation : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum class AnnotationType
    {
        LINE = 0,
        RANGE
    };
    RimPlotAxisAnnotation();

    void    setName( const QString& name );
    QString name() const;

    void           setValue( double value );
    virtual double value() const;

    AnnotationType annotationType() const;
    void           setAnnotationType( AnnotationType annotationType );

    void         setPenStyle( Qt::PenStyle penStyle );
    Qt::PenStyle penStyle() const;

    void           setColor( const cvf::Color3f& color );
    virtual QColor color() const;

    void                      setAlignment( RiaDefines::TextAlignment alignment );
    RiaDefines::TextAlignment textAlignment() const;

    double rangeStart() const;
    double rangeEnd() const;

    caf::PdmFieldHandle* userDescriptionField() override;
    caf::PdmFieldHandle* objectToggleField() override;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

protected:
    caf::PdmField<bool>                                    m_isActive;
    caf::PdmField<QString>                                 m_name;
    caf::PdmField<double>                                  m_value;
    caf::PdmField<double>                                  m_rangeStart;
    caf::PdmField<double>                                  m_rangeEnd;
    caf::PdmField<caf::AppEnum<Qt::PenStyle>>              m_penStyle;
    caf::PdmField<caf::AppEnum<RiaDefines::TextAlignment>> m_textAlignment;
    caf::PdmField<cvf::Color3f>                            m_color;

    AnnotationType m_annotationType;
};
