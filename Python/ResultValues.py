from ResInsight import ResInsight
import grpc, sys
import logging
		
def run():
    # NOTE(gRPC Python Team): .close() is possible on a channel and should be
    # used in circumstances in which the with statement does not fit the needs
    # of the code.
	logging.basicConfig()
	
	try:
		resInsight  = ResInsight('localhost:50051')
		timeStepsInfo = resInsight.grid.numberOfTimeSteps(ResInsight.Case(id=0))
		print ("Number of time steps: " + str(timeStepsInfo.value))
		resultsAllTimeSteps = []
		for timeStep in range(0, timeStepsInfo.value - 1):
			results = resInsight.grid.results(ResInsight.ResultRequest(ResInsight.Case(id=0), ResInsight.ResultAddress(0, "SOIL"), timeStep))
			print ("Got " + str(len(results.value)) + " values")
			resultsAllTimeSteps.append(results.value)

		print("Have stored results array containing " + str(len(resultsAllTimeSteps)) + " time steps")

		print("Looking for first cell with a decent SOIL value")
		indexFirstProperCell = 0
		for i in range(0, len(resultsAllTimeSteps[0])):
			result = resultsAllTimeSteps[0][i]
			if indexFirstProperCell == 0 and result > 0.01:
				indexFirstProperCell = i
		
		for resultsForTimeStep in resultsAllTimeSteps:
			print ("Result for cell " + str(indexFirstProperCell) + ": " + str(resultsForTimeStep[indexFirstProperCell]))
		
	except grpc.RpcError as e:
		if e.code() == grpc.StatusCode.NOT_FOUND:
			print("Case id not found")
		else:
			logging.error('Other error: %s', e)


if __name__ == '__main__':
    run()
