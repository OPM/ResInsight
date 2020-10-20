#pragma once

#define CAF_IS_DEFINING_PDM_FIELD
#define PdmDataValueField PdmField
#include "cafPdmDataValueField.h"
#undef PdmDataValueField
#undef CAF_IS_DEFINING_PDM_FIELD

#ifndef __clang__
namespace caf
{
// Specialization to create compiler errors to help finding the PdmField's to rename

#ifdef WIN32

template <typename DataType>
class PdmField<DataType*> : public Rename_PdmField_of_pointer_to_PdmChildField // You must rename PdmField<T*> to
                                                                               // PdmChildField<T*>
{
};

#endif // WIN32

} // namespace caf
#endif // __clang__
