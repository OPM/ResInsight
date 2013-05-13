#include <QtNetwork>
#include <octave/oct.h>

#include "riSettings.h"

void getCaseGroups(std::vector<QString>& groupNames, std::vector<int>& groupIds, const QString &hostName, quint16 port)
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
    QString command("GetCaseGroups");
    QByteArray cmdBytes = command.toLatin1();

    QDataStream socketStream(&socket);
    socketStream.setVersion(riOctavePlugin::qtDataStreamVersion);

    socketStream << (qint64)(cmdBytes.size());
    socket.write(cmdBytes);

    // Get response. First wait for the header
    while (socket.bytesAvailable() < (int)(2*sizeof(quint64)))
    {
        if (!socket.waitForReadyRead(timeout))
        {
            error((("Waiting for data: ") + socket.errorString()).toLatin1().data());
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
        if (!socket.waitForReadyRead(timeout))
        {
            error((("Waiting for data: ") + socket.errorString()).toLatin1().data());
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
           "Returns a vector of all the case groups in the current ResInsight project"
           )
{
    int nargin = args.length ();
    if (nargin > 0)
    {
        error("riGetCaseGroups: Too many arguments, this function does not take any arguments.\n");
        print_usage();
    }
    else if (nargout == 0 || nargout > 2)
    {
        error("riGetCaseGroups: Wrong number of output arguments, expects one or two output arguments.\n");
        print_usage();
    }
    else
    {
        std::vector<QString> groupNames;
        std::vector<int> groupIds;
        getCaseGroups(groupNames, groupIds, "127.0.0.1", 40001);

        int caseGroupCount = groupNames.size();
        if (caseGroupCount == 0)
        {
             return octave_value_list();
        }

        int maxStringLength = 0;
        for (size_t i = 0; i < caseGroupCount; i++)
        {
            if (groupNames[i].length() > maxStringLength)
            {
                maxStringLength = groupNames[i].length();
            }
        }

        int32NDArray octave_groupIds;
        dim_vector dv (1, 1);
        dv(0) = caseGroupCount;
        octave_groupIds.resize(dv);

        charMatrix ch;
        ch.resize(caseGroupCount, maxStringLength);

        octave_value_list retval;
        for (size_t i = 0; i < caseGroupCount; i++)
        {
            ch.insert(groupNames[i].toLatin1().data(), i, 0);

            octave_groupIds(i) = groupIds[i];
        }

        if (nargout >= 1)
        {
            retval(0) = octave_groupIds;
        }

        if (nargout >= 2)
        {
            retval(1) = octave_value (ch, true, '\'');
        }

        return retval;
    }

    return octave_value_list ();
}

