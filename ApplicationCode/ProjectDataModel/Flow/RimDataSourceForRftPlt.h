/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RimViewWindow.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafPdmPtrField.h"

#include "RifDataSourceForRftPltQMetaType.h"

#include "cafAppEnum.h"

#include <QDate>
#include <QMetaType>
#include <QPointer>

class RimObservedFmuRftData;
class RimSummaryCaseCollection;
class RimWellLogFile;
class RimEclipseCase;

//==================================================================================================
///
///
//==================================================================================================
class RimDataSourceForRftPlt : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimDataSourceForRftPlt();
    RimDataSourceForRftPlt( const RifDataSourceForRftPlt& addr );

    void                   setAddress( const RifDataSourceForRftPlt& address );
    RifDataSourceForRftPlt address() const;

    RimDataSourceForRftPlt& operator=( const RimDataSourceForRftPlt& other );

private:
    void InitPdmObject();

    caf::PdmField<caf::AppEnum<RifDataSourceForRftPlt::SourceType>> m_sourceType;
    caf::PdmPtrField<RimEclipseCase*>                               m_eclCase;
    caf::PdmPtrField<RimWellLogFile*>                               m_wellLogFile;
    caf::PdmPtrField<RimSummaryCaseCollection*>                     m_ensemble;
    caf::PdmPtrField<RimObservedFmuRftData*>                        m_observedFmuRftData;
};
