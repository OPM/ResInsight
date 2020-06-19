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
#include "cafPdmUiFieldSpecialization.h"

#include <QApplication>
#include <QColor>
#include <QString>
#include <QVariant>
#include <set>
#include <type_traits>

namespace caf
{
//==================================================================================================
/// Class to keep (principally static) gui presentation information
/// of a data structure item (field or object) used by PdmUiItem
//==================================================================================================

class PdmUiItemInfo
{
public:
    enum LabelPosType
    {
        LEFT,
        TOP,
        HIDDEN
    };

    PdmUiItemInfo()
        : m_editorTypeName( "" )
        , m_isHidden( -1 )
        , m_isTreeChildrenHidden( -1 )
        , m_isReadOnly( -1 )
        , m_labelAlignment( LEFT )
        , m_isCustomContextMenuEnabled( -1 )
    {
    }

    PdmUiItemInfo( const QString& uiName,
                   QString        iconResourceLocation = "",
                   QString        toolTip              = "",
                   QString        whatsThis            = "",
                   QString        extraDebugText       = "" );

    PdmUiItemInfo( const QString& uiName,
                   IconProvider   iconProvider   = IconProvider(),
                   QString        toolTip        = "",
                   QString        whatsThis      = "",
                   QString        extraDebugText = "" );

    std::unique_ptr<QIcon> icon() const;
    const IconProvider&    iconProvider() const;

private:
    friend class PdmUiItem;
    QString      m_uiName;
    IconProvider m_iconProvider;
    QColor  m_contentTextColor; ///< Color of a fields value text. Invalid by default. An Invalid color is not used.
    QString m_toolTip;
    QString m_whatsThis;
    QString m_extraDebugText;
    QString m_editorTypeName; ///< Use this exact type of editor to edit this UiItem
    QString m_3dEditorTypeName; ///< If set, use this editor type to edit this UiItem in 3D
    int     m_isHidden; ///< UiItem should be hidden. -1 means not set
    int     m_isTreeChildrenHidden; ///< Children of UiItem should be hidden. -1 means not set
    int     m_isReadOnly; ///< UiItem should be insensitive, or read only. -1 means not set.
    LabelPosType m_labelAlignment;
    int          m_isCustomContextMenuEnabled;
};

//==================================================================================================
/// Class to keep Ui information about an option /choice in a Combobox or similar.
//==================================================================================================

class PdmOptionItemInfo
{
public:
    // Template pass-through for enum types, ensuring the T type gets cast to an int before storing in the QVariant
    // Note the extra dummy parameter. This ensures compilation fails for non-enum types and these variants get removed
    // due to SFINAE (https://en.wikipedia.org/wiki/Substitution_failure_is_not_an_error)
    template <typename T>
    PdmOptionItemInfo( const QString&      anOptionUiText,
                       T                   aValue,
                       bool                isReadOnly                         = false,
                       const IconProvider& anIcon                             = IconProvider(),
                       typename std::enable_if<std::is_enum<T>::value>::type* = 0 )
        : PdmOptionItemInfo( anOptionUiText, QVariant( static_cast<int>( aValue ) ), isReadOnly, anIcon )
    {
    }
    PdmOptionItemInfo( const QString&      anOptionUiText,
                       const QVariant&     aValue,
                       bool                isReadOnly = false,
                       const IconProvider& anIcon     = IconProvider() );
    PdmOptionItemInfo( const QString&        anOptionUiText,
                       caf::PdmObjectHandle* obj,
                       bool                  isReadOnly = false,
                       const IconProvider&   anIcon     = IconProvider() );

    static PdmOptionItemInfo createHeader( const QString&      anOptionUiText,
                                           bool                isReadOnly = false,
                                           const IconProvider& anIcon     = IconProvider() );

    void setLevel( int level );

    const QString          optionUiText() const;
    const QVariant         value() const;
    bool                   isReadOnly() const;
    bool                   isHeading() const;
    std::unique_ptr<QIcon> icon() const;
    int                    level() const;

    // Static utility methods to handle QList of PdmOptionItemInfo
    // Please regard as private to the PDM system

