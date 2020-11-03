#include <QtNetwork>
#include <octave/oct.h>
#include <octave/oct-map.h>

#include "riSettings.h"

void getCases(std::vector<qint64>& caseIds, std::vector<QString>& caseNames, std::vector<QString>& caseTypes, std::vector<qint64>& caseGroupIds, const qint64& caseGroupId, const QString &hostName, quint16 port)
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

    QString command = QString("GetCases %1").arg(caseGroupId);
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

    while (socket.bytesAvailable() < (int)(byteCount))
    {
        if (!socket.waitForReadyRead(riOctavePlugin::longTimeOutMilliSecs))
        {
            error("Waiting for data: %s",socket.errorString().toLatin1().data());
            return;
        }
        OCTAVE_QUIT;
    }

    quint64 caseCount;
    socketStream >> caseCount;

    qint64  caseId = -1;
    QString caseName;
    QString caseType;
    qint64  caseGroupIdFromSocket = -1;

    for (size_t i = 0; i < caseCount; i++)
    {
        socketStream >> caseId;
        socketStream >> caseName;
        socketStream >> caseType;
        socketStream >> caseGroupIdFromSocket;

        caseIds.push_back(caseId);
        caseNames.push_back(caseName);
        caseTypes.push_back(caseType);
        caseGroupIds.push_back(caseGroupIdFromSocket);
    }

    return;
}



DEFUN_DLD (riGetCases, args, nargout,
           "Usage:\n"
           "\n"
           "   riGetCases([CaseGroupId])\n"
           "\n"
           "This function returns a CaseInfo Structure for all the cases in the current ResInsight project,\n"
           "including the Statistics cases and Source cases in a Grid Case Group.\n"
           "If a CaseGroupId is provided, only the cases in that Case Group will be returned.\n"
           )
{
    int nargin = args.length ();
    if (nargin > 1)
    {
        error("riGetCases: Too many arguments, this function takes one optional argument.\n");
        print_usage();
    }
    else if (nargout != 1)
    {
        error("riGetCases: Wrong number of output arguments, this function requires one output argument.\n");
        print_usage();
    }
    else
    {
        std::vector<qint64>  caseIds;
        std::vector<QString> caseNames;
        std::vector<QString> caseTypes;
        std::vector<qint64>  caseGroupIds;

        qint64 caseGroupId = -1;

        if (nargin == 1)
        {
            unsigned int argCaseId = args(0).uint_value();
            caseGroupId = argCaseId;
        }

        getCases(caseIds, caseNames, caseTypes, caseGroupIds, caseGroupId, "127.0.0.1", 40001);

        size_t caseCount = caseIds.size();

        if (caseCount != caseNames.size() ||
            caseCount != caseTypes.size() ||
            caseCount != caseGroupIds.size())
        {
            error("riGetCases: Inconsistent data received from ResInsight.\n");
        }
        else
        {
            // Create cells with N items for each field in the data structure
            
            Cell cellValuesA(caseCount, 1);
            Cell cellValuesB(caseCount, 1);
            Cell cellValuesC(caseCount, 1);
            Cell cellValuesD(caseCount, 1);

            for (size_t i = 0; i < caseCount; i++)
            {
                cellValuesA(i) = caseIds[i];
                cellValuesB(i) = caseNames[i].toLatin1().data();
                cellValuesC(i) = caseTypes[i].toLatin1().data();
                cellValuesD(i) = caseGroupIds[i];
            }

            // Build a map between the field name and field cell values
            
            octave_map m;

            m.assign(riOctavePlugin::caseInfo_CaseId,      cellValuesA);
            m.assign(riOctavePlugin::caseInfo_CaseName,    cellValuesB);
            m.assign(riOctavePlugin::caseInfo_CaseType,    cellValuesC);
            m.assign(riOctavePlugin::caseInfo_CaseGroupId, cellValuesD);

            return octave_value(m);
        }
    }

    return octave_value();
}

