from ResInsight import ResInsight
import grpc
import sys
		
def run():
    # NOTE(gRPC Python Team): .close() is possible on a channel and should be
    # used in circumstances in which the with statement does not fit the needs
    # of the code.
	try:
		port = str(50051)
		if len(sys.argv) > 1:
			port = sys.argv[1]
		resInsight  = ResInsight("localhost:" + port)
		gridCount = resInsight.GridInfo.GridCount(ResInsight.Case(id=0))
		print("Number of grids: " + str(gridCount.count))
		gridDimensions = resInsight.GridInfo.AllDimensions(ResInsight.Case(id=0))
		for dimensions in gridDimensions.dimensions:
			print("Grid Dimensions received: " + str(dimensions.i) + ", " + str(dimensions.j) + ", " + str(dimensions.k))

	except grpc.RpcError as e:
		if e.code() == grpc.StatusCode.NOT_FOUND:
			print("Case id not found")
		else:
			print("Other error")


if __name__ == '__main__':
    run()
