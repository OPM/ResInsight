from opm.io.parser import Parser
from opm.io.ecl_state import EclipseState
from opm.io.schedule import Schedule

def main():
    deck = Parser().parse('../tests/spe3/SPE3CASE1.DATA')
    es = EclipseState(deck)
    sc = Schedule(deck, es)
    wp = sc.get_wells(0)[0] # producer
    wi = sc.get_wells(0)[1] # injector
    print('state:     %s' % es)
    print('schedule:  %s' % sc)
    print('prod well: %s' % wp)
    print('inj  well: %s' % wi)
    for i in range(len(sc.timesteps)):
        if not sc.get_wells(i)[0].isproducer() or sc.get_wells(i)[0].isinjector():
            print('wp is not producer in step %s' % sc.timesteps[i])
        if not sc.get_wells(i)[1].isinjector() or sc.get_wells(i)[1].isproducer():
            print('wi is not injector in step %s' % sc.timesteps[i])

if __name__ == '__main__':
    main()
