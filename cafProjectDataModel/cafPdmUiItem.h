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
#include <QString>
#include <QIcon>
#include <QVariant>

namespace caf 
{

//==================================================================================================
/// Class to keep (principally static) gui presentation information 
/// of a data structure item (field or object) used by PdmUiItem
//==================================================================================================

class PdmUiItemInfo
{
public:  
    PdmUiItemInfo() {}

    PdmUiItemInfo( QString  uiName,   QIcon icon = QIcon(), QString  toolTip = "", QString  whatsThis = "")
                          : m_uiName(uiName), m_icon(icon),         m_toolTip(toolTip),    m_whatsThis(whatsThis)
    { }

    QString  m_uiName;
    QString  m_toolTip;
    QString  m_whatsThis;
    QIcon    m_icon;
};

//==================================================================================================
/// Class to keep Ui information about an option /choice in a Combobox or similar.
//==================================================================================================

class PdmOptionItemInfo
{
public:
    PdmOptionItemInfo( QString  anOptionUiText, QVariant aValue = QVariant(), bool anIsDimmed = false, QIcon anIcon = QIcon() )
        :  value(aValue), optionUiText(anOptionUiText), isDimmed(anIsDimmed), icon(anIcon)
    {}

    QString  optionUiText;
    bool     isDimmed;
    QIcon    icon;
    QVariant value;

    // Static utility methods to handle QList of PdmOptionItemInfo
    
    static QStringList extractUiTexts(const QList<PdmOptionItemInfo>& optionList );
    static bool        findValue     (const QList<PdmOptionItemInfo>& optionList , QVariant fieldValue, 
                                      unsigned int* indexToValue = NULL);
};

//==================================================================================================
/// Base class for all datastructure items (fields or objects) to make them have information on 
/// how to display them in the GUI. All the information can have a static variant valid for all 
/// instances of a PDM object, and a dynamic variant that can be changed for a specific instance.
/// the dynamic values overrides the static ones if set.
//==================================================================================================

class PdmUiItem
{
public:
    PdmUiItem() : m_staticItemInfo(NULL), m_isHidden(false)                     { }
    virtual ~PdmUiItem()                                                        { }

    // Copy and assignment to avoid hampering our internal pointer.
    PdmUiItem(const PdmUiItem& ) : m_staticItemInfo(NULL) , m_isHidden(false)   { }
    PdmUiItem& operator=(const PdmUiItem& ) { return *this; }
    
    const QString uiName()      const;
    void          setUiName(const QString& uiName)                              { m_dynamicItemInfo.m_uiName = uiName; } 

    const QIcon   uiIcon()      const;
    void          setUiIcon(const QIcon& uiIcon)                                { m_dynamicItemInfo.m_icon = uiIcon; } 

    const QString uiToolTip()   const;
    void          setUiToolTip(const QString& uiToolTip)                        { m_dynamicItemInfo.m_toolTip = uiToolTip; } 

    const QString uiWhatsThis() const;
    void          setUiWhatsThis(const QString& uiWhatsThis)                    { m_dynamicItemInfo.m_whatsThis = uiWhatsThis; } 

    bool          isHidden() const                                              { return m_isHidden; }
    void          setHidden(bool isHidden)                                      { m_isHidden = isHidden; }

    //==================================================================================================
    /// This method sets the GUI description pointer, which is supposed to be statically allocated 
    /// somewhere. the PdmGuiEntry class will not delete it in any way, and always trust it to be present.
    //==================================================================================================

    void           setUiItemInfo(PdmUiItemInfo* itemInfo) { m_staticItemInfo = itemInfo; }

private:
    PdmUiItemInfo* m_staticItemInfo;
    PdmUiItemInfo  m_dynamicItemInfo;
    bool           m_isHidden;
};



} // End of namespace caf

