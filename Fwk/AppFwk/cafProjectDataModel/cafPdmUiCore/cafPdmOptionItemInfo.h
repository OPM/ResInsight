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

#include <memory>
#include <type_traits>
#include <vector>

class QIcon;

namespace caf
{
class PdmObjectHandle;

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

    QString                optionUiText() const;
    QVariant               value() const;
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
    if ( fieldValue.metaType().id() == QMetaType::QVariantList )
    {
        QList<QVariant> valuesSelectedInField = fieldValue.toList();

        if ( !valuesSelectedInField.empty() )
        {
            // Create a list to be able to remove items as they are matched with values
            std::list<std::pair<QVariant, unsigned int>> optionVariantAndIndexPairs;

            for ( int i = 0; i < optionList.size(); ++i )
            {
                optionVariantAndIndexPairs.push_back( std::make_pair( optionList[i].value(), i ) );
            }

            for ( const auto& value : valuesSelectedInField )
            {
                std::list<std::pair<QVariant, unsigned int>>::iterator it;
                for ( it = optionVariantAndIndexPairs.begin(); it != optionVariantAndIndexPairs.end(); ++it )
                {
                    if ( PdmUiFieldSpecialization<T>::isDataElementEqual( value, it->first ) )
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
        return ( !foundIndexes.empty() );
    }
}

} // End of namespace caf