    static QStringList extractUiTexts( const QList<PdmOptionItemInfo>& optionList );
    template <typename T>
    static bool findValues( const QList<PdmOptionItemInfo>& optionList,
                            QVariant                        fieldValue,
                            std::vector<unsigned int>&      foundIndexes );

private:
    QString      m_optionUiText;
    QVariant     m_value;
    bool         m_isReadOnly;
    IconProvider m_iconProvider;
    int          m_level;
};

class PdmUiEditorHandle;

//--------------------------------------------------------------------------------------------------
/// Finds the indexes into the optionList that the field value(s) corresponds to.
/// In the case where the field is some kind of array, several indexes might be returned
/// The returned bool is true if all the fieldValues were found.
//--------------------------------------------------------------------------------------------------
template <typename T>
bool PdmOptionItemInfo::findValues( const QList<PdmOptionItemInfo>& optionList,
                                    QVariant                        fieldValue,
                                    std::vector<unsigned int>&      foundIndexes )
{
    foundIndexes.clear();

    // Find this fieldvalue in the optionlist if present

    // First handle lists/arrays of values
    if ( fieldValue.type() == QVariant::List )
    {
        QList<QVariant> valuesSelectedInField = fieldValue.toList();

        if ( valuesSelectedInField.size() )
        {
            // Create a list to be able to remove items as they are matched with values
            std::list<std::pair<QVariant, unsigned int>> optionVariantAndIndexPairs;

            for ( int i = 0; i < optionList.size(); ++i )
            {
                optionVariantAndIndexPairs.push_back( std::make_pair( optionList[i].value(), i ) );
            }

            for ( int i = 0; i < valuesSelectedInField.size(); ++i )
            {
                std::list<std::pair<QVariant, unsigned int>>::iterator it;
                for ( it = optionVariantAndIndexPairs.begin(); it != optionVariantAndIndexPairs.end(); ++it )
                {
                    if ( PdmUiFieldSpecialization<T>::isDataElementEqual( valuesSelectedInField[i], it->first ) )
                    {
                        foundIndexes.push_back( it->second );

                        // Assuming that one option is referenced only once, the option is erased. Then break
                        // out of the inner loop, as this operation can be costly for fields with many options and many
                        // values

                        optionVariantAndIndexPairs.erase( it );
                        break;
                    }
                }
            }
        }

        return ( static_cast<size_t>( valuesSelectedInField.size() ) <= foundIndexes.size() );
    }
    else // Then handle single value fields
    {
        for ( unsigned int opIdx = 0; opIdx < static_cast<unsigned int>( optionList.size() ); ++opIdx )
        {
            if ( PdmUiFieldSpecialization<T>::isDataElementEqual( optionList[opIdx].value(), fieldValue ) )
            {
                foundIndexes.push_back( opIdx );
                break;
            }
        }
        return ( foundIndexes.size() > 0 );
    }
}

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

    PdmUiItem( const PdmUiItem& ) = delete;
    PdmUiItem& operator=( const PdmUiItem& ) = delete;

    const QString uiName( const QString& uiConfigName = "" ) const;
    void          setUiName( const QString& uiName, const QString& uiConfigName = "" );

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
    void setUiTreeHidden( bool isHidden, const QString& uiConfigName = "" );

    bool isUiTreeChildrenHidden( const QString& uiConfigName = "" ) const;
    void setUiTreeChildrenHidden( bool isTreeChildrenHidden, const QString& uiConfigName = "" );

    bool isUiReadOnly( const QString& uiConfigName = "" ) const;
    void setUiReadOnly( bool isReadOnly, const QString& uiConfigName = "" );

    PdmUiItemInfo::LabelPosType uiLabelPosition( const QString& uiConfigName = "" ) const;
    void setUiLabelPosition( PdmUiItemInfo::LabelPosType alignment, const QString& uiConfigName = "" );

    bool isCustomContextMenuEnabled( const QString& uiConfigName = "" ) const;
    void setCustomContextMenuEnabled( bool enableCustomContextMenu, const QString& uiConfigName = "" );

    QString uiEditorTypeName( const QString& uiConfigName ) const;
    void    setUiEditorTypeName( const QString& editorTypeName, const QString& uiConfigName = "" );

    QString ui3dEditorTypeName( const QString& uiConfigName ) const;
    void    setUi3dEditorTypeName( const QString& editorTypeName, const QString& uiConfigName = "" );

    virtual bool isUiGroup() const;

    /// Intended to be called when fields in an object has been changed
    void updateConnectedEditors() const;

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
    const PdmUiItemInfo* defaultInfo() const;
    const PdmUiItemInfo* configInfo( const QString& uiConfigName ) const;

    PdmUiItemInfo*                   m_staticItemInfo;
    std::map<QString, PdmUiItemInfo> m_configItemInfos;

    static bool sm_showExtraDebugText;
};

} // End of namespace caf
