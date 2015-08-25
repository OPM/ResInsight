#pragma once

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
    static bool isEqual(const QVariant& variantValue, const QVariant& variantValue2)
    {
        return variantValue == variantValue2;
    }

};
} // End of namespace caf

#include "cafAppEnum.h"

namespace caf
{

template <typename T>
class PdmValueFieldSpecialization<caf::AppEnum<T> >
{
public:
    static QVariant convert(const caf::AppEnum<T>& value)
    {
        T enumValue = value;
        return QVariant(enumValue);
    }

    static void setFromVariant(const QVariant& variantValue, caf::AppEnum<T>& value)
    {
        value = (T)variantValue.toInt();
    }
 
    static bool isEqual(const QVariant& variantValue, const QVariant& variantValue2)
    {
        return variantValue == variantValue2;
    }

};

} // End of namespace caf



#include <vector>

namespace caf
{

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
            returnList.push_back(QVariant::fromValue(*it));
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
                value.push_back(lst[i].value<T>());
            }
        }
    }

    static bool isEqual(const QVariant& variantValue, const QVariant& variantValue2)
    {
        return variantValue == variantValue2;
    }

};

} // End of namespace caf
