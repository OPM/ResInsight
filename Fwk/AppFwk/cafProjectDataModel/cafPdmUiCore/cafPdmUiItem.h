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

#include "cafIconProvider.h"
#include "cafPdmOptionItemInfo.h"
#include "cafPdmUiItemInfo.h"

#include <list>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <type_traits>

#include <QVariant>

namespace caf
{
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
    PdmUiItem();
    virtual ~PdmUiItem();

    PdmUiItem( const PdmUiItem& )            = delete;
    PdmUiItem& operator=( const PdmUiItem& ) = delete;

    const QString  uiName( const QString& uiConfigName = "" ) const;
    void           setUiName( const QString& uiName, const QString& uiConfigName = "" );
    static QString uiConfigNameForStaticData();

    std::unique_ptr<QIcon> uiIcon( const QString& uiConfigName = "" ) const;
    const IconProvider     uiIconProvider( const QString& uiConfigName = "" ) const;
    void                   setUiIcon( const IconProvider& uiIcon, const QString& uiConfigName = "" );
    void setUiIconFromResourceString( const QString& uiIconResourceName, const QString& uiConfigName = "" );

    const QColor uiContentTextColor( const QString& uiConfigName = "" ) const;
    void         setUiContentTextColor( const QColor& uiIcon, const QString& uiConfigName = "" );

    const QString uiToolTip( const QString& uiConfigName = "" ) const;
    void          setUiToolTip( const QString& uiToolTip, const QString& uiConfigName = "" );

    const QString uiWhatsThis( const QString& uiConfigName = "" ) const;
    void          setUiWhatsThis( const QString& uiWhatsThis, const QString& uiConfigName = "" );

    bool isUiHidden( const QString& uiConfigName = "" ) const;
    void setUiHidden( bool isHidden, const QString& uiConfigName = "" );

    bool isUiTreeHidden( const QString& uiConfigName = "" ) const;
    void setUiTreeHidden( bool isTreeHidden, const QString& uiConfigName = "" );

    bool isUiTreeChildrenHidden( const QString& uiConfigName = "" ) const;
    void setUiTreeChildrenHidden( bool isTreeChildrenHidden, const QString& uiConfigName = "" );

    bool isUiReadOnly( const QString& uiConfigName = "" ) const;
    void setUiReadOnly( bool isReadOnly, const QString& uiConfigName = "" );

    bool notifyAllFieldsInMultiFieldChangedEvents( const QString& uiConfigName = "" ) const;
    void setNotifyAllFieldsInMultiFieldChangedEvents( bool enable, const QString& uiConfigName = "" );

    PdmUiItemInfo::LabelPosition uiLabelPosition( const QString& uiConfigName = "" ) const;
    void setUiLabelPosition( PdmUiItemInfo::LabelPosition position, const QString& uiConfigName = "" );

    bool isCustomContextMenuEnabled( const QString& uiConfigName = "" ) const;
    void setCustomContextMenuEnabled( bool enableCustomContextMenu, const QString& uiConfigName = "" );

    QString uiEditorTypeName( const QString& uiConfigName ) const;
    void    setUiEditorTypeName( const QString& editorTypeName, const QString& uiConfigName = "" );

    QString ui3dEditorTypeName( const QString& uiConfigName ) const;
    void    setUi3dEditorTypeName( const QString& editorTypeName, const QString& uiConfigName = "" );

    virtual bool isUiGroup() const;

    // Map-based attribute system

