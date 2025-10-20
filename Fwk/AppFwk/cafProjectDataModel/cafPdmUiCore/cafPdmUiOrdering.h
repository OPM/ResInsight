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

#include <QString>

#include <functional>
#include <memory>
#include <vector>

namespace caf
{
class PdmUiGroup;
class PdmUiLabel;
class PdmUiButton;
class PdmFieldHandle;
class PdmObjectHandle;

//==================================================================================================
/// Class storing the order and grouping of fields and groups of fields etc. to be used in the Gui
//==================================================================================================

class PdmUiOrdering
{
public:
    static const int MAX_COLUMN_SPAN = -1;

    struct LayoutOptions
    {
        bool newRow{ true };
        int  totalColumnSpan{ MAX_COLUMN_SPAN };
        int  leftLabelColumnSpan{ MAX_COLUMN_SPAN };
    };

    // Required to use a static function as workaround instead of using LayoutOptions()
    // https://stackoverflow.com/questions/53408962/try-to-understand-compiler-error-message-default-member-initializer-required-be
    static LayoutOptions defaultLayoutOptions() { return {}; }

    using FieldAndLayout = std::pair<PdmUiItem*, LayoutOptions>;
    using RowLayout      = std::vector<FieldAndLayout>;
    using TableLayout    = std::vector<RowLayout>;

    PdmUiOrdering();
    virtual ~PdmUiOrdering();

    PdmUiOrdering( const PdmUiOrdering& )            = delete;
    PdmUiOrdering& operator=( const PdmUiOrdering& ) = delete;

    void add( const PdmFieldHandle* field, LayoutOptions layout = defaultLayoutOptions() );

    // Add a field without creating a new row
    void appendToRow( const PdmFieldHandle* field );

    void add( const PdmObjectHandle* obj, LayoutOptions layout = defaultLayoutOptions() );
    bool insertBeforeGroup( const QString&        groupId,
                            const PdmFieldHandle* fieldToInsert,
                            LayoutOptions         layout = defaultLayoutOptions() );
    bool insertBeforeItem( const PdmUiItem*      item,
                           const PdmFieldHandle* fieldToInsert,
                           LayoutOptions         layout = defaultLayoutOptions() );

    PdmUiGroup*  addNewGroup( const QString& displayName, LayoutOptions layout = defaultLayoutOptions() );
    PdmUiLabel*  addNewLabel( const QString& labelText, LayoutOptions layout = defaultLayoutOptions() );
    PdmUiButton* addNewButton( const QString&               buttonText,
                               const std::function<void()>& callback,
                               LayoutOptions                layout = defaultLayoutOptions() );

    PdmUiGroup* createGroupBeforeGroup( const QString& groupId,
                                        const QString& displayName,
                                        LayoutOptions  layout = defaultLayoutOptions() );

    PdmUiGroup* createGroupBeforeItem( const PdmUiItem* item,
                                       const QString&   displayName,
                                       LayoutOptions    layout = defaultLayoutOptions() );

    PdmUiGroup* addNewGroupWithKeyword( const QString& displayName,
                                        const QString& groupKeyword,
                                        LayoutOptions  layout = defaultLayoutOptions() );

    PdmUiGroup* createGroupWithIdBeforeGroup( const QString& groupId,
                                              const QString& displayName,
                                              const QString& newGroupId,
                                              LayoutOptions  layout = defaultLayoutOptions() );

    PdmUiGroup* createGroupWithIdBeforeItem( const PdmUiItem* item,
                                             const QString&   displayName,
                                             const QString&   newGroupId,
                                             LayoutOptions    layout = defaultLayoutOptions() );

    PdmUiGroup* findGroup( const QString& groupId ) const;

    void skipRemainingFields( bool doSkip = true );

    // Pdm internal methods

    const std::vector<PdmUiItem*>      uiItems() const;
    const std::vector<FieldAndLayout>& uiItemsWithLayout() const;

    std::vector<std::vector<FieldAndLayout>> calculateTableLayout( const QString& uiConfigName ) const;
    int                                      nrOfColumns( const TableLayout& tableLayout ) const;
    int                                      nrOfRequiredColumnsInRow( const RowLayout& rowItems ) const;
    int                                      nrOfExpandingItemsInRow( const RowLayout& rowItems ) const;
    void                                     nrOfColumnsRequiredForItem( const FieldAndLayout& fieldAndLayout,
                                                                         int*                  totalColumnsRequired,
                                                                         int*                  labelColumnsRequired,
                                                                         int*                  fieldColumnsRequired ) const;
    bool                                     contains( const PdmUiItem* item ) const;
    bool                                     isIncludingRemainingFields() const;

protected:
    struct PositionFound
    {
        PdmUiOrdering* parent;
        size_t         indexInParent;
        PdmUiItem*     item();
        PdmUiGroup*    group();
    };

    PositionFound findGroupPosition( const QString& groupKeyword ) const;
    PositionFound findItemPosition( const PdmUiItem* item ) const;

private:
    void        insert( size_t index, const PdmFieldHandle* field, LayoutOptions layout = defaultLayoutOptions() );
    PdmUiGroup* insertNewGroupWithKeyword( size_t         index,
                                           const QString& displayName,
                                           const QString& groupKeyword,
                                           LayoutOptions  layout = defaultLayoutOptions() );

    std::vector<FieldAndLayout>               m_ordering;
    std::vector<std::unique_ptr<PdmUiGroup>>  m_createdGroups;
    std::vector<std::unique_ptr<PdmUiLabel>>  m_createdLabels;
    std::vector<std::unique_ptr<PdmUiButton>> m_createdButtons;
    bool                                      m_skipRemainingFields;
};

} // End of namespace caf

// Included after PdmUiOrdering, as the group derives from PdmUiOrdering
#include "cafPdmUiGroup.h"
