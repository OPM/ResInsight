/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "RimWorkflowFieldBinding.h"

#include "cafPdmObjectCollection.h"

class QJsonArray;

class RimWorkflowTaskInput : public caf::PdmObjectCollection<RimWorkflowFieldBinding>
{
    CAF_PDM_HEADER_INIT;

public:
    RimWorkflowTaskInput();

    QString taskName() const;
    void    setTaskName( const QString& name );

    void buildFromSchema( const QJsonArray& configFields );

    QString toTaskYamlBlock() const;

private:
    caf::PdmField<QString> m_taskName;
};
