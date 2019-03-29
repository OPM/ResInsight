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

#include "cafPdmUiItem.h"

#include <QObject>
#include <QPointer>

namespace caf 
{

class PdmUiItem;

//==================================================================================================
/// Abstract class to handle editors. Inherits QObject to be able to use signals and slots
//==================================================================================================

class PdmUiEditorHandle : public QObject
{
    Q_OBJECT
public:
    PdmUiEditorHandle();
    ~PdmUiEditorHandle() override;

public:
    void        updateUi(const QString& uiConfigName);;
    void        updateUi();

    void        updateUiIncludingParent();

signals:
    void uiUpdated();

protected:
    // Interface to override:
    /// Virtual method to be overridden. Needs to set up the supplied widget
    /// with all signals etc to make it communicate with this object
    /// Supposed to update all parts of the widgets, both visibility, sensitivity, decorations and field data
    virtual void configureAndUpdateUi(const QString& uiConfigName) = 0;

protected:
    /// This needs to be called from subclass when connecting to a PdmField or Object
    void                bindToPdmItem(PdmUiItem* item);
    PdmUiItem*          pdmItem()       { return m_pdmItem; }
    const PdmUiItem*    pdmItem() const { return m_pdmItem; }
public: // PDM Internal
    void                setParentEditor(PdmUiEditorHandle* parentEditor) { m_parentEditor = parentEditor; }

private:
    friend PdmUiItem::~PdmUiItem();
    PdmUiItem*          m_pdmItem;
    QString             m_currentConfigName;

    QPointer<PdmUiEditorHandle> m_parentEditor; // Editor containing this editor. Will be asked to updateUi (instead of this) if it exists

    bool m_isConfiguringUi; 
};

} // End of namespace caf

