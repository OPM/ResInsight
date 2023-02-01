import grpc


class RetryOnRpcErrorClientInterceptor(
    grpc.UnaryUnaryClientInterceptor, grpc.StreamUnaryClientInterceptor
):
    def __init__(
        self,
        *,
        retry_policy,
        status_for_retry,
    ):
        self.retry_policy = retry_policy
        self.status_for_retry = status_for_retry

    def _intercept_call(self, continuation, client_call_details, request_or_iterator):
        for retry_num in range(self.retry_policy.num_retries()):
            response = continuation(client_call_details, request_or_iterator)

            if isinstance(response, grpc.RpcError):
                # Return if it was last attempt
                if retry_num == (self.retry_policy.num_retries() - 1):
                    return response

                # If status code is not in retryable status codes
                if (
                    self.status_for_retry
                    and response.code() not in self.status_for_retry
                ):
                    return response

                self.retry_policy.sleep(retry_num)
            else:
                return response

    def intercept_unary_unary(self, continuation, client_call_details, request):
        return self._intercept_call(continuation, client_call_details, request)

    def intercept_stream_unary(
        self, continuation, client_call_details, request_iterator
    ):
        return self._intercept_call(continuation, client_call_details, request_iterator)
