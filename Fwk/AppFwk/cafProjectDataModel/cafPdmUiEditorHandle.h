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
#include <vector>
#include <QString>
#include <QObject>
#include "cafPdmUiItem.h"

namespace caf 
{

class PdmUiItem;

//==================================================================================================
/// Abstract class to handle editors 
//==================================================================================================

class PdmUiEditorHandle: public QObject
{
public:
    PdmUiEditorHandle() : m_pdmItem(NULL) {}
    virtual ~PdmUiEditorHandle();

public:
    /// Virtual method to be overridden. Needs to set up the supplied widget
    /// with all signals etc to make it communicate with this object
    
    void        updateUi(const QString& uiConfigName)
    {
        m_currentConfigName = uiConfigName;
        this->configureAndUpdateUi(uiConfigName);
    };

    void        updateUi()
    {
        this->configureAndUpdateUi(m_currentConfigName);
    };

protected: // Interface to override:

    /// Supposed to update all parts of the widgets, both visibility, sensitivity, decorations and field data
    virtual void configureAndUpdateUi(const QString& uiConfigName) = 0;

protected:
    /// This needs to be called from subclass when connecting to a PdmField or Object
    void                bindToPdmItem(PdmUiItem* item);
    PdmUiItem*          pdmItem() { return m_pdmItem; }

private:
    friend PdmUiItem::~PdmUiItem();
    PdmUiItem*          m_pdmItem;
    QString             m_currentConfigName;
};


//==================================================================================================
/// Proxy editor handle used to propagate updates to the editor responsible for the UI for this object
///
/// A tree view control is connected to the root item, and all nodes in the tree will have a proxy editor
/// pointing to the root node editor controlling the UI for the whole tree
//==================================================================================================
class PdmUiProxyEditorHandle: public PdmUiEditorHandle
{
public:
    PdmUiProxyEditorHandle(PdmUiEditorHandle* mainEditorHandle) : PdmUiEditorHandle() { m_mainEditorHandle = mainEditorHandle; }
    virtual ~PdmUiProxyEditorHandle() {};

protected: // Interface to override:

    /// Supposed to update all parts of the widgets, both visibility, sensitivity, decorations and field data
    virtual void configureAndUpdateUi(const QString& uiConfigName)
    {
        if (m_mainEditorHandle)
        {
            m_mainEditorHandle->updateUi(uiConfigName);
        }
    };

private:
    PdmUiEditorHandle* m_mainEditorHandle;
};


} // End of namespace caf

