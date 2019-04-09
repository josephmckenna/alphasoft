#!/usr/bin/env python

import sys
import hjson
from os import environ

def main():
    if 'AGRELEASE' in environ:
        agr = environ.get('AGRELEASE')
    else:
        sys.stderr.write('AGRELEASE environment variable not set, please source agconfig.sh\n')
        return 7

    defaultfilename = "%s/ana/ana_settings.json" % agr

    try:
        defaultfile = open(defaultfilename,"r")
    except IOError, err:
        sys.stderr.write('ERROR: %s\n' % str(err))
        return 2

    try:
        hj_def = hjson.load(defaultfile)
        print "Modules listed in default file:", hj_def.keys()
    except AttributeError, err:
        sys.stderr.write('ERROR: %s\n' % str(err))
        sys.stderr.write('Default settings file %s is missing or unreadable\n' % defaultfile)
        return 3
    except hjson.HjsonDecodeError, err:
        sys.stderr.write('ERROR: Broken default JSON file: %s\n' % str(err))
        return 4

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

    for module, settings in hj_def.items():
        if module in hj:
            for key, value in settings.items():
                if key in hj[module]:
                    hj_def[module][key] = hj[module][key]

    hjson.dumpJSON(hj_def, outfile, indent=4)

if __name__ == '__main__':
    main()
