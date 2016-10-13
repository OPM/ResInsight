import datetime
import os
import time
from threading import Thread

from ert import Version
from ert.enkf import EnKFMain
from ert.enkf.enums import RealizationStateEnum
from ert.server import ErtRPCServer, ErtRPCClient
from ert.test import ErtTestContext, ExtendedTestCase, ErtTest
from tests.ert.server import RPCServiceContext


def realizationIsInitialized(ert, case_name, realization_number):
    assert isinstance(ert, EnKFMain)
    fs = ert.getEnkfFsManager().getFileSystem(case_name)
    state_map = fs.getStateMap()
    state = state_map[realization_number]
    return state == RealizationStateEnum.STATE_INITIALIZED or state == RealizationStateEnum.STATE_HAS_DATA


class RPCServiceTest(ExtendedTestCase):
    def setUp(self):
        self.config = self.createTestPath("local/snake_oil_no_data/snake_oil.ert")

    def test_server_creation(self):
        with ErtTestContext("ert/server/rpc/server", self.config) as test_context:
            ert = test_context.getErt()
            server = ErtRPCServer(ert)

            self.assertIsNotNone(server.port)
            self.assertEqual(ert, server._config)
            self.assertEqual(os.path.basename(self.config), server._config_file)

            thread = Thread(name="ErtRPCServerTest")
            thread.run = server.start
            thread.start()

            server.stop()

    def test_client_interaction(self):
        target_case_names = ["batch_1", ".batch_1", "batch_1", ".batch_1"]
        kw = [
            {"SNAKE_OIL_PARAM": [0.50, 6, 1.750, 0.250, 0.990, 2, 1.770, 0.330, 0.550, 0.770]},
            {"SNAKE_OIL_PARAM": [0.51, 7, 1.751, 0.251, 0.991, 3, 1.771, 0.331, 0.551, 0.771]},
            {"SNAKE_OIL_PARAM": [0.52, 8, 1.752, 0.252, 0.992, 4, 1.772, 0.332, 0.552, 0.772]},
            {"SNAKE_OIL_PARAM": [0.53, 9, 1.753, 0.253, 0.993, 5, 1.773, 0.333, 0.553, 0.773]}
        ]

        expected_state_map = {
            "batch_1": [RealizationStateEnum.STATE_HAS_DATA,
                        RealizationStateEnum.STATE_UNDEFINED,
                        RealizationStateEnum.STATE_HAS_DATA
                        ],

            ".batch_1": [RealizationStateEnum.STATE_UNDEFINED,
                         RealizationStateEnum.STATE_HAS_DATA,
                         RealizationStateEnum.STATE_UNDEFINED,
                         RealizationStateEnum.STATE_HAS_DATA
                         ]
        }

        expected_custom_kw = [
            {'RATING': 'EXCELLENT', 'NPV': 125692.534209},
            {'RATING': 'AVERAGE', 'NPV': 87423.5773042},
            {'RATING': 'GOOD', 'NPV': 113243.319848},
            {'RATING': 'AVERAGE', 'NPV': 91781.8557083}
        ]

        with RPCServiceContext("ert/server/rpc/client", self.config) as server:
            client = ErtRPCClient("localhost", server.port)
            self.assertEqual(Version.currentVersion().versionTuple(), client.ertVersion())
            realization_count = len(target_case_names)

            client.startSimulationBatch("default", realization_count)

            self.assertTrue(server.isInitializationCaseAvailable())
            self.assertTrue(server.isRunning())


            for iens in range(realization_count):
                client.addSimulation(target_case_names[iens], geo_id=0, pert_id=0, sim_id=iens, keywords=kw[iens])
                self.assertTrue(realizationIsInitialized(server.ert, target_case_names[iens], iens))

            while client.isRunning():
                time.sleep(1)

            for case in expected_state_map:
                fs = server.ert.getEnkfFsManager().getFileSystem(case)
                state_map = fs.getStateMap()
                states = expected_state_map[case]

                for index, state in enumerate(states):
                    self.assertEqual(state, state_map[index])

            time_map = client.getTimeMap("batch_1")
            self.assertEqual(datetime.datetime(2010, 1, 1), time_map[0])
            self.assertEqual(datetime.datetime(2015, 6, 13), time_map[199])

            with self.assertRaises(KeyError):
                client.getGenDataResult(target_case_names[0], 0, 199, "UNKNOWN_KEYWORD")

            for iens, case in enumerate(target_case_names):
                self.assertTrue(client.isRealizationFinished(iens))
                self.assertTrue(client.didRealizationSucceed(iens))
                self.assertFalse(client.didRealizationFail(iens))

                self.assertTrue(client.isCustomKWKey("SNAKE_OIL_NPV"))
                self.assertFalse(client.isGenDataKey("SNAKE_OIL_NPV"))
                self.assertTrue(client.isGenDataKey("SNAKE_OIL_OPR_DIFF"))

                result = client.getGenDataResult(case, iens, 199, "SNAKE_OIL_OPR_DIFF")
                self.assertEqual(len(result), 2000)

                result = client.getCustomKWResult(case, iens, "SNAKE_OIL_NPV")
                self.assertTrue("NPV" in result)
                self.assertTrue("RATING" in result)
                self.assertEqual(expected_custom_kw[iens]["RATING"], result["RATING"])
                self.assertAlmostEqual(expected_custom_kw[iens]["NPV"], result["NPV"])


    def test_multiple_threads(self):
        expected_ckw = {
            0:{'RATING': 'EXCELLENT', 'NPV': 125692.534209},
            1:{'RATING': 'AVERAGE', 'NPV': 87384.4316741},
            2:{'RATING': 'GOOD', 'NPV': 113181.024141},
            3:{'RATING': 'AVERAGE', 'NPV': 91659.8650599},
            4:{'RATING': 'EXCELLENT', 'NPV': 134891.570703},
            5:{'RATING': 'GOOD', 'NPV': 117270.977546},
            6:{'RATING': 'GOOD', 'NPV': 106838.28455},
            7:{'RATING': 'EXCELLENT', 'NPV': 144001.339},
            8:{'RATING': 'AVERAGE', 'NPV': 95423.9155004},
            9:{'RATING': 'AVERAGE', 'NPV': 96123.0227439}
        }

        with RPCServiceContext("ert/server/rpc/multi_client", self.config, store_area=True) as server:
            client_count = len(expected_ckw)

            # initializeCase(server.ert, "default", 1)
            thread_success_state = {}

            def createClientInteractor(target_case_name, iens):
                def clientInteraction():
                    thread_success_state[iens] = False
                    keywords = {"SNAKE_OIL_PARAM": [0.50, iens + 2, 1.750, 0.250, 0.990, 2 + client_count - iens, 1.770, 0.330, 0.550, 0.770]}
                    client = ErtRPCClient("localhost", server.port)
                    client.startSimulationBatch("default", client_count)

                    self.assertTrue(client.isRunning())
                    self.assertTrue(client.isInitializationCaseAvailable())

                    client.addSimulation(target_case_name, 0, 0, iens, keywords)
                    self.assertTrue(realizationIsInitialized(server.ert, target_case_name, iens))

                    while not client.isRealizationFinished(iens):
                        time.sleep(0.5)

                    self.assertTrue(client.didRealizationSucceed(iens))

                    result = client.getCustomKWResult(target_case_name, iens, "SNAKE_OIL_NPV")
                    self.assertTrue("NPV" in result)
                    self.assertTrue("RATING" in result)
                    self.assertEqual(expected_ckw[iens]["RATING"], result["RATING"])
                    self.assertAlmostEqual(expected_ckw[iens]["NPV"], result["NPV"])

                    thread_success_state[iens] = True

                return clientInteraction

            threads = []
            for iens in range(client_count):
                thread = Thread(name="client_%d" % iens)
                target_case_name = "target" if iens % 2 == 0 else ".target"
                thread.run = createClientInteractor(target_case_name, iens)
                threads.append(thread)

            for thread in threads:
                thread.start()

            while server.isRunning():
                time.sleep(0.1)

            for thread in threads:
                thread.join()

            self.assertTrue(all(success for success in thread_success_state.values()))
