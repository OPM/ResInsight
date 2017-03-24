/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
// 
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
// 
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RifHdf5Reader.h"

#include <QStringList>
#include <QDateTime>

#include "H5Cpp.h"
#include <H5Exception.h>



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifHdf5Reader::RifHdf5Reader(const QString& fileName)
    : m_fileName(fileName)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifHdf5Reader::~RifHdf5Reader()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifHdf5Reader::dynamicResult(const QString& result, size_t stepIndex, std::vector<double>* values) const
{
    QStringList myProps = propertyNames();

	try
	{
		H5::Exception::dontPrint();								// Turn off auto-printing of failures to handle the errors appropriately

		std::string fileName = m_fileName.toStdString();		// her ligger trøbbel mht Unicode or det smalt i H5File med direkte bruk av c_str()

		H5::H5File file(fileName.c_str(), H5F_ACC_RDONLY);

		std::string groupName = "/Timestep_00001/GridFunctions/GridPart_00000/GridFunction_00002";

		getElementResultValues(file, groupName, values);

		return true;
	}

	catch (H5::FileIException error)		// catch failure caused by the H5File operations
	{
		return false;
	}

	catch (H5::DataSetIException error)		// catch failure caused by the DataSet operations
	{
		return false;
	}
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<QDateTime> RifHdf5Reader::timeSteps() const
{
    std::vector<QDateTime> times;

    QDateTime dt;
    times.push_back(dt);
    times.push_back(dt.addDays(1));
    times.push_back(dt.addDays(2));

    return times;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList RifHdf5Reader::propertyNames() const
{
    QStringList propNames;

	std::string str = m_fileName.toStdString();   // her ligger trøbbel mht Unicode or det smalt i H5File med direkte bruk av c_str()

	H5::H5File file(str.c_str(), H5F_ACC_RDONLY);

	std::string groupName = "/Timestep_00001/GridFunctions/GridPart_00000";

	std::vector<std::string> resultNames = getResultNames(file, groupName);

	for (std::vector<std::string>::iterator it = resultNames.begin(); it != resultNames.end(); it++)
	{
		propNames.push_back(it->c_str());
	}

    return propNames;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifHdf5Reader::resultNames(QStringList* resultNames, std::vector<size_t>* resultDataItemCounts)
{
    *resultNames = propertyNames();

    for (size_t i = 0; i < propertyNames().size(); i++)
    {
        resultDataItemCounts->push_back(16336);
    }
}





//=========================== PRIVATE METHODS =====================================================
//
//
//
//==================================================================================================


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RifHdf5Reader::getIntAttribute(H5::H5File file, std::string groupName, std::string attributeName) const
{
	try
	{
		H5::Group		group = file.openGroup(groupName.c_str());
		H5::Attribute	attr = group.openAttribute(attributeName.c_str());

		int value = 0;

		H5::DataType type = attr.getDataType();
		attr.read(type, &value);

		return value;
	}

	catch (H5::AttributeIException error)
	{
		return 0;
	}
}




//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RifHdf5Reader::getDoubleAttribute(H5::H5File file, std::string groupName, std::string attributeName) const
{
	try
	{
		H5::Group		group = file.openGroup(groupName.c_str());
		H5::Attribute	attr = group.openAttribute(attributeName.c_str());

		double value = 0.0;

		H5::DataType type = attr.getDataType();
		attr.read(type, &value);

		return value;
	}

	catch (H5::AttributeIException error)
	{
		return 0.0;
	}
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string RifHdf5Reader::getStringAttribute(H5::H5File file, std::string groupName, std::string attributeName) const
{
	try
	{
		H5::Group		group = file.openGroup(groupName.c_str());
		H5::Attribute	attr = group.openAttribute(attributeName.c_str());

		std::string stringAttribute(1024, '\0');

		H5::DataType nameType = attr.getDataType();
		attr.read(nameType, &stringAttribute[0]);

		return stringAttribute;
	}

	catch (H5::AttributeIException error)
	{
		return "";
	}

}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RifHdf5Reader::getSubGroupNames(H5::H5File file, std::string baseGroupName) const
{
	H5::Group baseGroup = file.openGroup(baseGroupName.c_str());

	std::vector<std::string> subGroupNames;

	hsize_t   groupSize = baseGroup.getNumObjs();

	for (hsize_t i = 0; i < groupSize; i++)
	{
		std::string nodeName(1024, '\0');

		ssize_t slen = baseGroup.getObjnameByIdx(i, &nodeName[0], 1023);

		nodeName.resize(slen + 1);

		subGroupNames.push_back(nodeName);
	}

	return subGroupNames;
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RifHdf5Reader::getStepTimeValues(H5::H5File file, std::string baseGroupName) const
{
	std::vector<std::string> subGroupNames = getSubGroupNames(file, baseGroupName);
	std::vector<double>		 stepTimeValues;

	for (std::vector<std::string>::iterator it = subGroupNames.begin(); it != subGroupNames.end(); it++)
	{
		if (it->find("Timestep_") != std::string::npos)
		{
			std::string groupName = baseGroupName + *it;

			double timestep_value = getDoubleAttribute(file, groupName, "timestep");

			stepTimeValues.push_back(timestep_value);
		}
	}

	return stepTimeValues;
}




//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RifHdf5Reader::getResultNames(H5::H5File file, std::string baseGroupName) const
{
	H5::Group baseGroup = file.openGroup(baseGroupName.c_str());

	std::vector<std::string> subGroupNames = getSubGroupNames(file, baseGroupName);

	std::vector<std::string> resultNames;

	for (std::vector<std::string>::iterator it = subGroupNames.begin(); it != subGroupNames.end(); it++)
	{
		std::string groupName = baseGroupName + "/" + *it;

		std::string name = getStringAttribute(file, groupName, "name");

		resultNames.push_back(name);
	}

	return resultNames;
}






//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifHdf5Reader::getElementResultValues(H5::H5File file, std::string groupName, std::vector<double>* resultValues) const
{
	H5::Group		group = file.openGroup(groupName.c_str());
	H5::DataSet		dataset = H5::DataSet(group.openDataSet("values"));

	hsize_t dims[2];
	H5::DataSpace	dataspace = dataset.getSpace();
	dataspace.getSimpleExtentDims(dims, NULL);

	(*resultValues).resize(dims[0]);
	dataset.read(resultValues->data(), H5::PredType::NATIVE_DOUBLE);
}



