//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################

#pragma once
#include "cafPdmPointer.h"
#include "cafPdmUiEditorHandle.h"
#include <QPointer>
#include <QString>
#include <QWidget>
#include <vector>

namespace caf
{
class PdmObjectHandle;

//==================================================================================================
///
//==================================================================================================

class PdmUiTreeEditorHandle : public PdmUiEditorHandle
{
public:
    PdmUiTreeEditorHandle() {}
    ~PdmUiTreeEditorHandle() override {}

    QWidget* getOrCreateWidget( QWidget* parent );
    QWidget* widget() { return m_widget; }

    void       setPdmItemRoot( PdmUiItem* root );
    PdmUiItem* pdmItemRoot();
    void       updateSubTree( PdmUiItem* root ) { this->updateMySubTree( root ); }

protected:
    virtual QWidget* createWidget( QWidget* parent ) = 0;

    /// Supposed to update the representation of the tree from root and downwards, as gracefully as possible.
    /// Will be called when the content of root might have been changed
    virtual void updateMySubTree( PdmUiItem* root ) = 0;

protected:
    QPointer<QWidget> m_widget;
};

} // End of namespace caf
