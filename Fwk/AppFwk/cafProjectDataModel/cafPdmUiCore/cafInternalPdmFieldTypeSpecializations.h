#pragma once

#include "cafPdmObjectHandle.h"
#include "cafPdmPointer.h"

#include <QStringList>

namespace caf
{

template <typename T> class PdmDataValueField;
template <typename T> class PdmPointer;
template <typename T> class AppEnum;

//==================================================================================================
/// Partial specialization for PdmField< PdmPointer<T> >
///
/// Will package the PdmPointer<T> into QVariant as PdmPointer<PdmObject>
/// Needed to support arbitrary types in PdmPointer without 
/// havning to declare everything Q_DECLARE_METATYPE()
/// Also introduces the need for a isEqual() method, as this was the first 
/// custom type embedded in QVariant
//==================================================================================================

template <typename T>
class PdmUiFieldSpecialization < PdmPointer<T> >
{
public:
    static QVariant convert(const PdmPointer<T>& value)
    {
        return QVariant::fromValue(PdmPointer<PdmObjectHandle>(value.rawPtr()));
    }

    static void setFromVariant(const QVariant& variantValue, PdmPointer<T>& value)
    {
        value.setRawPtr(variantValue.value<PdmPointer<PdmObjectHandle> >().rawPtr());
    }

    static bool isEqual(const QVariant& variantValue, const QVariant& variantValue2)
    {
        return variantValue.value<PdmPointer<PdmObjectHandle> >() == variantValue2.value<PdmPointer<PdmObjectHandle> >() ;
    }

    static QList<PdmOptionItemInfo> valueOptions(bool* useOptionsOnly, const PdmPointer<T>&)
    {
        return QList<PdmOptionItemInfo>();
    }
};

//==================================================================================================
/// Partial specialization for PdmField< std::list<T> >
//==================================================================================================

template <typename T>
class PdmUiFieldSpecialization < std::list<T> >
{
public:
    /// Convert the field value into a QVariant
    static QVariant convert(const std::list<T>& value)
    {
        QList<QVariant> returnList;
        typename std::list<T>::const_iterator it;
        for (it = value.begin(); it != value.end() ; ++it)
        {
            returnList.push_back(QVariant(*it));
        }
        return returnList;
    }


    /// Set the field value from a QVariant
    static void setFromVariant(const QVariant& variantValue, std::list<T>& value)
    {
        if (variantValue.canConvert< QList<QVariant> >())
        {
            value.clear();
            QList<QVariant> lst = variantValue.toList();
            int i;
            for (i = 0; i < lst.size(); ++i)
            {
                value.push_back(lst[i].value<T>());
            }
        }
    }

    static bool isEqual(const QVariant& variantValue, const QVariant& variantValue2)
    {
        return variantValue == variantValue2;
    }

    /// Methods to get a list of options for a field, specialized for AppEnum
    static QList<PdmOptionItemInfo> valueOptions( bool* useOptionsOnly, const std::list<T>& )
    {
        return QList<PdmOptionItemInfo>();
    }

    /// Methods to retrieve the possible PdmObject pointed to by a field
    static void childObjects(const PdmDataValueField< std::list<T> >& , std::vector<PdmObjectHandle*>* )    
    { }

};

//==================================================================================================
/// Partial specialization for PdmField< std::vector<T> >
//==================================================================================================

template <typename T>
class PdmUiFieldSpecialization < std::vector<T> >
{
public:
    /// Convert the field value into a QVariant
    static QVariant convert(const std::vector<T>& value)
    {
        QList<QVariant> returnList;
        typename std::vector<T>::const_iterator it;
        for (it = value.begin(); it != value.end() ; ++it)
        {
            returnList.push_back(QVariant(*it));
        }
        return returnList;
    }

    /// Set the field value from a QVariant
    static void setFromVariant(const QVariant& variantValue, std::vector<T>& value)
    {
        if (variantValue.canConvert< QList<QVariant> >())
        {
            value.clear();
            QList<QVariant> lst = variantValue.toList();
            int i;
            for (i = 0; i < lst.size(); ++i)
            {
                value.push_back(lst[i].value<T>());
            }
        }
    }

    static bool isEqual(const QVariant& variantValue, const QVariant& variantValue2)
    {
        return variantValue == variantValue2;
    }

    /// Methods to get a list of options for a field, specialized for AppEnum
    static QList<PdmOptionItemInfo> valueOptions( bool* useOptionsOnly, const  std::vector<T>& )
    {
        return QList<PdmOptionItemInfo>();
    }

    /// Methods to retrieve the possible PdmObject pointed to by a field
    static void childObjects(const PdmDataValueField< std::vector<T> > & field, std::vector<PdmObjectHandle*>* objects)
    { }

};

//==================================================================================================
/// Partial specialization for PdmField<  caf::AppEnum<T> >
///
/// Note :  Makes the setUiValue() and uiValue() interface index based, and NOT based on real enum values.
///         The valueOptions() interface is thus also index based (the value in the PdmOptionItemInfo is index NOT enum)    
///         This is probably going to change, ans it is strange. 
///         This conversion should really be done in the editors we think (now)
//==================================================================================================

#define PDMFIELDAPPENUM_USE_INDEX_BASED_INTERFACE 1

template <typename T>
class PdmUiFieldSpecialization < caf::AppEnum<T> >
{
public:
    /// Convert the field value into a QVariant
    static QVariant convert(const caf::AppEnum<T>& value)
    {
#if PDMFIELDAPPENUM_USE_INDEX_BASED_INTERFACE
        return QVariant(static_cast<unsigned int>(caf::AppEnum<T>::index(value)));
#else
        unsigned int enumVal = value;
        return QVariant(enumVal);
#endif
    }

    /// Set the field value from a QVariant
    static void setFromVariant(const QVariant& variantValue, caf::AppEnum<T>& value)
    {
#if PDMFIELDAPPENUM_USE_INDEX_BASED_INTERFACE
        value.setFromIndex(variantValue.toInt());
#else
        value = static_cast<T> (variantValue.toInt());
#endif
    }

    static bool isEqual(const QVariant& variantValue, const QVariant& variantValue2)
    {
        return variantValue == variantValue2;
    }

    /// Methods to get a list of options for a field, specialized for AppEnum
    static QList<PdmOptionItemInfo> valueOptions( bool* useOptionsOnly, const caf::AppEnum<T>& )
    {
        if (useOptionsOnly) *useOptionsOnly = true;

        QStringList optionTexts = caf::AppEnum<T>::uiTexts();
        QList<PdmOptionItemInfo> optionList;
        int i;
        for (i = 0; i < optionTexts.size(); ++i)
        {
#if PDMFIELDAPPENUM_USE_INDEX_BASED_INTERFACE
            optionList.push_back(PdmOptionItemInfo(optionTexts[i], static_cast<unsigned int>(i)));
#else
            optionList.push_back(PdmOptionItemInfo(optionTexts[i], caf::AppEnum<T>::fromIndex(i)));
#endif
        }

        return optionList;
    }

    /// Methods to retrieve the possible PdmObject pointed to by a field
    static void childObjects(const PdmDataValueField< caf::AppEnum<T> >& field, std::vector<PdmObjectHandle*>* objects)
    { }

};

} // End namespace caf
