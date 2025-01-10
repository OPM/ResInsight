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
    resinsight_executable_from_env = os.environ.get("RESINSIGHT_EXECUTABLE")
    if resinsight_executable_from_env is None:
        print("RESINSIGHT_EXECUTABLE environment variable is not set")
        return
    instance = rips.Instance.launch(console=True, launch_port=0)
    print(instance.location)

    print(f"Sleeping for {sec} second(s): ", instance.location)
    time.sleep(sec)
    print(f"Done sleeping", instance.location)

    instance.exit()


def test_launch_sequential(rips_instance, initialize_test):
    resinsight_executable_from_env = os.environ.get("RESINSIGHT_EXECUTABLE")
    if resinsight_executable_from_env is None:
        print("RESINSIGHT_EXECUTABLE environment variable is not set")
        return
    instance_list = []
    for i in range(4):
        rips_instance = rips.Instance.launch(console=True)
        instance_list.append(rips_instance)

    for instance in instance_list:
        print(instance)
        instance.exit()


def test_launch_parallell(rips_instance, initialize_test):
    resinsight_executable_from_env = os.environ.get("RESINSIGHT_EXECUTABLE")
    if resinsight_executable_from_env is None:
        print("RESINSIGHT_EXECUTABLE environment variable is not set")
        return

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
