/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

class RimColorLegend;
class RimColorLegendItem;

namespace caf
{
class PdmUiEditorAttribute;
}

//==================================================================================================
///
///
//==================================================================================================
class RimColorLegendCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimColorLegendCollection();
    ~RimColorLegendCollection() override;

    void createStandardColorLegends();
    void appendCustomColorLegend( RimColorLegend* customColorLegend );
    bool isStandardColorLegend( RimColorLegend* colorLegend );
    void deleteCustomColorLegends();

    std::vector<RimColorLegend*> allColorLegends() const;

    RimColorLegend* findByName( const QString& name ) const;

protected:
    void initAfterRead() override;

private:
    RimColorLegendItem* createColorLegendItem( const QString& name, int r, int g, int b ) const;
    RimColorLegend*     createRockTypeColorLegend() const;

    caf::PdmChildArrayField<RimColorLegend*> m_standardColorLegends; // ResInsight standard (built-in) legends
    caf::PdmChildArrayField<RimColorLegend*> m_customColorLegends; // user specified legends
};
