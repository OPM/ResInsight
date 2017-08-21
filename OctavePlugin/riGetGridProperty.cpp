#include <QtNetwork>
#include <QStringList>

#include <octave/oct.h>

#include "riSettings.h"
#include "RiaSocketDataTransfer.cpp"  // NB! Include cpp-file to avoid linking of additional file in oct-compile configuration


void getGridProperty(NDArray& propertyFrames, const QString &serverName, quint16 serverPort,
                        const int& caseId, int gridIdx, QString propertyName, const int32NDArray& requestedTimeSteps, QString porosityModel)
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
    command += "GetGridProperty " + QString::number(caseId) + " " + QString::number(gridIdx) + " " + propertyName + " " + porosityModel;

    for (qint64 i = 0; i < requestedTimeSteps.length(); ++i)
    {
        if (i == 0) command += " ";
        command += QString::number(static_cast<int>(requestedTimeSteps.elem(i)) - 1); // To make the index 0-based
        if (i != requestedTimeSteps.length() -1) command += " ";
    }

    QByteArray cmdBytes = command.toLatin1();

    socketStream << (qint64)(cmdBytes.size());
    socket.write(cmdBytes);

    // Get response. First wait for the header

    while (socket.bytesAvailable() < (qint64)(4*sizeof(quint64)))
    {
        if (!socket.waitForReadyRead(riOctavePlugin::longTimeOutMilliSecs))
        {
            error((("Waiting for header: ") + socket.errorString()).toLatin1().data());
            return;
        }
    }

    // Read sizes

    quint64 totalByteCount;
   
    quint64 cellCountI;
    quint64 cellCountJ;
    quint64 cellCountK;
    quint64 timestepCount;

    socketStream >> cellCountI;
    socketStream >> cellCountJ;
    socketStream >> cellCountK;
    socketStream >> timestepCount;

    totalByteCount = cellCountI*cellCountJ*cellCountK*timestepCount*sizeof(double);
    
    if (!(totalByteCount))
    {
        error ("Could not find the requested data in ResInsight");
        return;
    }

    dim_vector dv;
    dv.resize(4);
    dv(0) = cellCountI;
    dv(1) = cellCountJ;
    dv(2) = cellCountK;
    dv(3) = timestepCount;

    propertyFrames.resize(dv);

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

    QString tmp = QString("riGetGridProperty : Read %1").arg(propertyName);

    if (caseId < 0)
    {
        tmp += QString(" from current case,");
    }
    else
    {
        tmp += QString(" from case with Id: %1,").arg(caseId);
    }

    tmp += QString(" grid index: %1, ").arg(gridIdx);

    octave_stdout << tmp.toStdString() << " I, J, K " << cellCountI << ", " << cellCountJ << ", " << cellCountK << ", Timesteps : " << timestepCount << std::endl;

    return;
}



DEFUN_DLD (riGetGridProperty, args, nargout,
           "Usage:\n"
           "\n"
           "Matrix[numI][numJ][numK][numTimestepsRequested] riGetGridProperty([CaseId], GridIndex , PropertyName, [RequestedTimeSteps], [PorosityModel = \"Matrix\"|\"Fracture\"])"
           "\n"
           "This function returns a matrix of the requested property data for all the grid cells in the requested grid for each requested time step.\n"
           "Grids are indexed from 0 (main grid) to max number of LGR's.\n"
           "If the CaseId is not defined, ResInsight’s Current Case is used.\n"
           "The RequestedTimeSteps must contain a list of indices to the requested time steps. If not defined, all the time steps are returned.\n"
           )
{
    if (nargout < 1)
    {
        error("riGetGridProperty: Missing output argument.\n");
        print_usage();
        return octave_value_list ();
    }

    int nargin = args.length ();
    if (nargin < 2)
    {
        error("riGetGridProperty: Too few arguments. The name of the property and index of the grid requested is neccesary.\n");
        print_usage();
        return octave_value_list ();
    }

    if (nargin > 5)
    {
        error("riGetGridProperty: Too many arguments.\n");
        print_usage();
        return octave_value_list ();
    }

    std::vector<int> argIndices;
    argIndices.push_back(0); // caseId
    argIndices.push_back(1); // GridIndex
    argIndices.push_back(2); // PropertyName
    argIndices.push_back(3); // TimeSteps
    argIndices.push_back(4); // PorosityModel

    // Check if we do not have a CaseId:
    if (args(argIndices[1]).is_string()) // Check if second argument is a text. If it is, the caseid is missing
    {
        argIndices[0] = -1;
        for (size_t aIdx = 1; aIdx < argIndices.size(); ++aIdx)
            --argIndices[aIdx];
    }

    // Check if we have a Requested TimeSteps

    if (!(nargin > argIndices[3] && (args(argIndices[3]).is_matrix_type() || args(argIndices[3]).is_numeric_type()) && !args(argIndices[3]).is_string()))
    {
        argIndices[3] = -1;
        for (size_t aIdx = 3; aIdx < argIndices.size(); ++aIdx)
            --argIndices[aIdx];
    }

    // Check if we have a PorosityModel

    int lastArgumentIndex = argIndices[4] ;
    if (!(nargin > argIndices[4] && args(argIndices[4]).is_string()))
    {
        argIndices[4] = -1;
        for (size_t aIdx = 5; aIdx < argIndices.size(); ++aIdx)
            --argIndices[aIdx];
    }

    // Check if we have more arguments than we should
    if (nargin > lastArgumentIndex + 1)
    {
        error("riGetGridProperty: Unexpected argument after the PorosityModel.\n");
        print_usage();
        return octave_value_list ();
    }

    // Setup the argument list

    NDArray propertyFrames;
    int caseId = -1;
    int gridIdx = 0;
    std::string propertyName = "UNDEFINED";
    int32NDArray requestedTimeSteps;
    std::string porosityModel = "Matrix";

    if (argIndices[0] >= 0) caseId              = args(argIndices[0]).int_value();
    if (argIndices[1] >= 0) gridIdx             = args(argIndices[1]).int_value();
    if (argIndices[2] >= 0) propertyName        = args(argIndices[2]).char_matrix_value().row_as_string(0);
    if (argIndices[3] >= 0) requestedTimeSteps  = args(argIndices[3]).int32_array_value();
    if (argIndices[4] >= 0) porosityModel       = args(argIndices[4]).string_value();


    if (porosityModel != "Matrix" && porosityModel != "Fracture")
    {
        error("riGetGridProperty: The value for \"PorosityModel\" is unknown. Please use either \"Matrix\" or \"Fracture\"\n");
        print_usage();
        return octave_value_list ();
    }

    getGridProperty(propertyFrames, "127.0.0.1", 40001, caseId, gridIdx, propertyName.c_str(), requestedTimeSteps, porosityModel.c_str());

    return octave_value(propertyFrames);
}
