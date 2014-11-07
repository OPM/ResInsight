from math import sqrt, pi, cos, sin, acos


class UnityQuaternion(object):

    rad2deg = 180.0 / pi
    deg2rad = pi / 180.0

    def __init__(self, angle, x, y, z):
        super(UnityQuaternion, self).__init__()

        length = sqrt(x * x + y * y + z * z)
        c = cos(angle * UnityQuaternion.deg2rad / 2.0)
        s = sin(angle * UnityQuaternion.deg2rad / 2.0) / length

        self.W = c
        self.X = x * s
        self.Y = y * s
        self.Z = z * s


    def __mul__(self, other):
        w = self.W * other.W - self.X * other.X - self.Y * other.Y - self.Z * other.Z
        x = self.W * other.X + self.X * other.W + self.Y * other.Z - self.Z * other.Y
        y = self.W * other.Y - self.X * other.Z + self.Y * other.W + self.Z * other.X
        z = self.W * other.Z + self.X * other.Y - self.Y * other.X + self.Z * other.W

        result = UnityQuaternion(0.0, 1.0, 0.0, 0.0)
        result.W = w
        result.X = x
        result.Y = y
        result.Z = z

        result.normalize()
        return result


    def normalize(self):
        length = sqrt(self.W * self.W + self.X * self.X + self.Y * self.Y + self.Z * self.Z)
        self.W /= length
        self.X /= length
        self.Y /= length
        self.Z /= length

    def getAngleAsDegrees(self):
        return UnityQuaternion.rad2deg * 2.0 * acos(self.W)

    def __str__(self):
        return "%f degrees around (%f, %f, %f)" % (self.getAngleAsDegrees(), self.X, self.Y, self.Z)

