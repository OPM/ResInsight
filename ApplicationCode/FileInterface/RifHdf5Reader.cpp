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

#include "RiaLogging.h"

#include "H5Cpp.h"
#include "H5Exception.h"

#include "cvfAssert.h"
#include "cvfMath.h"

#include <QStringList>
#include <QDateTime>
#include <QDir>

#include <algorithm>

//--------------------------------------------------------------------------------------------------
/// May reduce constructor content upon discussion of overlying framework. 
///
///std::string dateString = getStringAttribute(file, "/KaseStudy/TransientSections", "initial_date");
///QDateTime initalDateTime = sourSimDateTimeToQDateTime(dateString);   // may rearrange/change to be a call in timeSteps()
//--------------------------------------------------------------------------------------------------
RifHdf5Reader::RifHdf5Reader(const QString& fileName)
    : m_fileName(fileName), m_fileStrategy(0)
{
	try
	{
		H5::H5File file(fileName.toStdString().c_str(), H5F_ACC_RDONLY);     // evt fileName.toLatin1().data()

		m_fileStrategy = getIntAttribute(file, "/", "file_strategy");		 // fileStrategy == 1 means one time step per file

		if (m_fileStrategy == 1)
		{
			m_timeStepFileNames = getSourSimTimeStepFileNames(fileName);
		}
	}

	catch (...)		// catch any failure
	{
	}
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
	if (m_fileStrategy != 1) return false;	// NB: currently incapable of handling all results in one sourres file

	try
	{
		QStringList props = propertyNames();

		QStringList::iterator it = std::find(props.begin(), props.end(), result);

		int propIdx = (it != props.end()) ? it - props.begin() : 0;      // index to 'result' in QStringList props (usually size_t but int gave no warning)
	
		H5::Exception::dontPrint();								     // Turn off auto-printing of failures to handle the errors appropriately

		std::string fileName      = m_timeStepFileNames[stepIndex];       
		std::string timeStepGroup = "/Timestep_" + getTimeStepNumberAs5DigitString(fileName);

		H5::H5File file(fileName.c_str(), H5F_ACC_RDONLY);

		std::string groupName = timeStepGroup + "/GridFunctions/GridPart_00000/GridFunction_" + IntTo5DigitString(propIdx + 1); // adjust to HDF5 one based indexing

		getElementResultValues(file, groupName, values);

		return true;
	}

	catch (...)		// catch any failure
	{
		RiaLogging::error(QString("Failed to read SourSimRL dynamic results"));

		return false;
	}
}

//--------------------------------------------------------------------------------------------------
/// Qt alternative fileName.toLatin1().data()
//--------------------------------------------------------------------------------------------------
std::vector<QDateTime> RifHdf5Reader::timeSteps() const
{
	std::vector<QDateTime> times;

	if (m_fileStrategy != 1) return times;										// NB: currently incapable of handling all results in one sourres file

	try
	{
		H5::H5File mainFile(m_fileName.toStdString().c_str(), H5F_ACC_RDONLY);		// initial date part is an attribute of SourSimRL main file

		std::string dateString = getStringAttribute(mainFile, "/KaseStudy/TransientSections", "initial_date");

		QDateTime dtInitial = sourSimDateTimeToQDateTime(dateString);

        int secondsPerDay = 60 * 60 * 24;

		for (size_t i = 0; i < m_timeStepFileNames.size(); i++)
		{
			std::string fileName      = m_timeStepFileNames[i];
			std::string timeStepGroup = "/Timestep_" + getTimeStepNumberAs5DigitString(fileName);

			H5::H5File file(fileName.c_str(), H5F_ACC_RDONLY);

			double timeStepValue = getDoubleAttribute(file, timeStepGroup, "timestep");				// Assumes only one time step per file
            int timeStepDays = cvf::Math::floor(timeStepValue);
            
            double fractionOfDay = timeStepValue - timeStepDays;
            double milliseconds = fractionOfDay * 24.0 * 60.0 * 60.0 * 1000.0;

			QDateTime dt = dtInitial;
            dt = dt.addDays(timeStepDays);
            dt = dt.addMSecs(milliseconds);

			times.push_back(dt);
		}
	}

	catch (...)		// catch any failure
	{
		times.clear();

		RiaLogging::error(QString("Failed to read SourSimRL time steps"));
	}

	return times;
}


