class Bounds(object):
    def __init__(self):
        super(Bounds, self).__init__()

        self.__min_x = None
        self.__min_y = None
        self.__min_z = None

        self.__max_x = None
        self.__max_y = None
        self.__max_z = None


    def addPoint(self, x, y, z):
        if self.__min_x is None or x < self.__min_x:
            self.__min_x = x

        if self.__min_y is None or y < self.__min_y:
            self.__min_y = y

        if self.__min_z is None or z < self.__min_z:
            self.__min_z = z

        if self.__max_x is None or x > self.__max_x:
            self.__max_x = x

        if self.__max_y is None or y > self.__max_y:
            self.__max_y = y

        if self.__max_z is None or z > self.__max_z:
            self.__max_z = z

    def __str__(self):
        return "x: [%f %f] y: [%f %f] z: [%f %f]" % (self.minX, self.maxX, self.minY, self.maxY, self.minZ, self.maxZ)


    def getMinX(self):
        return self.__min_x

    def getMinY(self):
        return self.__min_y

    def getMinZ(self):
        return self.__min_z


    def getMaxX(self):
        return self.__max_x

    def getMaxY(self):
        return self.__max_y

    def getMaxZ(self):
        return self.__max_z

    def getXDiff(self):
        return self.maxX - self.minX

    def getYDiff(self):
        return self.maxY - self.minY

    def getZDiff(self):
        return self.maxZ - self.minZ

    minX = property(getMinX)
    minY = property(getMinY)
    minZ = property(getMinZ)

    maxX = property(getMaxX)
    maxY = property(getMaxY)
    maxZ = property(getMaxZ)

    diffX = property(getXDiff)
    diffY = property(getYDiff)
    diffZ = property(getZDiff)