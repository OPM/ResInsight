#pragma once

#include "cafPdmObjectCapability.h"
#include "cafPdmUiItem.h"

class QMenu;

namespace caf
{
class PdmUiEditorAttribute;
class PdmUiTreeOrdering;
class PdmObjectHandle;
class PdmUiOrdering;
class PdmFieldHandle;
class PdmUiEditorAttribute;

class PdmUiObjectHandle : public PdmUiItem, public PdmObjectCapability
{
public:
    PdmUiObjectHandle( PdmObjectHandle* owner, bool giveOwnership );
    ~PdmUiObjectHandle() override {}

    PdmObjectHandle*       objectHandle() { return m_owner; }
    const PdmObjectHandle* objectHandle() const { return m_owner; }

    /// Method to be called from the Ui classes creating Auto Gui to get the group information
    /// supplied by the \sa defineUiOrdering method that can be reimplemented
    void uiOrdering( const QString& uiConfigName, PdmUiOrdering& uiOrdering );

    /// Method to be called by Ui displaying a tree representation of the object hierarchy
    /// Caller must delete the returned object.
    PdmUiTreeOrdering* uiTreeOrdering( const QString& uiConfigName = "" ) const;
    /// Helper function to expand a pre-defined tree start
    static void expandUiTree( PdmUiTreeOrdering* root, const QString& uiConfigName = "" );

    /// For a specific field, return editor specific parameters used to customize the editor behavior.
    void editorAttribute( const PdmFieldHandle* field, const QString& uiConfigName, PdmUiEditorAttribute* attribute );

    /// Return object editor specific parameters used to customize the editor behavior.
    void objectEditorAttribute( const QString& uiConfigName, PdmUiEditorAttribute* attribute );

    /// Field used to control if field change of and object should be covered by undo/redo framework
    virtual bool useUndoRedoForFieldChanged() { return true; }

    void updateUiIconFromToggleField();

    // Virtual interface to override in subclasses to support special behaviour if needed
public: // Virtual
    virtual caf::PdmFieldHandle* userDescriptionField() { return nullptr; }

    /// Field used to toggle object on/off in UI-related uses of the object (ie checkbox in treeview)
    virtual caf::PdmFieldHandle* objectToggleField() { return nullptr; }

    /// Method to reimplement to catch when the field has changed due to setUiValue()
    virtual void
        fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
    {
    }

    /// Method to reimplement to catch when a field in a contained object has changed due to setUiValue()
    virtual void childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField ) {}

    /// Method to re-implement to supply option values for a specific field
    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                 bool*                      useOptionsOnly )
    {
        return QList<PdmOptionItemInfo>();
    }

    /// Override used to attach application defined slots to caf created widgets
    /// All field editor widgets are supposed to be created when this function is called
    virtual void onEditorWidgetsCreated() {}

protected:
    /// Override to customize the order and grouping of the Gui.
    /// Fill up the uiOrdering object with groups and field references to create the gui structure
    /// If the uiOrdering is empty, it is interpreted as meaning all fields w/o grouping.
    virtual void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) {}

    /// Override to customize the tree representations of the object hierarchy.
    /// If the PdmUiTreeOrdering is empty, it is interpreted as meaning all fields containing child objects in order
    virtual void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) {}

    /// Override to provide editor specific data for the field and uiConfigName
    virtual void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                        QString                    uiConfigName,
                                        caf::PdmUiEditorAttribute* attribute )
    {
    }

    /// Override to provide tree editor specific attributes for the field and uiConfigName
    virtual void defineTreeEditorAttribute( const caf::PdmFieldHandle* field,
                                            QString                    uiConfigName,
                                            caf::PdmUiEditorAttribute* attribute )
    {
    }

    /// Override to provide editor specific data for the uiConfigName for the object
    virtual void defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) {}

    /// This method is intended to be used in macros to make compile time errors
    // if user uses them on wrong type of objects
    bool isInheritedFromPdmUiObject() { return true; }

    /// Override used to append actions to a context menu
    /// To use this method, enable custom context menu by
    /// m_myField.uiCapability()->setUseCustomContextMenu(true);
    friend class PdmUiFieldEditorHandle;
    virtual void
        defineCustomContextMenu( const caf::PdmFieldHandle* fieldNeedingMenu, QMenu* menu, QWidget* fieldEditorWidget )
    {
    }

private:
    /// Helper method for the TreeItem generation stuff
    void addDefaultUiTreeChildren( PdmUiTreeOrdering* uiTreeOrdering );

    PdmObjectHandle* m_owner;
};

PdmUiObjectHandle* uiObj( const PdmObjectHandle* obj );

} // End of namespace caf
