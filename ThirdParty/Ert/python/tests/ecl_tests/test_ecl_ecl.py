from tests import EclTest

class EclEclTest(EclTest):

    def test_import(self):
        import ecl.ecl as ecl
        grid = ecl.EclGrid
        sum = ecl.EclSum
        f = ecl.EclFile
        rft = ecl.EclRFTFile

    def test_import2(self):
        from ecl.ecl import EclFile
        from ecl.ecl import EclGrid
        from ecl.ecl import EclSum
