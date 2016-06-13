
#include "gtest/gtest.h"

#include "well.hpp"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(WellBaseTest, ReadFromFile)
{
    //std::string wellName = "C:/dev/projects/ResInsight/GitHub/NRWellProject/well_UnitTests/Bean_A.las";

    std::string wellName = "d:/Models/LAS Files/D-D' LAS Files/Bean/Bean_A.las";

    int wellFormat = NRLib::Well::LAS;
    NRLib::Well* well = NRLib::Well::ReadWell(wellName, wellFormat);

    std::cout << "Well name : " << well->GetWellName() << "\n";

    std::cout << "Total number of logs : " << well->GetNlog() << "\n";

    const std::map<std::string, std::vector<double> >& continuousLogs = well->GetContLog();

    std::cout << "Continuous log names : " << "\n";

    std::vector<std::string> names;
    std::map<std::string, std::vector<double> >::const_iterator it;
    for (it = continuousLogs.begin(); it != continuousLogs.end(); it++)
    {
        std::cout << "  " << it->first << "data value count : " << it->second.size() << "\n";
        names.push_back(it->first);
    }

    for (size_t i = 0; i < 20; i++)
    {
        for (size_t n = 0; n < names.size(); n++)
        {
            std::cout << "\t" << continuousLogs.at(names[n])[i];
        }
        std::cout << "\n";
    }
}
