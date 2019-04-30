from ResInsight import ResInsight
import grpc
		
def run():
    # NOTE(gRPC Python Team): .close() is possible on a channel and should be
    # used in circumstances in which the with statement does not fit the needs
    # of the code.
	try:
		resInsight  = ResInsight('localhost:50051')
		response = resInsight.grid.dimensions(ResInsight.Case(id=0))			
		print("Grid Dimensions received: " + str(response.i) + ", " + str(response.j) + ", " + str(response.k))
	except grpc.RpcError as e:
		if e.code() == grpc.StatusCode.NOT_FOUND:
			print("Case id not found")
		else:
			print("Other error")


if __name__ == '__main__':
    run()
