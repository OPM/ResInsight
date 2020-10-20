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

#include <QList>
#include <QString>

#include <vector>

namespace caf
{
class PdmFieldHandle;
class PdmObjectHandle;
class PdmUiEditorHandle;
class PdmUiItem;
class PdmUiTreeItemEditor;
class PdmUiTreeOrdering;

//==================================================================================================
/// Class storing a tree structure representation of some PdmObject hierarchy to be used for tree views in the Gui
//==================================================================================================

class PdmUiTreeOrdering
{
public:
    explicit PdmUiTreeOrdering( PdmObjectHandle* pdmItem );
    explicit PdmUiTreeOrdering( PdmFieldHandle* pdmField );
    PdmUiTreeOrdering( const QString& title, const QString& iconResourceName );

    ~PdmUiTreeOrdering();

    PdmUiTreeOrdering( const PdmUiTreeOrdering& ) = delete;
    PdmUiTreeOrdering& operator=( const PdmUiTreeOrdering& ) = delete;

    void               add( PdmFieldHandle* field, QString uiConfigName = "" );
    void               add( PdmObjectHandle* object );
    PdmUiTreeOrdering* add( const QString& title, const QString& iconResourceName );

    /// If the rest of the fields containing children is supposed to be omitted, set skipRemainingFields to true.
    void skipRemainingChildren( bool doSkip = true ) { m_forgetRemainingFields = doSkip; }
    /// To stop the tree generation at this level, setIgnoreSubTree to true
    void setIgnoreSubTree( bool doIgnoreSubTree ) { m_isToIgnoreSubTree = doIgnoreSubTree; }

    // Testing for the PdmItem being represented
    bool isRepresentingObject() const { return !isRepresentingField() && ( m_object != nullptr ); }
    bool isRepresentingField() const { return ( m_object != nullptr ) && ( m_field != nullptr ); }
    bool isDisplayItemOnly() const { return ( m_uiItem != nullptr ); }
    bool isValid() const { return ( this->activeItem() != nullptr ); }

    // Access to the PdmItem being represented
    PdmUiItem*       activeItem() const;
    PdmObjectHandle* object() const;
    PdmFieldHandle*  field() const;
    PdmUiItem*       uiItem() const;

    // Tree structure traversal access
    PdmUiTreeOrdering* child( int index ) const;
    int                childCount() const;
    PdmUiTreeOrdering* parent() const;
    int                indexInParent() const;

    // Debug helper
    void debugDump( int level ) const;

private:
    friend class PdmObjectHandle;
    friend class PdmUiObjectHandle;
    bool isIncludingRemainingChildren() const { return !m_forgetRemainingFields; }
    bool ignoreSubTree() const { return m_isToIgnoreSubTree; }
    bool containsField( const PdmFieldHandle* field );
    bool containsObject( const PdmObjectHandle* object );
    void appendChild( PdmUiTreeOrdering* child );

    friend class PdmUiTreeViewQModel;
    PdmUiEditorHandle* editor();
    void               setEditor( PdmUiEditorHandle* editor );
    void               insertChild( int position, PdmUiTreeOrdering* child );
    bool               removeChildren( int position, int count );
    bool               removeChildrenNoDelete( int position, int count );

private:
    PdmUiTreeOrdering( PdmUiTreeOrdering* parent, PdmObjectHandle* pdmItem );
    PdmUiTreeOrdering( PdmUiTreeOrdering* parent, PdmFieldHandle* pdmField );

    // Item that we represent
    PdmPointer<PdmObjectHandle> m_object; // We keep both pointer to object and pointer to field when representing a field
    PdmFieldHandle* m_field; // because m_object is guarded while PdmFieldHandle::ownerObject() is not guarded
    PdmUiItem*      m_uiItem;

    // Tree generation control
    bool m_forgetRemainingFields;
    bool m_isToIgnoreSubTree;

    // Editor propagating changes from PdmItem to TreeViewEditor
    PdmUiEditorHandle* m_treeItemEditor;

    // Tree data
    QList<PdmUiTreeOrdering*> m_childItems;
    PdmUiTreeOrdering*        m_parentItem;
};

} // End of namespace caf
