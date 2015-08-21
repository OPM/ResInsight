from ert_gui.models import ErtConnector
from ert_gui.models.mixins import BasicModelMixin
from ert.test import ExtendedTestCase


class EmptyModel(ErtConnector, BasicModelMixin):
    def __init__(self):
        super(EmptyModel, self).__init__()


class TestModel(ErtConnector, BasicModelMixin):
    TEST_EVENT = "test_event"

    def __init__(self):
        self.value = None
        super(TestModel, self).__init__()

    def registerDefaultEvents(self):
        self.observable().addEvent(TestModel.TEST_EVENT)

    def getValue(self):
        return self.value

    def setValue(self, value):
        self.value = value
        self.observable().notify(TestModel.TEST_EVENT)

    def hasErt(self):
        return self.ert() is not None




class ModelTest(ExtendedTestCase):


    def setUp(self):
        TestModel().setValue(None)

    def tearDown(self):
        TestModel().setValue(None)
        ErtConnector.setErt(None)


    def test_abstract_model(self):
        model = EmptyModel()

        with self.assertRaises(NotImplementedError):
            model.getValue()

        with self.assertRaises(NotImplementedError):
            model.setValue("Error")



    def test_observer(self):
        model = TestModel()

        def observe():
            self.assertEqual(model.getValue(), 2)

        model.observable().attach(TestModel.TEST_EVENT, observe)

        model.setValue(2) # will call the observe function

        model.observable().detach(TestModel.TEST_EVENT, observe)


    def test_ertification(self):
        model = TestModel()

        self.assertFalse(TestModel().hasErt())

        ErtConnector.setErt("ERT")
        self.assertEqual(model.ert(), "ERT")

        self.assertTrue(TestModel().hasErt())


    def test_observability(self):
        model = EmptyModel()
        model1 = TestModel()
        model2 = TestModel()

        self.assertIsNone(ErtConnector())
        self.assertIsNotNone(model)
        self.assertIsNotNone(model1)
        self.assertIsNotNone(model2)

        self.assertNotEqual(model, model1)
        self.assertEqual(model1, model2)





