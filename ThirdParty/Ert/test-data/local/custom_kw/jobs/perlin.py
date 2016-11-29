#! /usr/bin/env python
import math
import random

PRIME_INDEX_1 = [49193,11887,23819,93983,28283,87179,74933,82561,29741,98453,72719,48193,66883,95071,12841,89603,
                 49261,52529,57697,70321,54617,49363,41233,39883,35393,33149,37493,42989,58073,62507,99829,41999,
                 44087,31907,10627,29231,57559,36809,17123,50593,38449,71317,38149,60637,10607,48677,23189,83701,
                 78853,35617,28477,86117,46901,80819,89491,36097,54881,94781,20707,20011,69457,14593,49253,35257,
                 14753,44851,10289,36097,62017,82723,10037,77551,89513,70429,30269,30703,77711,69313,20021,31657,
                 33851,27749,85667,42793,47599,92789,97771,84551,61637,68659,93263,62201,42131,78823,17747,31183,
                 89611,91009,76673,47051,94099,96757,90977,65141,46051,11093,19073,44633,27967,25171,41221,96223,
                 88997,74941,49559,11909,11593,97369,55733,35869,55849,87931,81131,99023,76561,78977,48857,74717]

PRIME_INDEX_2 = [7360349,1287757,3764759,5276833,2735671,7351777,7705903,2034551,
                 2427493,3883639,4260859,6814097,3226933,2648249,4458793,8015303,
                 2323733,7991233,5560879,9826913,3634811,3746299,1051543,2954789,
                 7874983,9380681,4577789,4306829,6714599,8395733,2718493,1429867,
                 5675147,6104573,3118727,2657243,9750043,1853377,9441353,7247969,
                 7669553,5334157,9376649,9518137,9368297,3912679,3230237,7291939,
                 1361677,1034167,4998089,1178239,5160677,6130199,8056553,8527361,
                 4261093,8640553,5553391,6024797,7275019,7245019,7661483,5120033,
                 4388117,5941147,7682189,9303467,7165777,1509163,5223929,9696487,
                 8012383,6254273,1400731,9958177,7733573,1498993,1553467,4791257,
                 4524521,7048633,3630821,7931179,2341457,6432269,9597703,4338011,
                 6665059,7911653,8384317,2230531,7904621,1633559,9096533,6873301,
                 2717821,5897977,3608543,2248243,3174599,8634233,4028963,6435001,
                 6611399,3250469,4046353,1429943,8552111,1970261,1045043,9552523,
                 6993799,6141467,5723479,9578867,9233299,7224641,3165023,4583899,
                 3905861,1633993,5013137,5092613,2197163,7732213,6559019,2538499]

PRIME_INDEX_3 = [1368916889,3054015583,6066123341,8673964289,9002483141,7187080993,5319345529,6961795349,
                 1653814157,3416288497,6454122317,2480898239,3878100221,5956454227,9767569853,5981528503,
                 4962084931,4489312199,3013312061,9818685161,4061204663,1816202221,7567463471,9839749459,
                 3993070667,5872839331,9256050443,4854483611,4876755749,3823459247,6244209637,4199084081,
                 6053970359,1093521049,7385602219,7289318273,9333908789,9701161343,8139801689,5013046681,
                 4094649187,2372669671,9010267157,4298511787,7575340187,9252205969,5923706413,7112626819,
                 6531270523,8379490583,4521945149,6804302789,6984132251,9173492033,1657527653,1532523367,
                 3132088123,5910371431,7551540169,1643193353,6127000571,2637510193,7904761379,2954227033,
                 7344843263,8077648457,9397237879,6775740173,1950824101,1152859999,2990299673,8197021109,
                 2184824123,4309539167,1742841137,9113517421,4752058561,5594292329,9565022153,8519292151,
                 6553311239,5204301593,8405487593,1987918357,3175759277,5659428917,6611421781,8765753053,
                 3781235599,5651365571,8399394649,3867050417,3258145379,9836441977,2499690049,2742615479,
                 7720787857,6135275183,9580731373,1860360379,2879750459,4302251633,8019104719,3889658671,
                 7242891343,2516043583,8081336113,7718332591,4940550151,2216825899,7387662781,5562762407,
                 2486416781,9111045257,1197705721,6649659239,6110149477,4548595937,3169540631,8993669479,
                 6444114251,3098519969,1609592407,5803463207,8385117647,3056488453,1046337653,8165632597]

