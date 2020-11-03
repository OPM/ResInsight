#include <QtNetwork>
#include <octave/oct.h>
#include <octave/oct-map.h>

#include "riSettings.h"

void getWellNames(std::vector<QString>& wellNames, const QString &hostName, quint16 port, 
                        const qint64& caseId)
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
    command += QString("GetWellNames") + " " + QString::number(caseId);
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

    // QString byteCountString = QString::number(byteCount);
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

    quint64 wellCount;
    socketStream >> wellCount;

    QString wellName;

    for (size_t i = 0; i < wellCount; i++)
    {
        socketStream >> wellName;

        wellNames.push_back(wellName);
    }

    return;
}



DEFUN_DLD (riGetWellNames, args, nargout,
    "Usage:\n"
    "\n"
    "   riGetWellNames([CaseId]) \n"
    "\n"
    "This function returns the names of all the wells in the case as a Vector of strings.\n"
    "If the CaseId is not defined, ResInsight’s Current Case is used.\n"
    )
{
    int nargin = args.length ();
    if (nargin > 1)
    {
        error("riGetWellNames: Too many arguments, this function takes one optional arguments.\n");
        print_usage();
    }
    else if (nargout != 1)
    {
        error("riGetWellNames: Wrong number of output arguments, this function requires one output argument.\n");
        print_usage();
    }
    else
    {
        qint64 argCaseId = -1;

        if (nargin == 1)
        {
            argCaseId = args(0).uint_value();
        }

        std::vector<QString> wellNames;

        getWellNames(wellNames, "127.0.0.1", 40001, argCaseId);
                
        size_t caseCount = wellNames.size();

        // Create cells with N items for each field in the data structure

        string_vector octaveWellNames;
        for (size_t i = 0; i < caseCount; i++)
        {
            octaveWellNames.append(wellNames[i].toStdString());
        }

        return octave_value(Cell(octaveWellNames));
    }

    return octave_value();
}

