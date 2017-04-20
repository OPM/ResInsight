/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016  Statoil ASA
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

#include "RimSummaryCase.h"

#include "cafPdmProxyValueField.h"

class RimEclipseCase;

//==================================================================================================
//
// 
//
//==================================================================================================

class RimGridSummaryCase: public RimSummaryCase
{
    CAF_PDM_HEADER_INIT;
public:
    RimGridSummaryCase();
    virtual ~RimGridSummaryCase();

    void                            setAssociatedEclipseCase(RimEclipseCase* eclipseCase);
    RimEclipseCase*                 associatedEclipseCase();

    virtual QString                 summaryHeaderFilename() const override;
    virtual QString                 caseName() override;

private:
    QString                         eclipseGridFileName() const;

private:
    caf::PdmPtrField<RimEclipseCase*>   m_eclipseCase;
    mutable caf::PdmField<QString>      m_cachedCaseName;
    caf::PdmProxyValueField<QString>    m_eclipseGridFileName;
    //caf::PdmProxyValueField<QString>    m_eclipseSummaryFileName;
};


