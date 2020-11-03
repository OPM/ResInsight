#include <QtNetwork>
#include <octave/oct.h>
#include <octave/oct-map.h>

#include "riSettings.h"

void getPropertyNames(std::vector<QString>& propNames, std::vector<QString>& propTypes, const QString &hostName, quint16 port, 
                        const qint64& caseId, QString porosityModel)
{
    QString serverName = hostName;
    quint16 serverPort = port;

    QTcpSocket socket;
    socket.connectToHost(serverName, serverPort);

    if (!socket.waitForConnected(riOctavePlugin::connectTimeOutMilliSecs))
    {
        error("Connection: %s",socket.errorString().toLatin1().data());
        return;
    }

    // Create command and send it:

    QString command;
    command += QString("GetPropertyNames") + " " + QString::number(caseId) + " " + porosityModel;
    QByteArray cmdBytes = command.toLatin1();

    QDataStream socketStream(&socket);
    socketStream.setVersion(riOctavePlugin::qtDataStreamVersion);

    socketStream << (qint64)(cmdBytes.size());
    socket.write(cmdBytes);

    // Get response. First wait for the header

    while (socket.bytesAvailable() < (int)(sizeof(quint64)))
    {
        if (!socket.waitForReadyRead(riOctavePlugin::longTimeOutMilliSecs))
        {
            error("Waiting for header: %s",socket.errorString().toLatin1().data());
            return;
        }
    }

    quint64 byteCount;
    socketStream >> byteCount;
    QString byteCountString = QString::number(byteCount);

    //error(byteCountString.toLatin1().data());

    while (socket.bytesAvailable() < (int)(byteCount))
    {
        if (!socket.waitForReadyRead(riOctavePlugin::longTimeOutMilliSecs))
        {
            error("Waiting for data: %s",socket.errorString().toLatin1().data());
            return;
        }
        OCTAVE_QUIT;
    }

    quint64 propCount;
    socketStream >> propCount;

    QString propName;
    QString propType;

    for (size_t i = 0; i < propCount; i++)
    {
        socketStream >> propName;
        socketStream >> propType;

        propNames.push_back(propName);
        propTypes.push_back(propType);
    }

    return;
}



DEFUN_DLD (riGetPropertyNames, args, nargout,
    "Usage:\n"
    "\n"
    "   riGetPropertyNames([CaseId]), [PorosityModel = \"Matrix\"|\"Fracture\"] \n"
    "\n"
    "This function returns the name and type of all the properties in the case as a Vector of Structures.\n"
    "The Structure is defined as: \n"
    "PropertyInfo {\n"
    "    PropName	= string # Name of the property as received from the analysis tool \n"
    "    PropType	= string # The type of the property: \"StaticNative\", \"DynamicNative\", \"Input\", \"Generated\" \n"
    "} \n"
    "If the CaseId is not defined, ResInsight’s Current Case is used.\n"
    )
{
    int nargin = args.length ();
    if (nargin > 2)
    {
        error("riGetPropertyNames: Too many arguments, this function takes two optional arguments.\n");
        print_usage();
    }
    else if (nargout != 1)
    {
        error("riGetPropertyNames: Wrong number of output arguments, this function requires one output argument.\n");
        print_usage();
    }
    else
    {
        qint64 argCaseId = -1;
        QString porosityModel = "Matrix";

        if (nargin == 1)
        {
            if (args(0).is_string())
            {
                porosityModel = args(0).char_matrix_value().row_as_string(0).c_str();
            }
            else
            {
                argCaseId = args(0).uint_value();
            }
        }
        else if (nargin == 2)
        {
            argCaseId = args(0).uint_value();
            porosityModel = args(1).char_matrix_value().row_as_string(0).c_str();
        }

        if (porosityModel != "Matrix" && porosityModel != "Fracture")
        {
            error("riGetPropertyNames: The value for \"PorosityModel\" is unknown. Please use either \"Matrix\" or \"Fracture\"\n");
            print_usage();
            return octave_value_list ();
        }

        std::vector<QString> propertyNames;
        std::vector<QString> propertyTypes;

        getPropertyNames(propertyNames, propertyTypes, "127.0.0.1", 40001, argCaseId, porosityModel);
                
        size_t caseCount = propertyNames.size();

        if (propertyNames.size() != propertyTypes.size() )
        {
            error("riGetPropertyNames: Inconsistent data received from ResInsight.\n");
        }
        else
        {
            // Create cells with N items for each field in the data structure

            Cell cellValuesB(caseCount, 1);
            Cell cellValuesC(caseCount, 1);

            for (size_t i = 0; i < caseCount; i++)
            {
                cellValuesB(i) = propertyNames[i].toLatin1().data();
                cellValuesC(i) = propertyTypes[i].toLatin1().data();
            }

            // Build a map between the field name and field cell values

            octave_map m;

            m.assign(riOctavePlugin::propertyInfo_PropName,    cellValuesB);
            m.assign(riOctavePlugin::propertyInfo_PropType,    cellValuesC);

            return octave_value(m);
        }
    }

    return octave_value();
}

