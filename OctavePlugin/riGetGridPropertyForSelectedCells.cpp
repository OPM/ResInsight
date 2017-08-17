#include <QtNetwork>
#include <QStringList>

#include <octave/oct.h>

#include "riSettings.h"

#include "RiaSocketDataTransfer.cpp"  // NB! Include cpp-file to avoid linking of additional file in oct-compile configuration

void getGridPropertyForSelectedCells(Matrix& propertyFrames, const QString &serverName, quint16 serverPort,
                        const qint64& caseId, QString propertyName, const int32NDArray& requestedTimeSteps, QString porosityModel)
{
    QTcpSocket socket;
    socket.connectToHost(serverName, serverPort);

    if (!socket.waitForConnected(riOctavePlugin::connectTimeOutMilliSecs))
    {
        error((("Connection: ") + socket.errorString()).toLatin1().data());
        return;
    }

    QDataStream socketStream(&socket);
    socketStream.setVersion(riOctavePlugin::qtDataStreamVersion);

    // Create command as a string with arguments , and send it:

    QString command;
    command += "GetGridPropertyForSelectedCells " + QString::number(caseId) + " " + propertyName + " " + porosityModel;

    for (int i = 0; i < requestedTimeSteps.length(); ++i)
    {
        if (i == 0) command += " ";
        command += QString::number(static_cast<int>(requestedTimeSteps.elem(i)) - 1); // To make the index 0-based
        if (i != requestedTimeSteps.length() - 1) command += " ";
    }

    QByteArray cmdBytes = command.toLatin1();

    socketStream << (qint64)(cmdBytes.size());
    socket.write(cmdBytes);

    // Get response. First wait for the header

    while (socket.bytesAvailable() < (int)(2*sizeof(quint64)))
    {
        if (!socket.waitForReadyRead(riOctavePlugin::longTimeOutMilliSecs))
        {
            error((("Waiting for header: ") + socket.errorString()).toLatin1().data());
            return;
        }
    }

    // Read timestep count and blocksize

    quint64 timestepCount;
    quint64 byteCount;
    size_t  selectedCellCount;

    socketStream >> timestepCount;
    socketStream >> byteCount;

    selectedCellCount = byteCount / sizeof(double);
    propertyFrames.resize(selectedCellCount, timestepCount);

    if (!(byteCount && timestepCount))
    {
        error ("Could not find the requested data in ResInsight");
        return;
    }

    quint64 totalByteCount = byteCount * timestepCount;

    double* internalMatrixData = propertyFrames.fortran_vec();
    QStringList errorMessages;
    if (!RiaSocketDataTransfer::readBlockDataFromSocket(&socket, (char*)(internalMatrixData), totalByteCount, errorMessages))
    {
        for (int i = 0; i < errorMessages.size(); i++)
        {
            error(errorMessages[i].toLatin1().data());
        }

        return;
    }

    QString tmp = QString("riGetGridPropertyForSelectedCells : Read %1").arg(propertyName);

    if (caseId < 0)
    {
        tmp += QString(" from current case.");
    }
    else
    {
        tmp += QString(" from case with Id: %1.").arg(caseId);
    }
    octave_stdout << tmp.toStdString() << " Selected cells cells : " << selectedCellCount << ", Time steps : " << timestepCount << std::endl;

    return;
}



DEFUN_DLD (riGetGridPropertyForSelectedCells, args, nargout,
           "Usage:\n"
           "\n"
           "Matrix[numSelectedCells][numTimestepsRequested]\n"
           "  riGetGridPropertyForSelectedCells([CaseId], PropertyName, [RequestedTimeSteps], [PorosityModel = \"Matrix\"|\"Fracture\"] )\n"
           "\n"
           "This function returns a two dimensional matrix: [numSelectedCells][numTimestepsRequested] containing the requested property data from the case with CaseId.\n"
           "If the CaseId is not defined, ResInsight's Current Case is used.\n"
           "The RequestedTimeSteps must contain a list of 1-based indices to the requested time steps. If not defined, all the time steps are returned.\n"
           )
{
    if (nargout < 1)
    {
        error("riGetGridPropertyForSelectedCells: Missing output argument.\n");
        print_usage();
        return octave_value_list ();
    }

    int nargin = args.length ();
    if (nargin < 1)
    {
        error("riGetGridPropertyForSelectedCells: Too few arguments. The name of the property requested is necessary.\n");
        print_usage();
        return octave_value_list ();
    }

    if (nargin > 4)
    {
        error("riGetGridPropertyForSelectedCells: Too many arguments.\n");
        print_usage();
        return octave_value_list ();
    }

    std::vector<int> argIndices;
    argIndices.push_back(0);
    argIndices.push_back(1);
    argIndices.push_back(2);
    argIndices.push_back(3);

    // Check if we have a CaseId:
    if (!args(argIndices[0]).is_numeric_type())
    {
        argIndices[0] = -1;
        for (size_t aIdx = 1; aIdx < argIndices.size(); ++aIdx)
            --argIndices[aIdx];
    }

    // Check if we have a Requested TimeSteps
    
    if (!(nargin > argIndices[2] && (args(argIndices[2]).is_matrix_type() || args(argIndices[2]).is_numeric_type()) && !args(argIndices[2]).is_string()))
    {
        argIndices[2] = -1;
        for (size_t aIdx = 3; aIdx < argIndices.size(); ++aIdx)
            --argIndices[aIdx];
    }

    // Check if we have a PorosityModel

    int lastArgumentIndex = argIndices[3] ;
    if (!(nargin > argIndices[3] && args(argIndices[3]).is_string()))
    {
        argIndices[3] = -1;
        for (size_t aIdx = 4; aIdx < argIndices.size(); ++aIdx)
            --argIndices[aIdx];
    }

    // Check if we have more arguments than we should
    if (nargin > lastArgumentIndex + 1)
    {
        error("riGetGridPropertyForSelectedCells: Unexpected argument after the PorosityModel.\n");
        print_usage();
        return octave_value_list ();
    }

    // Setup the argument list

    Matrix propertyFrames;
    int caseId = -1;
    std::string propertyName = "UNDEFINED";
    int32NDArray requestedTimeSteps;
    std::string porosityModel = "Matrix";

    if (argIndices[0] >= 0) caseId              = args(argIndices[0]).int_value();
    if (argIndices[1] >= 0) propertyName        = args(argIndices[1]).char_matrix_value().row_as_string(0);
    if (argIndices[2] >= 0) requestedTimeSteps  = args(argIndices[2]).int32_array_value();
    if (argIndices[3] >= 0) porosityModel       = args(argIndices[3]).string_value();

    if (porosityModel != "Matrix" && porosityModel != "Fracture")
    {
        error("riGetGridPropertyForSelectedCells: The value for \"PorosityModel\" is unknown. Please use either \"Matrix\" or \"Fracture\"\n");
        print_usage();
        return octave_value_list ();
    }

    getGridPropertyForSelectedCells(propertyFrames, "127.0.0.1", 40001, caseId, propertyName.c_str(), requestedTimeSteps, porosityModel.c_str());

    return octave_value(propertyFrames);
}
