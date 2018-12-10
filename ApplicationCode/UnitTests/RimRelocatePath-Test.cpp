#include "gtest/gtest.h"

#include "RiaQDateTimeTools.h"
#include "RiaTestDataDirectory.h"
#include "RimProject.h"

#include "cafFilePath.h"
#include "cafPdmObjectHandle.h"

#include <QDateTime>
#include <QString>

#include <vector>



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename T>
void fieldsByType(caf::PdmObjectHandle* object, std::vector<T*>& typedFields)
{
    if (!object) return;

    std::vector<caf::PdmFieldHandle*> allFieldsInObject;
    object->fields(allFieldsInObject);

    std::vector<caf::PdmObjectHandle*> children;

    for (const auto& field : allFieldsInObject)
    {
        caf::PdmField<T>* typedField = dynamic_cast<caf::PdmField<T>*>(field);
        if (typedField) typedFields.push_back(&typedField->v());

        caf::PdmField< std::vector<T> >* typedFieldInVector = dynamic_cast<caf::PdmField< std::vector<T> >*>(field);
        if (typedFieldInVector)
        {
            for (T& typedFieldFromVector : typedFieldInVector->v())
            {
                typedFields.push_back(&typedFieldFromVector);
            }
        }

        field->childObjects(&children);
    }

    for (const auto& child : children)
    {
        fieldsByType(child, typedFields);
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

    std::vector< caf::FilePath* > filePaths;

    fieldsByType(&project, filePaths);

    for (auto filePath : filePaths)
    {
        std::cout << filePath->path().toStdString() << std::endl;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST(RimRelocatePathTest, DISABLED_LocaleDateStringTest)
{
    // Set a non-english system locale on local machine for this test to be useful

    QDateTime dt;
    {
        QDate d(2018, 10, 1);
        dt.setDate(d);
    }

    QString formatString("ddd MMM yyyy");

    // Change the default locale on your system to get a different text then the english formatted text
    QString defaultString = dt.toString(formatString);

    std::cout << "default " << defaultString.toStdString() << std::endl;

    QString englishString = RiaQDateTimeTools::toStringUsingApplicationLocale(dt, formatString);

    std::cout << "english " << englishString.toStdString() << std::endl;
}
