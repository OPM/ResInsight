#pragma once

#include <QList>
#include <QVariant>

#include <vector>

namespace caf
{
template <typename T>
class PdmDataValueField;
class PdmOptionItemInfo;
class PdmObjectHandle;
class PdmFieldHandle;

//==================================================================================================
/// Base class providing default implementations for PdmUiFieldSpecialization methods.
/// Specializations can inherit from this to avoid repeating empty/simple implementations.
//==================================================================================================
struct PdmUiFieldSpecializationDefaults
{
    static bool isDataElementEqual( const QVariant& variantValue, const QVariant& variantValue2 )
    {
        return variantValue == variantValue2;
    }

    template <typename T>
    static QList<PdmOptionItemInfo> valueOptions( PdmFieldHandle*, const T& )
    {
        return QList<PdmOptionItemInfo>();
    }

    template <typename T>
    static void childObjects( const PdmDataValueField<T>&, std::vector<PdmObjectHandle*>* )
    {
    }
};

//==================================================================================================
/// A proxy class that implements the Gui interface of fields
///
/// This class collects methods that need specialization when introducing a new type in a PdmField.
/// Having those methods in a separate class makes it possible to "partially specialize" the methods
/// for container classes etc. since partial specialization of template functions is not C++ as of yet.
///
/// When introducing a new type in a PdmField, you might need to implement a (partial)specialization
/// of this class.
///
/// The primary template delegates to pdmToVariant/pdmFromVariant/pdmVariantEqual, so types with
/// custom overloads of those functions will automatically get the correct behavior without needing
/// an explicit PdmUiFieldSpecialization.
//==================================================================================================

template <typename T>
class PdmUiFieldSpecialization; // Forward declaration, defined in cafInternalPdmFieldTypeSpecializations.h
} // End of namespace caf

#include "cafInternalPdmFieldTypeSpecializations.h"
