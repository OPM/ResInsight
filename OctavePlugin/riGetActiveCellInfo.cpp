#include <QtNetwork>
#include <QStringList>

#include <octave/oct.h>
#include "riSettings.h"
#include "RiaSocketDataTransfer.cpp"  // NB! Include cpp-file to avoid linking of additional file in oct-compile configuration

void getActiveCellInfo(int32NDArray& activeCellInfo, const QString &hostName, quint16 port, const qint64& caseId, const QString& porosityModel)
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

    QString command = QString("GetActiveCellInfo %1 %2").arg(caseId).arg(porosityModel);
    QByteArray cmdBytes = command.toLatin1();

    QDataStream socketStream(&socket);
    socketStream.setVersion(riOctavePlugin::qtDataStreamVersion);

    socketStream << (qint64)(cmdBytes.size());
    socket.write(cmdBytes);

    // Get response. First wait for the header

    while (socket.bytesAvailable() < (int)(2*sizeof(quint64)))
    {
        if (!socket.waitForReadyRead(riOctavePlugin::longTimeOutMilliSecs))
        {
            error("Waiting for header: %s",socket.errorString().toLatin1().data());
            return;
        }
    }

    // Read timestep count and blocksize

    quint64 columnCount;
    quint64 byteCountForOneTimestep;
    size_t  activeCellCount;

    socketStream >> columnCount;
    socketStream >> byteCountForOneTimestep;

    activeCellCount = byteCountForOneTimestep / sizeof(qint32);

    dim_vector dv (2, 1);
    dv(0) = activeCellCount;
    dv(1) = columnCount;
    activeCellInfo.resize(dv);

    if (!(byteCountForOneTimestep && columnCount))
    {
        error ("Could not find the requested data in ResInsight");
        return;
    }

    qint32* internalMatrixData = (qint32*)activeCellInfo.fortran_vec()->mex_get_data();
    QStringList errorMessages;
    if (!RiaSocketDataTransfer::readBlockDataFromSocket(&socket, (char*)(internalMatrixData), columnCount * byteCountForOneTimestep, errorMessages))
    {
        for (int i = 0; i < errorMessages.size(); i++)
        {
            error("%s",errorMessages[i].toLatin1().data());
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
            if (riOctavePlugin::isOctaveValueNumeric(args(0)))
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
            if (riOctavePlugin::isOctaveValueNumeric(args(1)))
            {
                unsigned int argCaseId = args(1).uint_value();
                caseId = argCaseId;
            }
            else
            {
                porosityModel = args(1).char_matrix_value().row_as_string(0).c_str();
            }
        }     

        getActiveCellInfo(propertyFrames, "127.0.0.1", 40001, caseId, porosityModel);

        return octave_value(propertyFrames);
    }

    return octave_value();
}

