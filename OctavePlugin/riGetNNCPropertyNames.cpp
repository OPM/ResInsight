#include <QtNetwork>
#include <octave/oct.h>
#include <octave/oct-map.h>

#include "riSettings.h"

void getNNCPropertyNames(std::vector<QString>& propNames, std::vector<QString>& propTypes, const QString &hostName, quint16 port, 
                        const qint64& caseId)
{
    QString serverName = hostName;
    quint16 serverPort = port;

    QTcpSocket socket;
    socket.connectToHost(serverName, serverPort);

    if (!socket.waitForConnected(riOctavePlugin::connectTimeOutMilliSecs))
    {
        error((("Connection: ") + socket.errorString()).toLatin1().data());
        return;
    }

    // Create command and send it:

    QString command;
    command += QString("GetNNCPropertyNames") + " " + QString::number(caseId);
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
            error((("Waiting for header: ") + socket.errorString()).toLatin1().data());
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
            error((("Waiting for data: ") + socket.errorString()).toLatin1().data());
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



DEFUN_DLD (riGetNNCPropertyNames, args, nargout,
    "Usage:\n"
    "\n"
    "   riGetNNCPropertyNames([CaseId])\n"
    "\n"
    "This function returns the name and type of all the NNC properties in the case as a Vector of Structures.\n"
    "The Structure is defined as: \n"
    "PropertyInfo {\n"
    "    PropName	= string # Name of the NNC property as received from the analysis tool \n"
    "    PropType	= string # The type of the property: \"StaticNative\", \"DynamicNative\", \"Generated\" \n"
    "} \n"
    "If the CaseId is not defined, ResInsight's Current Case is used.\n"
    )
{
    int nargin = args.length ();
    if (nargin > 1)
    {
        error("riGetNNCPropertyNames: Too many arguments, this function takes one optional argument.\n");
        print_usage();
    }
    else if (nargout != 1)
    {
        error("riGetNNCPropertyNames: Wrong number of output arguments, this function requires one output argument.\n");
        print_usage();
    }
    else
    {
        qint64 argCaseId = -1;

        if (nargin == 1)
        {
            argCaseId = args(0).uint_value();
        }

        std::vector<QString> propertyNames;
        std::vector<QString> propertyTypes;

        getNNCPropertyNames(propertyNames, propertyTypes, "127.0.0.1", 40001, argCaseId);
                
        size_t caseCount = propertyNames.size();

        if (propertyNames.size() != propertyTypes.size() )
        {
            error("riGetNNCPropertyNames: Inconsistent data received from ResInsight.\n");
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

