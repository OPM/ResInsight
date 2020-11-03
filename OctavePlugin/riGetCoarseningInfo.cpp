#include <QtNetwork>
#include <octave/oct.h>

#include "riSettings.h"


void getCoarseningInfo(int32NDArray& coarseningInfo, const QString &hostName, quint16 port, const qint64& caseId)
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

    QString command = QString("GetCoarseningInfo %1").arg(caseId);
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

    quint64 boxCount = byteCount / (6 * sizeof(qint32));

    dim_vector dv (1, 1);
    dv(0) = boxCount;
    dv(1) = 6;

    coarseningInfo.resize(dv);

    for (size_t i = 0; i < boxCount; i++)
    {
        qint32 i1;
        qint32 i2;
        qint32 j1;
        qint32 j2;
        qint32 k1;
        qint32 k2;

        socketStream >> i1;
        socketStream >> i2;
        socketStream >> j1;
        socketStream >> j2;
        socketStream >> k1;
        socketStream >> k2;

        coarseningInfo(i, 0) = i1;
        coarseningInfo(i, 1) = i2;
        coarseningInfo(i, 2) = j1;
        coarseningInfo(i, 3) = j2;
        coarseningInfo(i, 4) = k1;
        coarseningInfo(i, 5) = k2;
    }

    return;
}



DEFUN_DLD (riGetCoarseningInfo, args, nargout,
           "Usage:\n"
           "\n"
           "   riGetCoarseningInfo([CaseId])\n"
           "\n"
           "This function returns all the coarse box definitions used in the grid.\n"
           "The columns contain the following information:\n"
           "[I1, I2, J1, J2, K1, K2]: 1-based index addresses of the min and max corners of the coarsening box.\n"
           "If the CaseId is not defined, ResInsight’s Current Case is used.\n"
           )
{
    int nargin = args.length ();
    if (nargin > 1)
    {
        error("riGetCoarseningInfo: Too many arguments. Only the name or index of the case is valid input.\n");
        print_usage();
    }
    else if (nargout < 1)
    {
        error("riGetCoarseningInfo: Missing output argument.\n");
        print_usage();
    }
    else
    {
        qint64 caseId = -1;
        if (nargin > 0)
        {
            if (riOctavePlugin::isOctaveValueNumeric(args(0)))
            {
                unsigned int argCaseId = args(0).uint_value();
                caseId = argCaseId;
            }
        }

        int32NDArray coarseningInfo;
        getCoarseningInfo(coarseningInfo, "127.0.0.1", 40001, caseId);

        return octave_value(coarseningInfo);
    }

    return octave_value_list ();
}

