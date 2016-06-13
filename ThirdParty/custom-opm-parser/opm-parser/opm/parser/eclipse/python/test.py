#!/usr/bin/env python
import sys
sys.path += ["/private/joaho/work/OPM/opm-parser/opm/parser/eclipse/python/python"]
from opm.parser.parser import Parser

p = Parser()

deck = p.parseFile( sys.argv[1] )    
print "Number of keywords in deck: %s" % len(deck)

for kw in deck:
    print "%s:%d" % (kw.name() , len(kw))
