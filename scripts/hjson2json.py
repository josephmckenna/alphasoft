#!/usr/bin/env python

import sys
import hjson

def main():
    try:
        ifn = sys.argv[1]
        infile = open(ifn,"r")
    except IndexError:
        sys.stderr.write("Usage: hjson2json <infile> [outfile]\n")
        sys.stderr.write("Omitting outfile argument will result in <infile>.json\n")
        return 1
    except IOError, err:
        sys.stderr.write('ERROR: %s\n' % str(err))
        return 2

    try:
        hj = hjson.load(infile)
        print "Modules listed in file:", hj.keys()
    except AttributeError, err:
        sys.stderr.write('ERROR: %s\n' % str(err))
        sys.stderr.write('This does not look like a usable hjson file\n')
        return 3
    except hjson.HjsonDecodeError, err:
        sys.stderr.write('ERROR: Broken HJSON file: %s\n' % str(err))
        return 4

    try:
        if(len(sys.argv) > 2):
            outfile = open(sys.argv[2],"w")
        else:
            outfile = open(ifn+".json","w")
    except IOError, err:
        sys.stderr.write('ERROR: %s\n' % str(err))
        return 2

    hjson.dumpJSON(hj, outfile, indent=4)


if __name__ == '__main__':
    main()
