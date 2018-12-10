#include "gtest/gtest.h"

#include "RifEclipseOutputFileTools.h"

#include <QString>

#include "ert/ecl/ecl_file.h"
#include "ert/ecl/ecl_kw_magic.h"
#include "ert/ecl/ecl_kw.hpp"



//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST(IntersectDataImport, DISABLED_TestImportPORV)
{
    QString baseFolder = "d:/Models/Statoil/IX_output_files/";
    QString filename   = baseFolder + "NORNE_IX2.INIT";

    std::string porv_kw("PORV");

    ecl_file_type* ecl_file = ecl_file_open(filename.toStdString().data(), ECL_FILE_CLOSE_STREAM);

    bool isIntersect = RifEclipseOutputFileTools::isExportedFromIntersect(ecl_file);
    EXPECT_TRUE(isIntersect);

    if (ecl_file_has_kw(ecl_file, porv_kw.data()))
    {
        ecl_file_load_all(ecl_file);

        int keywordCount = ecl_file_get_num_named_kw(ecl_file, porv_kw.data());
        for (int index = 0; index < keywordCount; index++)
        {
            auto fileKeyword = ecl_file_iget_named_file_kw(ecl_file, porv_kw.data(), index);

            float porvThreshold = 0.0f;
            auto actnumFromPorv = ecl_kw_alloc_actnum(ecl_file_kw_get_kw_ptr(fileKeyword), porvThreshold);

            EXPECT_TRUE(actnumFromPorv != nullptr);
        }
    }
}
