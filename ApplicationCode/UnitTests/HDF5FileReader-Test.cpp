#include "gtest/gtest.h"

#include "H5Cpp.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
/*
TEST(RimWellPathAsciiFileReaderTest, TestWellNameNoColon)
{
QTemporaryFile file;
if (file.open())
{
QString wellName = "My test Wellname";
{
QTextStream out(&file);
out << "name " << wellName << "\n";
out << "1 2 3";
}

RifWellPathAsciiFileReader reader;
RifWellPathAsciiFileReader::WellData wpData = reader.readWellData(file.fileName(), 0);
EXPECT_TRUE(wpData.m_name == wellName);
}
}
*/


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(HDFTests, BasicFileRead)
{
	std::string file_path;

	// Check file exists
	H5::H5File file(file_path, H5F_ACC_RDONLY);
	H5::Group group = H5::Group(file.openGroup("name"));
/*
	Group group = Group(file.openGroup(GROUP_NAME_FLOW_TRANSPORT));
	auto dataset_exists = H5Lexists(group.getId(), "GRIDPROPTIME", H5F_ACC_RDONLY);
	std::vector<std::vector<double>> sgas, soil, swat;
*/
}