class PerlinNoise(object):

    def __init__(self, persistence=0.5, number_of_octaves=4, prime_1=15731, prime_2=789221, prime_3=1376312589):
        self.persistence = persistence
        self.number_of_octaves = number_of_octaves
        self.prime_1 = prime_1
        self.prime_2 = prime_2
        self.prime_3 = prime_3


    def cosineInterpolation(self, a, b, x):
        ft = x * 3.1415927
        f = (1.0 - math.cos(ft)) * 0.5
        return a * (1 - f) + b * f


    def noise(self, x):
        x = (x << 13) ^ x
        return 1.0 - ((x * (x * x + self.prime_1 + self.prime_2) + self.prime_3) & 0x7fffffff) / 1073741824.0


    def smoothedNoise(self, x):
        return self.noise(x) / 2.0 + self.noise(x - 1) / 4.0 + self.noise(x + 1) / 4.0


    def interpolatedNoise(self, x):
        int_x = int(x)
        frac_x = x - int_x

        v1 = self.smoothedNoise(int_x)
        v2 = self.smoothedNoise(int_x + 1)

        return self.cosineInterpolation(v1, v2, frac_x)


    def perlinNoise1D(self, x):
        total = 0.0

        for octave in range(self.number_of_octaves - 1):
            frequency = math.pow(2, octave)
            amplitude = math.pow(self.persistence, octave)

            total += self.interpolatedNoise(x * frequency) * amplitude

        return total


    @staticmethod
    def isPrime(num):
        for j in range(2, int(math.sqrt(num) + 1)):
            if (num % j) == 0:
                return False
        return True

    @staticmethod
    def createPrime(digits=10):
        done = False
        low = int("1" + "0" * (digits - 1))
        high = int("9" * digits)

        if low == 1:
            low = 2

        while not done:
            num = random.randint(low, high)
            if PerlinNoise.isPrime(num):
                return num


def createObservationFile(report_step, observation, count, std=0.2):
    with open("perlin_obs_%d.txt" % report_step, "w") as f:

        for index in range(count):
            x = index / 8.0
            f.write("%f %f\n" % (observation.perlinNoise1D(x), std))



def readParameters(filename):
    params = {}
    with open(filename, "r") as f:
        for line in f:
            key, value = line.split(":", 1)
            params[key] = float(value)

    return params


if __name__ == "__main__":
    count = 100

    # primes = []
    # for p in range(128):
    #     primes.append(str(PerlinNoise.createPrime(7)))
    #
    # print(",".join(primes))

    observations = {1: PerlinNoise(prime_1=15731, prime_2=789221, prime_3=1376312589),
                    2: PerlinNoise(prime_1=8831, prime_2=1300237, prime_3=32416187567),
                    3: PerlinNoise(prime_1=10657, prime_2=105767, prime_3=2902956923)}

    for report_step in observations:
        observation = observations[report_step]
        # createObservationFile(report_step, observation, count)

        params = readParameters("perlin_params.txt")

        scale = params["SCALE"]
        offset = params["OFFSET"]
        octaves = int(round(params["OCTAVES"]))
        persistence = params["PERSISTENCE"]
        p1_index = int(round(params["PRIME_1"]))
        p2_index = int(round(params["PRIME_2"]))
        p3_index = int(round(params["PRIME_3"]))

        with open("perlin_%d.txt" % report_step, "w") as f:
            P1 = PRIME_INDEX_1[p1_index]
            P2 = PRIME_INDEX_2[p2_index]
            P3 = PRIME_INDEX_3[p3_index]
            # P1 = PerlinNoise.createPrime()
            # P2 = PerlinNoise.createPrime()
            # P3 = PerlinNoise.createPrime()
            report_step_noise = PerlinNoise(persistence=persistence, number_of_octaves=octaves, prime_1=P1, prime_2=P2, prime_3=P3)

            for i in range(count):
                x = i / 8.0
                obs = observation.perlinNoise1D(x)
                noise = report_step_noise.perlinNoise1D(x)
                f.write("%f\n" % (obs + offset + noise * scale))
