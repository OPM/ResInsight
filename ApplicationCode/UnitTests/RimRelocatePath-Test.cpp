#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"
#include "RimProject.h"

#include "cafPdmObjectHandle.h"
#include "cafFilePath.h"

#include <QString>

#include <vector>



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename T>
void fieldByType(caf::PdmObjectHandle* object, std::vector< caf::PdmField<T>* >* typedFields)
{
    if (!typedFields) return;
    if (!object) return;

    std::vector<caf::PdmFieldHandle*> fields;
    object->fields(fields);

    std::vector<caf::PdmObjectHandle*> children;

    for (const auto& field : fields)
    {
        caf::PdmField<T>* typedField = dynamic_cast<caf::PdmField<T>*>(field);
        if (typedField) typedFields->push_back(typedField);

        field->childObjects(&children);
    }

    for (const auto& child : children)
    {
        fieldByType(child, typedFields);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RimRelocatePathTest, findPathsInProjectFile)
{
    QString fileName = QString("%1/RimRelocatePath/RelocatePath.rsp").arg(TEST_DATA_DIR);

    if (fileName.isEmpty()) return;

    RimProject project;

    project.fileName = fileName;
    project.readFile();

    std::vector< caf::PdmField<std::vector<caf::FilePath>>* > filePathsVectors;

    fieldByType(&project, &filePathsVectors);

    for (auto fpVec : filePathsVectors)
    {
        for (auto fp : fpVec->v())
        {
            std::cout << fp.path().toStdString() << std::endl;
        }
    }
}