//---------------------------------------------------------------------------------------------------
/// Result variables of first timestep listed by HDF5 subgroups to the following base group: 
/// "/Timestep_00001/GridFunctions/GridPart_00000"
/// According to sourres file description, it seems safe to assume equal presence for all time steps.
//---------------------------------------------------------------------------------------------------
QStringList RifHdf5Reader::propertyNames() const
{
	QStringList propNames;

	if (m_fileStrategy != 1) return propNames;          // NB: currently incapable of handling all results in one sourres file

    if (m_timeStepFileNames.empty())
    {
        RiaLogging::error(QString("Failed to read properties. Transient data does not exist."));
        return propNames;
    }

    std::string fileName  = m_timeStepFileNames[0];      // assume the result variables to be identical across time steps => extract names from first time step file 
    std::string groupName = "/Timestep_" + getTimeStepNumberAs5DigitString(fileName) + "/GridFunctions/GridPart_00000";

	try 
	{
		H5::H5File file(fileName.c_str(), H5F_ACC_RDONLY);

		std::vector<std::string> resultNames = getResultNames(file, groupName);

        for (const std::string& s : resultNames)
		{
			propNames.push_back(s.c_str());
		}
	}

	catch (...)		// catch any failure
	{
		propNames.clear();

		RiaLogging::error(QString("Failed to read properties from file : '%1'").arg(fileName.c_str()));
	}

	return propNames;
}






//=========================== PRIVATE METHODS =====================================================
//
//
//
//==================================================================================================


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RifHdf5Reader::getSourSimTimeStepFileNames(const QString& fileName) const
{
	QFileInfo fi(fileName);
	QString name = fi.fileName();
	QString dir = fi.path();

	QDir baseDir(dir);
	baseDir.setFilter(QDir::Files);


	QStringList nameFilters;
	nameFilters << name + ".?????";
	baseDir.setNameFilters(nameFilters);

	QStringList fileNames = baseDir.entryList();

	std::vector<std::string> timeStepFileNames;

	for (int i = 0; i < fileNames.size(); i++)
	{
		std::string fullPath = dir.toStdString() + "/" + fileNames[i].toStdString();

		timeStepFileNames.push_back(fullPath);
	}

	return timeStepFileNames;
}



//--------------------------------------------------------------------------------------------------
/// Build a QDateTime based on a SourSimRL HDF date attribute
/// Did not succeed with QDateTime dt = QDateTime::fromString(dateString, "YYYY MM DD hh mm ss");
/// Thus a conversion of substrings via integers
//--------------------------------------------------------------------------------------------------
QDateTime RifHdf5Reader::sourSimDateTimeToQDateTime(std::string dateString) const
{
	int year = std::stoi(dateString.substr(0, 4));
	int month = std::stoi(dateString.substr(5, 2));
	int day = std::stoi(dateString.substr(8, 2));

	int hours = std::stoi(dateString.substr(11, 2));
	int minutes = std::stoi(dateString.substr(14, 2));
	int seconds = std::stoi(dateString.substr(17, 2));

	QDate d(year, month, day);
	QTime t(hours, minutes, seconds);

	QDateTime dt;
	dt.setDate(d);
	dt.setTime(t);

	return dt;
}




//--------------------------------------------------------------------------------------------------
/// Build a string based on an int that consists of exactly 5 characters for HDF5 numbering
//--------------------------------------------------------------------------------------------------
std::string RifHdf5Reader::getTimeStepNumberAs5DigitString(std::string fileName) const
{
	return fileName.substr(fileName.size() - 5);  // extract the 5 last characters/digits
}





//--------------------------------------------------------------------------------------------------
/// Build a string based on an int that consists of exactly 5 characters for HDF5 numbering
//--------------------------------------------------------------------------------------------------
std::string RifHdf5Reader::IntTo5DigitString(int i) const
{
	std::string numString = "00000" + std::to_string(i);

	return numString.substr(numString.size() - 5);  // extract the 5 last characters/digits
}



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

	catch (...)
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

	catch (...)
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

	catch (...)
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
/// Intended for finding all timesteps of one SourSimRL result file
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



