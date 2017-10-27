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
#include "cafPdmPtrField.h"
#include "cafPdmPointer.h"

#include "RifWellRftAddressQMetaType.h"

#include "cafAppEnum.h"

#include <QPointer>
#include <QDate>
#include <QMetaType>

class RimWellLogFile;
class RimEclipseCase;


//==================================================================================================
///  
///  
//==================================================================================================
class RimRftAddress : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimRftAddress();
    RimRftAddress(const RifWellRftAddress& addr);

    void setAddress(const RifWellRftAddress& address);
    RifWellRftAddress address() const;

    RimRftAddress& operator=(const RimRftAddress& other);

private:
    void InitPdmObject();

    caf::PdmField<caf::AppEnum<RifWellRftAddress::SourceType> > m_sourceType;
    caf::PdmPtrField<RimEclipseCase*>                           m_eclCase;
    caf::PdmPtrField<RimWellLogFile*>                           m_wellLogFile;
};
