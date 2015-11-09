import math
from . import PrimeGenerator, PerlinNoise


class Interpolator(object):
    def __init__(self, x, y):
        self.x = x
        self.y = y

        assert len(x) == len(y)

    def __call__(self, x):
        if x <= self.x[0]:
            y = self.y[0]
        elif x >= self.x[len(self.x) - 1]:
            y = self.y[len(self.x) - 1]
        else:
            y = None
            for i in range(len(self.x) - 1):
                if self.x[i] <= x < self.x[i + 1]:
                    x_diff = self.x[i + 1] - self.x[i]
                    frac_x = (x - self.x[i]) / x_diff
                    y = self.cosineInterpolation(self.y[i], self.y[i + 1], frac_x)
                    break

        return y

    def cosineInterpolation(self, a, b, x):
        ft = x * 3.1415927
        f = (1.0 - math.cos(ft)) * 0.5
        return a * (1 - f) + b * f


class ShapeFunction(object):
    def __init__(self, x, y, scale=1.0):
        self.scale = scale
        self.interpolator = Interpolator(x, y)

    def __call__(self, x):
        return self.interpolator(x) * self.scale

    def scaledCopy(self, scale=1.0):
        return ShapeFunction(self.interpolator.x, self.interpolator.y, scale)


class ConstantShapeFunction(ShapeFunction):
    def __init__(self, value):
        super(ConstantShapeFunction, self).__init__([0.0], [value])



class ShapedNoise(object):

    def __init__(self, noiseFunction, shapeFunction, divergenceFunction, offset=0.0, cutoff=None):
        self.shapeFunction = shapeFunction
        self.divergenceFunction = divergenceFunction
        self.noiseFunction = noiseFunction
        self.offset = offset
        self.cutoff = cutoff

    def __call__(self, x, scale=1.0):
        scaled_x = x * scale
        result = self.shapeFunction(scaled_x) + self.noiseFunction(scaled_x) * self.divergenceFunction(scaled_x)
        result += self.offset
        if self.cutoff is not None:
            result = max(result, self.cutoff)
        return result


class ShapeCreator(object):

    @staticmethod
    def createShapeFunction(count=1000, persistence=0.2, octaves=8, seed=1):
        """ @rtype: ShapeFunction """
        prime_generator = PrimeGenerator(seed=seed)
        perlininator = PerlinNoise(persistence=persistence, number_of_octaves=octaves, prime_generator=prime_generator)

        x_values = [x / float(count) for x in range(count)]
        y_values = [perlininator(x) for x in x_values]

        return ShapeFunction(x_values, y_values)

    @staticmethod
    def createShapedPerlinFunction(divergence_x, divergence_y, shape_seed=None, perlin_seed=None, count=1000, persistence=0.2, octaves=8, offset=0.0, cutoff=None):
        """ @rtype: ShapedNoise """
        shapeFunction = ShapeCreator.createShapeFunction(count, persistence, octaves, shape_seed)
        divergenceFunction = ShapeFunction(divergence_x, divergence_y)
        prime_generator = PrimeGenerator(perlin_seed)
        perlin_noise = PerlinNoise(persistence, octaves, prime_generator)

        return ShapedNoise(perlin_noise, shapeFunction, divergenceFunction, offset=offset, cutoff=cutoff)

    @staticmethod
    def createNoiseFunction(shapeFunction=None, divergenceFunction=None, seed=None, persistence=0.2, octaves=8, offset=0.0, cutoff=None):
        """ @rtype: ShapedNoise """
        if shapeFunction is None:
            shapeFunction = ConstantShapeFunction(0.0)

        if divergenceFunction is None:
            divergenceFunction = ConstantShapeFunction(1.0)

        prime_generator = PrimeGenerator(seed)
        perlin_noise = PerlinNoise(persistence, octaves, prime_generator)

        noise = ShapedNoise(perlin_noise, shapeFunction, divergenceFunction, offset=offset, cutoff=cutoff)
        return noise
