/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include <QWidget>

namespace caf
{
class PdmUiItem;
class PdmUiTreeView;
class PdmUiPropertyView;
class PdmObjectHandle;
} // namespace caf

//--------------------------------------------------------------------------------------------------
class RiuProjectAndPropertyView : public QWidget
{
public:
    RiuProjectAndPropertyView( QWidget* parent = nullptr, Qt::WindowFlags f = nullptr );

    void setPdmItem( caf::PdmUiItem* object );
    void showProperties( caf::PdmObjectHandle* object );

private:
    caf::PdmUiTreeView*     m_projectTreeView;
    caf::PdmUiPropertyView* m_propertyView;
};