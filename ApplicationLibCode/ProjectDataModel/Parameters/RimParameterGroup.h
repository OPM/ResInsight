/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 -    Equinor ASA
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

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"

#include <QString>
#include <QVariant>
#include <vector>

class RimGenericParameter;
class RimParameterList;

//==================================================================================================
///
///
//==================================================================================================
class RimParameterGroup : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimParameterGroup();
    ~RimParameterGroup() override;

    void addParameter( RimGenericParameter* p );
    void addParameter( QString name, int value );
    void addParameter( QString name, QString value );
    void addParameter( QString name, double value );

    void addList( RimParameterList* paramList );

    void appendParametersToList( std::list<RimGenericParameter*>& parameterList );

    void setName( QString name );
    void setLabel( QString label );
    void setComment( QString comment );
    void setExpanded( bool expand );

    void setParameterValue( QString name, int value );
    void setParameterValue( QString name, QString value );
    void setParameterValue( QString name, double value );

    bool    isExpanded() const;
    QString name() const;
    QString comment() const;
    QString label() const;

    std::vector<RimGenericParameter*> parameters() const;

    RimGenericParameter* parameter( QString name ) const;
    QVariant             parameterValue( QString name ) const;

private:
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    caf::PdmFieldHandle* userDescriptionField() override;
    QString              labelOrName() const;

    bool isListParameter( QString paramName ) const;

private:
    caf::PdmChildArrayField<RimGenericParameter*> m_parameters;
    caf::PdmField<bool>                           m_showExpanded;
    caf::PdmField<QString>                        m_name;
    caf::PdmField<QString>                        m_label;
    caf::PdmField<QString>                        m_comment;
    caf::PdmProxyValueField<QString>              m_labelProxy;
    caf::PdmChildArrayField<RimParameterList*>    m_lists;
};
