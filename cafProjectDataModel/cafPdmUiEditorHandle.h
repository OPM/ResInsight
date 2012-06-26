//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
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



} // End of namespace caf

