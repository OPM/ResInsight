#include <QtNetwork>
#include <octave/oct.h>
#include <octave/oct-map.h>

#include "riSettings.h"

void getCurrentCase(qint64& caseId, QString& caseName, QString& caseType, qint64& caseGroupId, const QString &hostName, quint16 port)
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

    QString command("GetCurrentCase");
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

    socketStream >> caseId;
    socketStream >> caseName;
    socketStream >> caseType;
    socketStream >> caseGroupId;

    return;
}



DEFUN_DLD (riGetCurrentCase, args, nargout,
           "Usage:\n"
           "\n"
           "   riGetCurrentCase()\n"
           "\n"
           "This function returns a CaseInfo Structure for the Case considered being the 'Current Case' by ResInsight.\n"
           "When ResInsigt loops over a selection of cases and executes an Octave script for each of them,\n"
           "this function returns the CaseInfo for that particular Case.\n"
           )
{
    octave_value retval;
    
    int nargin = args.length ();
    if (nargin > 0)
    {
        error("riGetCurrentCase: Too many input arguments, this function does not take any input arguments.\n");
        print_usage();
    }
    else if (nargout != 1)
    {
        error("riGetCurrentCase: Wrong number of output arguments, this function requires one output argument.\n");
        print_usage();
    }
    else
    {
        qint64  caseId = -1;
        QString caseName;
        QString caseType;
        qint64  caseGroupId = -1;

        getCurrentCase(caseId, caseName, caseType, caseGroupId, "127.0.0.1", 40001);

        octave_map fieldMap;

        fieldMap.assign(riOctavePlugin::caseInfo_CaseId,      Cell(caseId));
        fieldMap.assign(riOctavePlugin::caseInfo_CaseName,    Cell(std::string(caseName.toLatin1().data()), true));
        fieldMap.assign(riOctavePlugin::caseInfo_CaseType,    Cell(std::string(caseType.toLatin1().data()), true));
        fieldMap.assign(riOctavePlugin::caseInfo_CaseGroupId, Cell(caseGroupId));

        retval = octave_value(fieldMap);
    }
    
    return retval;
}

