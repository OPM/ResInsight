from ert.enkf.enums.realization_state_enum import RealizationStateEnum
from ert.enkf.state_map import StateMap
from ert.test import TestAreaContext
from ert.test import ExtendedTestCase


class StateMapTest(ExtendedTestCase):

    def test_state_map(self):
        state_map = StateMap()

        self.assertEqual(len(state_map), 0)

        with self.assertRaises(TypeError):
            r = state_map["r"]

        with self.assertRaises(IOError):
            s2 = StateMap("DoesNotExist")

        with self.assertRaises(IOError):
            state_map.load("/file/does/not/exist")

        with self.assertRaises(IndexError):
            v = state_map[0]

        with self.assertRaises(TypeError):
            state_map["r"] = RealizationStateEnum.STATE_INITIALIZED

        with self.assertRaises(TypeError):
            state_map[0] = "INITIALIZED"

        with self.assertRaises(IndexError):
            state_map[-1] = RealizationStateEnum.STATE_INITIALIZED


        state_map[0] = RealizationStateEnum.STATE_INITIALIZED

        self.assertEqual(len(state_map), 1)

        state_map[1] = RealizationStateEnum.STATE_INITIALIZED
        state_map[1] = RealizationStateEnum.STATE_HAS_DATA

        self.assertEqual(len(state_map), 2)

        index = 0
        for state in state_map:
            self.assertEqual(state, state_map[index])
            index += 1

        states = [state for state in state_map]

        self.assertEqual(states, [RealizationStateEnum.STATE_INITIALIZED, RealizationStateEnum.STATE_HAS_DATA])


        state_map[5] = RealizationStateEnum.STATE_INITIALIZED
        self.assertEqual(len(state_map), 6)

        self.assertEqual(state_map[2], RealizationStateEnum.STATE_UNDEFINED)
        self.assertEqual(state_map[3], RealizationStateEnum.STATE_UNDEFINED)
        self.assertEqual(state_map[4], RealizationStateEnum.STATE_UNDEFINED)
        self.assertEqual(state_map[5], RealizationStateEnum.STATE_INITIALIZED)

        self.assertFalse(state_map.isReadOnly())

        with TestAreaContext("python/state-map/fwrite") as work_area:
            state_map.save("MAP")
            s2 = StateMap("MAP")
            self.assertTrue( state_map == s2 )
            


    def test_state_map_transitions(self):
        self.assertTrue(StateMap.isLegalTransition(RealizationStateEnum.STATE_UNDEFINED, RealizationStateEnum.STATE_INITIALIZED))
        self.assertTrue(StateMap.isLegalTransition(RealizationStateEnum.STATE_INITIALIZED, RealizationStateEnum.STATE_HAS_DATA))
        self.assertTrue(StateMap.isLegalTransition(RealizationStateEnum.STATE_INITIALIZED, RealizationStateEnum.STATE_LOAD_FAILURE))
        self.assertTrue(StateMap.isLegalTransition(RealizationStateEnum.STATE_INITIALIZED, RealizationStateEnum.STATE_PARENT_FAILURE))
        self.assertTrue(StateMap.isLegalTransition(RealizationStateEnum.STATE_HAS_DATA, RealizationStateEnum.STATE_PARENT_FAILURE))

        self.assertFalse(StateMap.isLegalTransition(RealizationStateEnum.STATE_UNDEFINED, RealizationStateEnum.STATE_LOAD_FAILURE))
        self.assertFalse(StateMap.isLegalTransition(RealizationStateEnum.STATE_UNDEFINED, RealizationStateEnum.STATE_HAS_DATA))

        with self.assertRaises(TypeError):
            StateMap.isLegalTransition("error", RealizationStateEnum.STATE_UNDEFINED)

        with self.assertRaises(TypeError):
            StateMap.isLegalTransition(RealizationStateEnum.STATE_UNDEFINED, "error")

        with self.assertRaises(TypeError):
            StateMap.isLegalTransition("error", "exception")


    def test_active_list(self):
        state_map = StateMap()
        state_map[0] = RealizationStateEnum.STATE_INITIALIZED
        state_map[2] = RealizationStateEnum.STATE_INITIALIZED
        state_map[2] = RealizationStateEnum.STATE_HAS_DATA
    
        initialized = state_map.realizationList( RealizationStateEnum.STATE_INITIALIZED )
        self.assertEqual( len(initialized) , 1 )
        self.assertEqual( initialized[0] , 0 )
        
        has_data = state_map.realizationList( RealizationStateEnum.STATE_HAS_DATA )
        self.assertEqual( len(has_data) , 1 )
        self.assertEqual( has_data[0] , 2)

