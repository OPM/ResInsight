import sys
import os

sys.path.insert(1, os.path.join(sys.path[0], '../../'))
import rips

import dataroot
import tempfile
import pytest

import cv2
import numpy as np
import imutils

from os import walk

_resinsight_width = 1000
_resinsight_height = 600
_default_arguments = ['--snapshotsize', _resinsight_width, _resinsight_height, '--savesnapshots', 'all']

# set this environment variable to the regression test root, ie "d:/gitroot-ceesol/ResInsight-regression-test"
_regtest_root_folder = os.environ.get('RESINSIGHT_REG_TEST_ROOT')
#_regtest_root_folder = "d:/gitroot-ceesol/ResInsight-regression-test"

def launch_resinsight(testRootFolder, command_line_parameters):
    resinsight = rips.Instance.launch(command_line_parameters=command_line_parameters)

    resinsight.project.close()
    resinsight.exit()


def compareImages(testRootFolder):
    export_folder = testRootFolder + "/RegTestGeneratedImages"
    base_folder = testRootFolder + "/RegTestBaseImages"
    diff_folder = testRootFolder + "/RegTestDiffImages"

    generatedFileNames = []
    for (dirpath, dirnames, filenames) in walk(export_folder):
        generatedFileNames.extend(filenames)
        break

    for fileName in generatedFileNames:
        generated_image_file_name = export_folder + '/' + fileName
        generated_image = cv2.imread(generated_image_file_name)

        base_image_file_name = base_folder + '/' + fileName
        base_image = cv2.imread(base_image_file_name)

        # todo : check if images has same size
        if True:
            difference = cv2.subtract(generated_image, base_image)

            grayA = cv2.cvtColor(difference, cv2.COLOR_BGR2GRAY)
            cv2.imwrite(diff_folder + '/gray_' + fileName, grayA)

            thresh = cv2.threshold(grayA, 3, 255, cv2.THRESH_BINARY_INV | cv2.THRESH_OTSU)[1]
            threshold_image_file_name = diff_folder + '/thres_' + fileName
            cv2.imwrite(threshold_image_file_name, thresh)

            diff_image_file_name = diff_folder + '/' + fileName
            cv2.imwrite(diff_image_file_name, difference)
        


@pytest.mark.skipif(not _regtest_root_folder, reason="missing environment variable _regtest_root_folder")
def test_import_summary_case():
    
    casePath = _regtest_root_folder + "/ModelData/norne/NORNE_ATW2013.SMSPEC"
    testRootFolder = _regtest_root_folder + "/ProjectFiles/PythonDiff/summary"

    export_folder = testRootFolder + "/RegTestGeneratedImages"
    launch_resinsight(testRootFolder, _default_arguments + ['--summaryplot', 'FOPT', casePath, '--snapshotfolder', export_folder])
    compareImages(testRootFolder)


@pytest.mark.skipif(not _regtest_root_folder, reason="missing environment variable _regtest_root_folder")
def test_default_view():
    casePath = _regtest_root_folder + "/ModelData/norne/NORNE_ATW2013.EGRID"
    testRootFolder = _regtest_root_folder + "/ProjectFiles/PythonDiff/eclipse_view"
    export_folder = testRootFolder + "/RegTestGeneratedImages"

    launch_resinsight(testRootFolder, _default_arguments  + ['--case', casePath, '--snapshotfolder', export_folder])
    compareImages(testRootFolder)
