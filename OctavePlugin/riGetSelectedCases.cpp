#include <QtNetwork>
#include <octave/oct.h>

#include "riSettings.h"

void getSelectedCases(std::vector<qint64>& caseIds, std::vector<QString>& caseNames, std::vector<qint64>& caseTypes, std::vector<qint64>& caseGroupIds, const QString &hostName, quint16 port)
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

    QString command("GetSelectedCases");
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

    quint64 selectionCount;
    socketStream >> selectionCount;

    qint64  caseId = -1;
    QString caseName;
    qint64  caseType = -1;
    qint64  caseGroupId = -1;

    for (size_t i = 0; i < selectionCount; i++)
    {
        socketStream >> caseId;
        socketStream >> caseName;
        socketStream >> caseType;
        socketStream >> caseGroupId;

        caseIds.push_back(caseId);
        caseNames.push_back(caseName);
        caseTypes.push_back(caseType);
        caseGroupIds.push_back(caseGroupId);
    }

    return;
}



DEFUN_DLD (riGetSelectedCases, args, nargout,
           "Usage:\n"
           "\n"
           "   riGetSelectedCases()\n"
           "\n"
           "Returns meta information for the Selected Case(s) in ResInsight.\n"
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
        std::vector<qint64>  caseIds;
        std::vector<QString> caseNames;
        std::vector<qint64>  caseTypes;
        std::vector<qint64>  caseGroupIds;

        getSelectedCases(caseIds, caseNames, caseTypes, caseGroupIds, "127.0.0.1", 40001);

        int caseCount = caseIds.size();

        int maxStringLength = 0;
        for (size_t i = 0; i < caseCount; i++)
        {
            if (caseNames[i].length() > maxStringLength)
            {
                maxStringLength = caseNames[i].length();
            }
        }
        
        dim_vector dv (1, 1);
        dv(0) = caseCount;

        int32NDArray octave_caseIds;
        octave_caseIds.resize(dv);

        int32NDArray octave_caseTypes;
        octave_caseTypes.resize(dv);

        int32NDArray octave_caseGroupIds;
        octave_caseGroupIds.resize(dv);

        charMatrix ch;
        ch.resize(caseCount, maxStringLength);

        for (size_t i = 0; i < caseCount; i++)
        {
            octave_caseIds(i) = caseIds[i];

            ch.insert(caseNames[i].toLatin1().data(), i, 0);

            octave_caseTypes(i) = caseTypes[i];
            octave_caseGroupIds(i) = caseGroupIds[i];
        }

        octave_value_list retval;

        if (nargout >= 1)
        {
            retval(0) = octave_caseIds;
        }

        if (nargout >= 2)
        {
            retval(1) = octave_value (ch, true, '\'');
        }

        if (nargout >= 3)
        {
            retval(2) = octave_caseTypes;
        }

        if (nargout >= 4)
        {
            retval(3) = octave_caseGroupIds;
        }

        return retval;
    }

    return octave_value_list ();
}

