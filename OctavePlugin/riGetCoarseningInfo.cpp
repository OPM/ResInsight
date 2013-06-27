#include <QtNetwork>
#include <octave/oct.h>

#include "riSettings.h"


void getCoarseningInfo(int32NDArray& coarseningInfo, const QString &hostName, quint16 port, const qint64& caseId)
{
    QString serverName = hostName;
    quint16 serverPort = port;

    const int Timeout = 5 * 1000;

    QTcpSocket socket;
    socket.connectToHost(serverName, serverPort);

    if (!socket.waitForConnected(Timeout))
    {
        error((("Connection: ") + socket.errorString()).toLatin1().data());
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
        if (!socket.waitForReadyRead(Timeout))
        {
            error((("Waiting for header: ") + socket.errorString()).toLatin1().data());
            return;
        }
    }

    quint64 byteCount;
    socketStream >> byteCount;

    quint64 boxCount = byteCount / (6 * sizeof(qint32));

    dim_vector dv (1, 1);
    dv(0) = 6;
    dv(1) = boxCount;

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

        coarseningInfo(0, i) = i1;
        coarseningInfo(1, i) = i2;
        coarseningInfo(2, i) = j1;
        coarseningInfo(3, i) = j2;
        coarseningInfo(4, i) = k1;
        coarseningInfo(5, i) = k2;
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
            if (args(0).is_numeric_type())
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

