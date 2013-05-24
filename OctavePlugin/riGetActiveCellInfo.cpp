#include <QtNetwork>
#include <octave/oct.h>

#include "riSettings.h"


void getActiveCellInfo(int32NDArray& activeCellInfo, const QString &hostName, quint16 port, const qint64& caseId, const QString& porosityModel)
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

    QString command = QString("GetActiveCellInfo %1 %2").arg(caseId).arg(porosityModel);
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
            error((("Waiting for header: ") + socket.errorString()).toLatin1().data());
            return;
        }
    }

    // Read timestep count and blocksize

    quint64 columnCount;
    quint64 byteCount;
    size_t  activeCellCount;

    socketStream >> columnCount;
    socketStream >> byteCount;

    activeCellCount = byteCount / sizeof(qint32);

    dim_vector dv (2, 1);
    dv(0) = activeCellCount;
    dv(1) = columnCount;
    activeCellInfo.resize(dv);

    if (!(byteCount && columnCount))
    {
        error ("Could not find the requested data in ResInsight");
        return;
    }

    // Wait for available data for each column, then read data for each column
    for (size_t tIdx = 0; tIdx < columnCount; ++tIdx)
    {
        while (socket.bytesAvailable() < (int)byteCount)
        {
            if (!socket.waitForReadyRead(timeout))
            {
                QString errorMsg = QString("Waiting for column number: %1 of %2: %3").arg(tIdx).arg(columnCount).arg(socket.errorString());

                error(errorMsg.toLatin1().data());
                octave_stdout << "Active cells: " << activeCellCount << ", Columns: " << columnCount << std::endl;
                return ;
            }
           OCTAVE_QUIT;
        }

        qint64 bytesRead = 0;
        qint32* internalMatrixData = (qint32*)activeCellInfo.fortran_vec()->mex_get_data();

#if 1 // Use raw data transfer. Faster.
        bytesRead = socket.read((char*)(internalMatrixData + tIdx * activeCellCount), byteCount);
#else
        for (size_t cIdx = 0; cIdx < activeCellCount; ++cIdx)
        {
            socketStream >> internalMatrixData[tIdx * activeCellCount + cIdx];

            if (socketStream.status() == QDataStream::Ok) bytesRead += sizeof(int);
        }
#endif

        if ((int)byteCount != bytesRead)
        {
            error("Could not read binary double data properly from socket");
            octave_stdout << "Active cells: " << activeCellCount << ", Columns: " << columnCount << std::endl;
        }

        OCTAVE_QUIT;
    }

    QString tmp = QString("riGetActiveCellInfo : Read active cell info");
    if (caseId < 0)
    {
        tmp += QString(" from current case.");
    }
    else
    {
        tmp += QString(" from caseID: %1.").arg(caseId);
    }

    octave_stdout << tmp.toStdString() << " Active cells: " << activeCellCount << ", Columns: " << columnCount << std::endl;

    return;
}



DEFUN_DLD (riGetActiveCellInfo, args, nargout,
           "Usage:\n"
           "\n"
           "   riGetActiveCellInfo([CaseId], [PorosityModel = “Matrix”|”Fracture”] )\n"
           "\n"
           "This function returns a two dimensional matrix containing grid and IJK information\n"
           "for each of the active cells in the requested case.\n"
           "The columns contain the following information:\n"
           "[GridIdx, I, J, K, ParentGridIdx, PI, PJ, PK, CoarseBoxIdx]\n"
           "    GridIdx :       The index of the grid the cell resides in. (Main grid has index 0)\n"
           "    I, J, K :       1-based index address of the cell in the grid.\n"
           "    ParentGridIdx : The index to the grid that this cell's grid is residing in.\n"
           "    PI, PJ, PK :    1-based address of the parent grid cell that this cell is a part of.\n"
           "    CoarseBoxIdx :  Coarsening box index, -1 if none.\n"
           "If the CaseId is not defined, ResInsight’s Current Case is used. If PorosityModel is not defined, “Matrix“ is used.\n"
           )
{
    int nargin = args.length ();
    if (nargin > 2)
    {
        error("riGetActiveCellInfo: Too many arguments. CaseId and PorosityModel are optional input arguments.\n");
        print_usage();
    }
    else if (nargout < 1)
    {
        error("riGetActiveCellInfo: Missing output argument.\n");
        print_usage();
    }
    else
    {
        int32NDArray propertyFrames;

        qint64 caseId = -1;
        QString porosityModel = "Matrix";

        if (nargin > 0)
        {
            if (args(0).is_numeric_type())
            {
                unsigned int argCaseId = args(0).uint_value();
                caseId = argCaseId;
            }
            else
            {
                porosityModel = args(0).char_matrix_value().row_as_string(0).c_str();
            }
        }

        if (nargin > 1)
        {
            if (args(1).is_numeric_type())
            {
                unsigned int argCaseId = args(1).uint_value();
                caseId = argCaseId;
            }
            else
            {
                porosityModel = args(1).char_matrix_value().row_as_string(0).c_str();
            }
        }


        octave_stdout << "Porosity: " << porosityModel.toStdString() << " CaseId : " << caseId << std::endl;

        getActiveCellInfo(propertyFrames, "127.0.0.1", 40001, caseId, porosityModel);

        return octave_value(propertyFrames);
    }

    return octave_value();
}

