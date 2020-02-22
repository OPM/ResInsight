# This file contains feature checks which affect the API of the final
# product; i.e. if the test for zlib fails the function
# buffer_fwrite_compressed() will not be available in the final
# installation.
# 
# The results of these tests will be assembled in the
# ert/util/ert_api_config.h header; all the symbols in that header will
# have a ERT_ prefix. The generated header is part of the api and can be
# included by other header files in the ert source.

#-----------------------------------------------------------------

