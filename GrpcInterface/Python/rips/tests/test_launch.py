import sys
import os
import math
import pytest
import grpc
import tempfile
import time
import multiprocessing

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import rips

import dataroot


def launch_resinsight(sec=1):
    instance = rips.Instance.launch(console=True)
    print(instance.location)

    print(f"Sleeping for {sec} second(s)")
    time.sleep(sec)
    print(f"Done sleeping")

    instance.exit()


def test_OneCase(rips_instance, initialize_test):
    launch_seq = False
    launch_parallell = True

    if launch_seq:
        instance_list = []
        for i in range(4):
            rips_instance = rips.Instance.launch(console=True)
            instance_list.append(rips_instance)

        for instance in instance_list:
            print(instance)
            instance.exit()

    if launch_parallell:
        process_list = []

        instance_count = 10
        for i in range(instance_count):
            process = multiprocessing.Process(target=launch_resinsight)
            process_list.append(process)

        for process in process_list:
            process.start()

        # completing process
        for p in process_list:
            p.join()
