from . import ShapeFunction, ShapeCreator

class OilSimulator(object):
    OPR_SHAPE = ShapeFunction([0.0, 0.2, 0.5, 0.7, 1.0], [0.0, 0.7, 0.2, 0.1, 0.01])
    GPR_SHAPE = ShapeFunction([0.0, 0.2, 0.5, 0.7, 1.0], [0.0, 0.5, 0.7, 0.7, 0.3])
    WPR_SHAPE = ShapeFunction([0.0, 0.2, 0.5, 0.7, 1.0], [0.0, 0.01, 0.3, 0.7, 1])
    BPR_SHAPE= ShapeFunction([0.0, 0.2, 0.5, 0.7, 1.0], [1.0, 0.7, 0.5, 0.3, 0.1])

    O_DIVERGENCE = ShapeFunction([0.0, 0.5, 0.7, 0.9, 1.0], [0.0, 0.5, 0.3, 0.1, 0.01])
    G_DIVERGENCE = ShapeFunction([0.0, 0.5, 0.7, 0.9, 1.0], [0.0, 0.1, 0.3, 0.2, 0.1])
    W_DIVERGENCE = ShapeFunction([0.0, 0.5, 0.7, 0.9, 1.0], [0.0, 0.1, 0.3, 0.2, 0.01])
    B_DIVERGENCE = ShapeFunction([0.0, 0.5, 0.7, 0.9, 1.0], [0.0, 0.1, 0.2, 0.3, 0.5])

    def __init__(self):
        self.__oprFunc = {}
        self.__gprFunc = {}
        self.__wprFunc = {}
        self.__bprFunc = {}
        self.__current_step = 0

        self.__fopt = 0.0
        self.__fopr = 0.0
        self.__fgpt = 0.0
        self.__fgpr = 0.0
        self.__fwpt = 0.0
        self.__fwpr = 0.0

        self.__fgor = 0.0
        self.__fwct = 0.0

        self.__wells = {}
        self.__bpr = {}

    def addWell(self, name, seed, persistence=0.2, octaves=8, divergence_scale=1.0):
        oil_div = OilSimulator.O_DIVERGENCE.scaledCopy(divergence_scale)
        gas_div = OilSimulator.G_DIVERGENCE.scaledCopy(divergence_scale)
        water_div = OilSimulator.W_DIVERGENCE.scaledCopy(divergence_scale)
        self.__oprFunc[name] = ShapeCreator.createNoiseFunction(OilSimulator.OPR_SHAPE, oil_div, seed, persistence=persistence, octaves=octaves, cutoff=0.0)
        self.__gprFunc[name] = ShapeCreator.createNoiseFunction(OilSimulator.GPR_SHAPE, gas_div, seed * 7, persistence=persistence * 3.5, octaves=octaves / 2, cutoff=0.0)
        self.__wprFunc[name] = ShapeCreator.createNoiseFunction(OilSimulator.WPR_SHAPE, water_div, seed * 11, persistence=persistence, octaves=octaves, cutoff=0.0)

        self.__wells[name] = {"opr": 0.0, "opt": 0.0, "gpr": 0.0, "gpt": 0.0, "wpr": 0.0, "wpt": 0.0}

    def addBlock(self, name, seed, persistence=0.2):
        self.__bprFunc[name] = ShapeCreator.createNoiseFunction(OilSimulator.BPR_SHAPE, OilSimulator.B_DIVERGENCE, seed, persistence=persistence, cutoff=0.0)
        self.__bpr[name] = 0.0

    def step(self, scale=1.0):
        self.__fopr = 0.0
        self.__fgpr = 0.0
        self.__fwpr = 0.0
        self.__fgor = 0.0
        self.__fwct = 0.0
        for key in self.__wells:
            oprFunction = self.__oprFunc[key]
            gprFunction = self.__gprFunc[key]
            wprFunction = self.__wprFunc[key]
            opr_value = oprFunction(self.__current_step, scale)
            gpr_value = gprFunction(self.__current_step, scale)
            wpr_value = wprFunction(self.__current_step, scale)

            well = self.__wells[key]
            well["opr"] = opr_value
            well["opt"] += opr_value
            well["gpr"] = gpr_value
            well["gpt"] += gpr_value
            well["wpr"] = wpr_value
            well["wpt"] += wpr_value
            self.__fopr += opr_value
            self.__fgpr += gpr_value
            self.__fwpr += wpr_value

            self.__fgor += self.gor(key)
            self.__fwct += self.wct(key)

        self.__fopt += self.__fopr
        self.__fgpt += self.__fgpr
        self.__fwpt += self.__fwpr

        self.__fgor /= len(self.__wells)
        self.__fwct /= len(self.__wells)

        for key in self.__bpr:
            bprFunction = self.__bprFunc[key]
            self.__bpr[key] = bprFunction(self.__current_step, scale)

        self.__current_step += 1

    def fopt(self):
        return self.__fopt

    def fopr(self):
        return self.__fopr

    def fgpt(self):
        return self.__fgpt

    def fgpr(self):
        return self.__fgpr

    def fwpt(self):
        return self.__fwpt

    def fwpr(self):
        return self.__fwpr

    def fgor(self):
        return self.__fgor

    def fwct(self):
        return self.__fwct

    def opr(self, well_name):
        return self.__wells[well_name]["opr"]

    def gpr(self, well_name):
        return self.__wells[well_name]["gpr"]

    def wpr(self, well_name):
        return self.__wells[well_name]["wpr"]

    def wct(self, well_name):
        opr = self.opr(well_name)
        wpr = self.wpr(well_name)
        opr = max(opr, 0.1)
        return wpr / (wpr + opr) if (wpr + opr) > 0.0 else 0.0

    def gor(self, well_name):
        opr = self.opr(well_name)
        gpr = self.gpr(well_name)
        opr = max(opr, 0.1)
        gpr = max(gpr, 0.1)
        return gpr / opr

    def bpr(self, block_name):
        return self.__bpr[block_name]
