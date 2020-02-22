#pragma once

#include "cafAppEnum.h"
#include "cafFilePath.h"
#include "cafPdmPointer.h"

#include <type_traits>
#include <vector>
#include <assert.h>

#include <QVariant>

namespace caf
{

//==================================================================================================
/// A proxy class that implements the generic QVariant interface for a field
///
/// This class collects methods that need specialization when introducing a new type in a PdmField.
/// Having those methods in a separate class makes it possible to "partially specialize" the methods
/// for container classes etc. since partial specialization of template functions is not C++ as of yet.
///
/// When introducing a new type in a PdmField, you might need to implement a (partial)specialization 
/// of this class.
//==================================================================================================

template <typename T>
class PdmValueFieldSpecialization
{
public:
    /// Convert the field value into a QVariant
    static QVariant convert(const T& value)
    {
        return QVariant::fromValue(value);
    }

    /// Set the field value from a QVariant
    static void setFromVariant(const QVariant& variantValue, T& value)
    {
        value = variantValue.value<T>();
    }

    /// Check equality between QVariants that carries a Field Value. 
    /// The == operator will normally work, but does not support custom types in the QVariant 
    /// See http://qt-project.org/doc/qt-4.8/qvariant.html#operator-eq-eq-64
    /// Using the == between the real types is more safe.
    static bool isEqual(const QVariant& variantValue, const QVariant& variantValue2)
    {
        return variantValue.value<T>() == variantValue2.value<T>();
    }
};


//==================================================================================================
/// Partial specialization for caf::AppEnum
//==================================================================================================
template <typename T>
class PdmValueFieldSpecialization<caf::AppEnum<T> >
{
public:
    static QVariant convert(const caf::AppEnum<T>& value)
    {
        T enumValue = value;
        // Explicit cast to an int before storage in a QVariant. This allows the use of enum class instead of enum
        return QVariant(static_cast<int>(enumValue));
    }

    static void setFromVariant(const QVariant& variantValue, caf::AppEnum<T>& value)
    {
        value = static_cast<T>(variantValue.toInt());
    }
 
    static bool isEqual(const QVariant& variantValue, const QVariant& variantValue2)
    {
        return variantValue == variantValue2;
    }
};


//==================================================================================================
/// Partial specialization for caf::PdmPointer<T>
/// Used internally to avoid havning to declare everything Q_DECLARE_METATYPE()
/// User must use PdmPtrField or PdmChildField
//==================================================================================================
template <typename T>
class PdmValueFieldSpecialization<PdmPointer<T> >
{
public:
    static QVariant convert(const PdmPointer<T>& value)
    {
        return QVariant::fromValue(PdmPointer<PdmObjectHandle>(value.rawPtr()));
    }

    static void setFromVariant(const QVariant& variantValue, caf::PdmPointer<T>& value)
    {
        value.setRawPtr(variantValue.value<PdmPointer<PdmObjectHandle> >().rawPtr());
    }

    static bool isEqual(const QVariant& variantValue, const QVariant& variantValue2)
    {
        return variantValue.value<PdmPointer<PdmObjectHandle> >() == variantValue2.value<PdmPointer<PdmObjectHandle> >() ;
    }
};

//==================================================================================================
/// Partial specialization for std::vector
//==================================================================================================
template <typename T>
class PdmValueFieldSpecialization<std::vector<T> >
{
public:
    static QVariant convert(const std::vector<T>& value)
    {
        QList<QVariant> returnList;
        typename std::vector<T>::const_iterator it;
        for (it = value.begin(); it != value.end() ; ++it)
        {
            returnList.push_back(PdmValueFieldSpecialization<T>::convert(*it));
        }

        return returnList;
    }

    static void setFromVariant(const QVariant& variantValue, std::vector<T>& value)
    {
        if (variantValue.canConvert< QList<QVariant> >())
        {
            value.clear();
            QList<QVariant> lst = variantValue.toList();
            for (int i = 0; i < lst.size(); ++i)
            {
                T val;
                PdmValueFieldSpecialization<T>::setFromVariant(lst[i], val);

                value.push_back(val);
            }
        }
    }

    static bool isEqual(const QVariant& variantValue, const QVariant& variantValue2)
    {
        return variantValue == variantValue2;
    }
};

//==================================================================================================
/// Partial specialization for caf::FilePath
//==================================================================================================
template <>
class PdmValueFieldSpecialization<FilePath>
{
public:
    static QVariant convert(const FilePath& value)
    {
        return QVariant(value.path());
    }

    static void setFromVariant(const QVariant& variantValue, FilePath& value)
    {
        value.setPath(variantValue.toString());
    }
 
    static bool isEqual(const QVariant& variantValue, const QVariant& variantValue2)
    {
        return variantValue.toString() == variantValue2.toString();
    }
};

} // End of namespace caf
