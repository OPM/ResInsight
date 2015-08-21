from ert_gui.models import Observable
from ert.test import ExtendedTestCase


def observeWithoutSelf():
    pass

class ObservableTest(ExtendedTestCase):

    def observeWithSelf(self):
        pass


    def test_observable(self):
        observable = Observable("Test Observer")

        observable.addEvent("test_event")
        self.assertTrue("test_event" in observable)

        observable.attach("test_event", self.observeWithSelf)
        observable.attach("test_event", observeWithoutSelf)

        with self.assertRaises(AssertionError):
            observable.attach("test_event", "Fail!") #not a callable

        with self.assertRaises(LookupError):
            observable.attach("fail", observeWithoutSelf) # not a valid event

        observable.notify("test_event")

        observable.detach("test_event", observeWithoutSelf)

        with self.assertRaises(ValueError):
            observable.detach("test_event", observeWithoutSelf)


        observer_as_lambda = lambda : observeWithoutSelf()
        observable.attach("test_event", observer_as_lambda)
        observable.notify("test_event")
        observable.detach("test_event", observer_as_lambda)


    def test_events(self):
        observable = Observable("Event observer")
        observable.addEvent("event1")
        observable.addEvent("event2")

        def event1():
            raise ValueError("event 1 error")

        def event2():
            raise NotImplementedError("event 1 error")

        observable.attach("event1", event1)
        observable.attach("event2", event2)

        with self.assertRaises(ValueError):
            observable.notify("event1")

        with self.assertRaises(NotImplementedError):
            observable.notify("event2")

        with self.assertRaises(LookupError):
            observable.notify("event3")

        observable.detach("event2", event2)

        # no exception raised when notify list is empty
        observable.notify("event2")

        with self.assertRaises(LookupError):
            observable.detach("event3", event2)

    def test_weak_references(self):
        observable = Observable("Event observer")
        observable.addEvent("event")

        self.addAndRemoveListener(observable) #the listener dies when the method returns.

        observable.notify("event") # Should not raise assertion error

    def addAndRemoveListener(self, observable):

        class WeakTest(object):
            def __init__(self):
                super(WeakTest, self).__init__()

            def listener(self):
                raise AssertionError("This should be caught!")

        o = WeakTest()

        observable.attach("event", o.listener)

        with self.assertRaises(AssertionError):
            observable.notify("event")
