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
#include <QString>
#include <QIcon>
#include <QVariant>
#include <set>

namespace caf 
{

//==================================================================================================
/// Class to keep (principally static) gui presentation information 
/// of a data structure item (field or object) used by PdmUiItem
//==================================================================================================

class PdmUiItemInfo
{
public:  
    PdmUiItemInfo()
        : m_editorTypeName(""), m_isHidden(-1), m_isChildrenHidden(-1), m_isReadOnly(-1)
    {}

    PdmUiItemInfo( QString  uiName,   QIcon icon = QIcon(), QString  toolTip = "", QString  whatsThis = "")
        : m_uiName(uiName), m_icon(icon), m_toolTip(toolTip), m_whatsThis(whatsThis),
          m_editorTypeName(""), m_isHidden(false), m_isChildrenHidden(false), m_isReadOnly(false), m_labelAlignment(LEFT)
    { }

    enum LabelPosType { LEFT, TOP, HIDDEN };

private: 
    friend class PdmUiItem;
    QString             m_uiName;
    QString             m_toolTip;
    QString             m_whatsThis;
    QIcon               m_icon;         
    QString             m_editorTypeName;   ///< Use this exact type of editor to edit this UiItem
    int                 m_isHidden;         ///< UiItem should be hidden. -1 means not set
    int                 m_isChildrenHidden; ///< Children of UiItem should be hidden. -1 means not set
    int                 m_isReadOnly;       ///< UiItem should be insensitive, or read only. -1 means not set.
    LabelPosType       m_labelAlignment;
};

//==================================================================================================
/// Class to keep Ui information about an option /choice in a Combobox or similar.
//==================================================================================================

class PdmOptionItemInfo
{
public:
    PdmOptionItemInfo( QString  anOptionUiText, QVariant aValue, bool anIsDimmed = false, QIcon anIcon = QIcon() )
        :  value(aValue), optionUiText(anOptionUiText), isDimmed(anIsDimmed), icon(anIcon)
    {}

    QString  optionUiText;
    bool     isDimmed;
    QIcon    icon;
    QVariant value;

    // Static utility methods to handle QList of PdmOptionItemInfo
    // Please regard as private to the PDM system 
    
    static QStringList extractUiTexts(const QList<PdmOptionItemInfo>& optionList );
    static bool        findValues     (const QList<PdmOptionItemInfo>& optionList , QVariant fieldValue, 
                                      std::vector<unsigned int>& foundIndexes);
};

class PdmUiEditorHandle;

//==================================================================================================
/// Base class for all datastructure items (fields or objects) to make them have information on 
/// how to display them in the GUI. All the information can have a static variant valid for all 
/// instances of a PDM object, and a dynamic variant that can be changed for a specific instance.
/// the dynamic values overrides the static ones if set.
//==================================================================================================

class PdmUiItem
{
public:
    PdmUiItem() : m_staticItemInfo(NULL)                                                   { }
    virtual ~PdmUiItem();

    // Copy and assignment to avoid hampering our internal pointer.
    PdmUiItem(const PdmUiItem& ) : m_staticItemInfo(NULL)                                  { }
    PdmUiItem&       operator=(const PdmUiItem& )                                          { return *this; }
    
    const QString    uiName(QString uiConfigName = "")      const;
    void             setUiName(const QString& uiName, QString uiConfigName = "")           { m_configItemInfos[uiConfigName].m_uiName = uiName; } 

    const QIcon      uiIcon(QString uiConfigName = "")      const;
    void             setUiIcon(const QIcon& uiIcon, QString uiConfigName = "")             { m_configItemInfos[uiConfigName].m_icon = uiIcon; } 

    const QString    uiToolTip(QString uiConfigName = "")   const;
    void             setUiToolTip(const QString& uiToolTip, QString uiConfigName = "")     { m_configItemInfos[uiConfigName].m_toolTip = uiToolTip; } 

    const QString    uiWhatsThis(QString uiConfigName = "") const;
    void             setUiWhatsThis(const QString& uiWhatsThis, QString uiConfigName = "") { m_configItemInfos[uiConfigName].m_whatsThis = uiWhatsThis; } 

    bool             isUiHidden(QString uiConfigName = "") const;
    void             setUiHidden(bool isHidden, QString uiConfigName = "")                 { m_configItemInfos[uiConfigName].m_isHidden = isHidden; } 

    bool             isUiChildrenHidden(QString uiConfigName = "") const;
    void             setUiChildrenHidden(bool isChildrenHidden, QString uiConfigName = "")     { m_configItemInfos[uiConfigName].m_isChildrenHidden = isChildrenHidden; } 

    bool             isUiReadOnly(QString uiConfigName = "");
    void             setUiReadOnly(bool isReadOnly, QString uiConfigName = "")             { m_configItemInfos[uiConfigName].m_isReadOnly = isReadOnly; } 
   
    PdmUiItemInfo::LabelPosType  
                     uiLabelPosition(QString uiConfigName = "") const;
    void             setUiLabelPosition(PdmUiItemInfo::LabelPosType alignment, QString uiConfigName = "") { m_configItemInfos[uiConfigName].m_labelAlignment = alignment; } 

    QString          uiEditorTypeName(const QString& uiConfigName) const;
    void             setUiEditorTypeName(const QString& editorTypeName, QString uiConfigName = "") { m_configItemInfos[uiConfigName].m_editorTypeName = editorTypeName; }

    virtual bool     isUiGroup()                                                           { return false; }

    void             updateConnectedEditors();

    void             updateUiIconFromState(bool active);

public: // Pdm-Private only
    //==================================================================================================
    /// This method sets the GUI description pointer, which is supposed to be statically allocated 
    /// somewhere. the PdmGuiEntry class will not delete it in any way, and always trust it to be present.
    /// Consider as PRIVATE to the PdmSystem
    //==================================================================================================

    void              setUiItemInfo(PdmUiItemInfo* itemInfo)            { m_staticItemInfo = itemInfo; }

    void              removeFieldEditor(PdmUiEditorHandle* fieldView)   { m_editors.erase(fieldView); }
    void              addFieldEditor(PdmUiEditorHandle* fieldView)      { m_editors.insert(fieldView); }

protected:
    std::set<PdmUiEditorHandle*>        m_editors;

private:
    const PdmUiItemInfo*                defaultInfo() const;
    const PdmUiItemInfo*                configInfo(QString uiConfigName) const;

    PdmUiItemInfo*                      m_staticItemInfo;
    std::map< QString, PdmUiItemInfo >  m_configItemInfos; 
};



} // End of namespace caf

