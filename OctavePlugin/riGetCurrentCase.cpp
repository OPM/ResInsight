#include <QtNetwork>
#include <octave/oct.h>

#include "riSettings.h"

void getCurrentCase(qint64& caseId, QString& caseName, qint64& caseType, qint64& caseGroupId, const QString &hostName, quint16 port)
{
    QString serverName = hostName;
    quint16 serverPort = port;

    const int timeout = riOctavePlugin::timeOutMilliSecs;

    QTcpSocket socket;
    socket.connectToHost(serverName, serverPort);

    if (!socket.waitForConnected(timeout))
    {
        error((("Connection: ") + socket.errorString()).toLatin1().data());
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
        if (!socket.waitForReadyRead(timeout))
        {
            error((("Wating for header: ") + socket.errorString()).toLatin1().data());
            return;
        }
    }

    quint64 byteCount;
    socketStream >> byteCount;

    while (socket.bytesAvailable() < (int)(byteCount))
    {
        if (!socket.waitForReadyRead(timeout))
        {
            error((("Waiting for data: ") + socket.errorString()).toLatin1().data());
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
           "Returns meta information for the Case considered to be the “Current Case” by ResInsight.\n"
           "When ResInsigt loops over a selection of cases and executes an Octave script for each of them,\n"
           "this function returns the CaseId for that particular Case."
           )
{
    int nargin = args.length ();
    if (nargin > 0)
    {
        error("riGetCurrentCase: Too many arguments, this function does not take any arguments.\n");
        print_usage();
    }
    else
    {
        qint64  caseId = -1;
        QString caseName;
        qint64  caseType = -1;
        qint64  caseGroupId = -1;

        getCurrentCase(caseId, caseName, caseType, caseGroupId, "127.0.0.1", 40001);

        octave_value_list retval;

        if (nargout >= 1)
        {
            retval(0) = caseId;
        }

        if (nargout >= 2)
        {
            charMatrix ch;
            ch.resize(1, caseName.length());
            ch.insert(caseName.toLatin1().data(), 0, 0);

            retval(1) = octave_value (ch, true, '\'');
        }

        if (nargout >= 3)
        {
            retval(2) = caseType;
        }

        if (nargout >= 4)
        {
            retval(3) = caseGroupId;
        }

        return retval;
    }

    return octave_value_list ();
}

