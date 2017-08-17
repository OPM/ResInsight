#!/usr/bin/env python
import sys

# This is a small executable which is used to verify the install,
# Prior to calling this the environment must have been prepared to
# ensure that the newly installed versions are picked up - this can
# typically be achieved with the bash script bin/test_install which is
# generated during the configure process.

from ecl.ecl import EclSum, EclGrid