    /// Sets an attribute value for this UI item.
    ///
    /// Attributes provide a flexible way to configure UI editor behavior without
    /// modifying the editor classes themselves. Each attribute is stored as a key-value
    /// pair and can be retrieved later by editors during configuration.
    ///
    /// String Type Handling:
    /// ---------------------
    /// The method automatically converts C-style strings to QString for Qt integration:
    /// - String literals: "text" or char[N] arrays
    /// - String pointers: const char* or char*
    /// All other types are stored directly in QVariant.
    ///
    /// Usage Examples:
    /// ---------------
    /// @code
    ///   // String literal (automatically converted to QString)
    ///   field.uiCapability()->setAttribute("dateFormat", "yyyy-MM-dd");
    ///
    ///   // Lambda/function (stored in QVariant)
    ///   field.uiCapability()->setAttribute("callback", myCallbackFunction);
    ///
    ///   // Numeric value (stored in QVariant)
    ///   field.uiCapability()->setAttribute("maxLength", 100);
    ///
    /// @endcode
    template <typename T>
    void setAttribute( const QString& key, const T& value, const QString& uiConfigName = "" )
    {
        // Decay type removes references, const, and volatile qualifiers
        // This helps us identify the true underlying type
        using DecayedT = std::decay_t<T>;

        // Check if value is a C-style string pointer (const char* or char*)
        constexpr bool isCStringPointer = std::is_same_v<DecayedT, const char*> || std::is_same_v<DecayedT, char*>;

        // Check if value is a character array (string literal like "text" or char[75])
        constexpr bool isCharArray = std::is_array_v<T> && std::is_same_v<std::remove_extent_t<T>, char>;

        if constexpr ( isCStringPointer || isCharArray )
        {
            // Convert C-style strings to QString for proper Qt string handling
            m_attributeMaps[uiConfigName][key] = QString( value );
        }
        else
        {
            // Store all other types (int, double, function pointers, etc.) in QVariant
            // QVariant handles type-safe storage and retrieval of various types
            m_attributeMaps[uiConfigName][key] = QVariant::fromValue( value );
        }
    }

    template <typename T>
    std::optional<T> attribute( const QString& key, const QString& uiConfigName = "" ) const
    {
        QVariant value = attributeVariant( key, uiConfigName );
        if ( value.isValid() && value.canConvert<T>() )
        {
            return value.value<T>();
        }
        return std::nullopt;
    }

    std::list<QString> attributeNames( const QString& uiConfigName = "" ) const;

    // Helper function to validate attributes against a supported set
    void validateAttributes( const QString&           contextName,
                             const std::set<QString>& supportedAttributes,
                             const QString&           uiConfigName = "" ) const;

    /// Intended to be called when fields in an object has been changed
    void updateConnectedEditors() const;
    void scheduleUpdateConnectedEditors() const;

    /// Intended to be called when an object has been created or deleted
    void updateAllRequiredEditors() const;

    void updateUiIconFromState( bool isActive, const QString& uiConfigName = "" );

    std::vector<PdmUiEditorHandle*> connectedEditors() const;

    bool hasEditor( PdmUiEditorHandle* editor ) const;

    static bool showExtraDebugText();
    static void enableExtraDebugText( bool enable );

public: // Pdm-Private only
    //==================================================================================================
    /// This method sets the GUI description pointer, which is supposed to be statically allocated
    /// somewhere. the PdmGuiEntry class will not delete it in any way, and always trust it to be present.
    /// Consider as PRIVATE to the PdmSystem
    //==================================================================================================

    void setUiItemInfo( PdmUiItemInfo* itemInfo );

    void removeFieldEditor( PdmUiEditorHandle* fieldView );
    void addFieldEditor( PdmUiEditorHandle* fieldView );

protected:
    std::set<PdmUiEditorHandle*> m_editors;

private:
    QVariant attributeVariant( const QString& key, const QString& uiConfigName = "" ) const;

private:
    const PdmUiItemInfo* defaultInfo() const;
    const PdmUiItemInfo* configInfo( const QString& uiConfigName ) const;

    PdmUiItemInfo*                   m_staticItemInfo;
    std::map<QString, PdmUiItemInfo> m_configItemInfos;

    // Map-based attributes: uiConfigName -> (attributeKey -> value)
    std::map<QString, std::map<QString, QVariant>> m_attributeMaps;

    static bool sm_showExtraDebugText;
};

} // End of namespace caf
