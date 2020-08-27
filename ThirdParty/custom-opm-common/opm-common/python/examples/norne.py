#!/usr/bin/env python

import sys
from os.path import isdir, join
from datetime import datetime as dt

import opm.io
from opm.io import Parser, ParseContext
from opm.io.ecl_state import EclipseState
from opm.io.schedule import Schedule
from opm.io.summary import SummaryConfig


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
    e = dt.now()
    print('Parsing took %s sec' % (e - s).seconds)
    return deck

def swof_krw(ecl):
    assert('SWOF' in ecl.tables())
    krw = lambda x: ecl.tables().evaluate('SWOF', 0, 'KRW', x)
    krow = lambda x: ecl.tables().evaluate('SWOF', 0, 'KROW', x)
    pcow = lambda x: ecl.tables().evaluate('SWOF', 0, 'PCOW', x)

    print('SWOF\tKRW\tKROW\tPCOW')
    for i in range(21):
        print('%.2f\t%.4f\t%.4f\t%.4f' % (i/20.0, krw(i/20.0), krow(i/20.0), pcow(i/20.0)))

def main():
    deck = parse(join(opmdatadir(), 'norne/NORNE_ATW2013.DATA'))
    es = EclipseState(deck)
    sc = Schedule(deck, es)
    wp = sc.get_wells(100)[20]
    wi = sc.get_wells(100)[19]
    fn = es.faultNames()
    f0 = fn[0]
    fl = es.faultFaces(f0)
    print('state:     %s' % es)
    print('schedule:  %s' % sc)
    print('the grid:  %s' % es.grid())
    print('at timestep 100 (%s)' % sc.timesteps[100])
    print('prod well: %s' % wp)
    print('inj  well: %s' % wi)
    print('pos:       %s' % list(wp.pos()))
    print('fault:     %s' % f0)
    print('           comprised of %d cells' % len(fl))
    swof_krw(es)

if __name__ == '__main__':
    global OPMDATA_DIR
    OPMDATA_DIR = '../../opm-data'
    if haveopmdata():
        print('Found norne, parsing ...')
        main()
    else:
        print('Need to have path "%s" or give opm-data as argument' % OPMDATA_DIR)
