#include <QtNetwork>
#include <octave/oct.h>
#include <octave/oct-map.h>

#include "riSettings.h"

void getCaseGroups(std::vector<QString>& groupNames, std::vector<int>& groupIds, const QString &hostName, quint16 port)
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
    QString command("GetCaseGroups");
    QByteArray cmdBytes = command.toLatin1();

    QDataStream socketStream(&socket);
    socketStream.setVersion(riOctavePlugin::qtDataStreamVersion);

    socketStream << (qint64)(cmdBytes.size());
    socket.write(cmdBytes);

    // Get response. First wait for the header
    while (socket.bytesAvailable() < (int)(2*sizeof(quint64)))
    {
        if (!socket.waitForReadyRead(riOctavePlugin::longTimeOutMilliSecs))
        {
            error("Waiting for data: %s",socket.errorString().toLatin1().data());
            return;
        }
        OCTAVE_QUIT;
    }

    quint64 byteCount;
    socketStream >> byteCount;

    quint64 groupCount;
    socketStream >> groupCount;

    
    // Get response. Read all data for command
    while (socket.bytesAvailable() < (int)byteCount)
    {
        if (!socket.waitForReadyRead(riOctavePlugin::longTimeOutMilliSecs))
        {
            error("Waiting for data: %s",socket.errorString().toLatin1().data());
            return;
        }
        OCTAVE_QUIT;
    }

    quint64 group = 0;
    while (group < groupCount)
    {
        QString caseGroupName;
        qint64 caseGroupId;

        socketStream >> caseGroupName;
        socketStream >> caseGroupId;

        groupNames.push_back(caseGroupName);
        groupIds.push_back(caseGroupId);

        group++;
    }

    return;
}


DEFUN_DLD (riGetCaseGroups, args, nargout,
           "Usage:\n"
           "\n"
           "   riGetCaseGroups()\n"
           "\n"
           "This function returns a CaseGroupInfo Structure for each of the case groups in the current ResInsight project.\n"
           "CaseGroupInfo = {\n"
           "    CaseGroupId = int # A project-unique integer used to address this particular CaseGroup\n"
           "    CaseGroupName = string # The name assigned to the CaseGroup in ResInsight\n"
           "}\n"
           )
{
    int nargin = args.length ();
    if (nargin > 0)
    {
        error("riGetCaseGroups: Too many arguments, this function does not take any arguments.\n");
        print_usage();
    }
    else if (nargout != 1)
    {
        error("riGetCaseGroups: Wrong number of output arguments, expects one output argument.\n");
        print_usage();
    }
    else
    {
        std::vector<QString> groupNames;
        std::vector<int> groupIds;
        getCaseGroups(groupNames, groupIds, "127.0.0.1", 40001);

        size_t groupCount = groupNames.size();

        if (groupCount != groupIds.size())
        {
            error("riGetCurrentCase: Inconsistent data received from ResInsight.\n");
        }
        else
        {
            // Create cells with N items for each field in the data structure

            Cell cellValuesA(groupCount, 1);
            Cell cellValuesB(groupCount, 1);

            for (size_t i = 0; i < groupCount; i++)
            {
                cellValuesA(i) = groupIds[i];
                cellValuesB(i) = groupNames[i].toLatin1().data();
            }

            // Build a map between the field name and field cell values

            octave_map m;

            m.assign(riOctavePlugin::caseGroupInfo_CaseGroupId,   cellValuesA);
            m.assign(riOctavePlugin::caseGroupInfo_CaseGroupName, cellValuesB);

            return octave_value(m);
        }
    }

    return octave_value();
}

