#!/usr/bin/env python

import sys
from os.path import isdir, join
from datetime import datetime as dt

import numpy as np
import matplotlib.pyplot as plt

import opm.io
from opm.io.parser import Parser, ParseContext
from opm.io.ecl_state import EclipseState
from opm.io.schedule import Schedule


def plotswof(ecl):
    assert('SWOF' in ecl.tables())
    krw  = lambda x: ecl.tables().evaluate('SWOF', 0, 'KRW', x)
    krow = lambda x: ecl.tables().evaluate('SWOF', 0, 'KROW', x)
    pcow = lambda x: ecl.tables().evaluate('SWOF', 0, 'PCOW', x)

    swofl = [x/20.0       for x in range(21)]
    krwl  = [krw(x/20.0)  for x in range(21)]
    krowl = [krow(x/20.0) for x in range(21)]
    pcowl = [pcow(x/20.0) for x in range(21)]

    plt.figure(1)
    plt.plot(swofl, krwl, label = 'KRW')
    plt.plot(swofl, krowl, label = 'KROW')
    plt.legend()
    plt.show()
    plt.figure(2)
    plt.plot(swofl, pcowl, label = 'Water-oil capillary pressure')
    plt.legend()
    plt.show()



def opmdatadir():
    global OPMDATA_DIR
    if isdir(OPMDATA_DIR):
        return OPMDATA_DIR
    if len(sys.argv) < 2:
        return None
    d = sys.argv[1]
    if isdir(d) and isdir(join(d, 'norne')):
        return d
    return None

def haveopmdata():
    return opmdatadir() is not None

def parse(fname):
    s = dt.now()
    ps = ParseContext([('PARSE_RANDOM_SLASH', opm.io.action.ignore)])
    deck = Parser().parse(fname, ps)
    es = EclipseState(deck)
    e = dt.now()
    print('Parsing took %s sec' % (e - s).seconds)
    return es


def main():
    es = parse(join(opmdatadir(), 'norne/NORNE_ATW2013.DATA'))
    plotswof(es)

if __name__ == '__main__':
    global OPMDATA_DIR
    OPMDATA_DIR = '../../opm-data'
    if haveopmdata():
        print('Found norne, parsing ...')
        main()
    else:
        print('Need to have path "%s" or give opm-data as argument' % OPMDATA_DIR)
