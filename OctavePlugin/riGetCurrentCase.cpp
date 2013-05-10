#include <QtNetwork>
#include <octave/oct.h>

#include "riSettings.h"

void getCurrentCase(int& currentCaseId, const QString &hostName, quint16 port)
{
    QString serverName = hostName;
    quint16 serverPort = port;

    const int Timeout = riOctave::timeOutMilliSecs;

    QTcpSocket socket;
    socket.connectToHost(serverName, serverPort);

    if (!socket.waitForConnected(Timeout))
    {
        error((("Connection: ") + socket.errorString()).toLatin1().data());
        return;
    }

    // Create command and send it:

    QString command("GetCurrentCase");
    QByteArray cmdBytes = command.toLatin1();

    QDataStream socketStream(&socket);
    socketStream.setVersion(QDataStream::Qt_4_0);

    socketStream << (qint64)(cmdBytes.size());
    socket.write(cmdBytes);

    // Get response. First wait for the header

    while (socket.bytesAvailable() < (int)(sizeof(qint64)))
    {
        if (!socket.waitForReadyRead(Timeout))
        {
            error((("Wating for header: ") + socket.errorString()).toLatin1().data());
            return;
        }
    }

    qint64 caseId;
    socketStream >> caseId;

    octave_stdout << "riGetCurrentCase: " << caseId << std::endl;

    currentCaseId = caseId;

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
        int caseId;

        getCurrentCase(caseId, "127.0.0.1", 40001);

        return octave_value(caseId);
    }

    return octave_value_list ();
}

