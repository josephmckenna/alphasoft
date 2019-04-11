#!/usr/bin/env python

import sys
import hjson
from os import environ, path, getcwd
import getopt

def usage():
    sys.stderr.write("Usage: hjson2json [option] <infile> [outfile]\n")
    sys.stderr.write("Omitting outfile argument will result in <infile>.json\n")
    sys.stderr.write("Options:\n")
    sys.stderr.write("\t-h, --help:          display this help\n")
    sys.stderr.write("\t-f, --force:         overwrite output file without asking\n")
    sys.stderr.write("\t-D, --write-default: write default file from input, instead of merging with default file (implies -f)\n")

def main():
    if 'AGRELEASE' in environ:
        agr = environ.get('AGRELEASE')
    else:
        sys.stderr.write('AGRELEASE environment variable not set, please source agconfig.sh\n')
        return 7

    force = False
    writeDef = False

    try:
        opts, args = getopt.getopt(sys.argv[1:], "hfD", ["help", "force", "write-default"])
    except getopt.GetoptError as err:
        # print help information and exit:
        print str(err)  # will print something like "option -a not recognized"
        usage()
        sys.exit(2)

    for o, a in opts:
        if o in ("-h", "--help"):
            usage()
            sys.exit()
        elif o in ("-D", "--write-default"):
            force = True
            writeDef = True
        elif o in ("-f", "--force"):
            force = True

    defaultfilename = "%s/ana/ana_settings.json" % agr

    try:
        ifn = sys.argv[1+len(opts)]
        infile = open(ifn,"r")
    except IndexError:
        usage()
        return 1
    except IOError, err:
        sys.stderr.write('ERROR: %s\n' % str(err))
        return 2

    if(len(sys.argv) > 2+len(opts)):
        ofn = sys.argv[2+len(opts)]
    else:
        ofn = ifn+".json"

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

    if(path.isfile(ofn) and not force):
        if (path.samefile(ofn, defaultfilename)):
            print "You're trying to overwrite the default settings file, but didn't set -D option!"
        response = raw_input("Do you really want to overwrite the file %s? (y/n)" % ofn)
        if(response != "y"):
            print "OK, exiting here."
            return 17

    if(writeDef):
        hj_def = hj
    else:
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
        outfile = open(ofn, "w")
    except IOError, err:
        sys.stderr.write('ERROR: %s\n' % str(err))
        return 2

    if(hj != hj_def):
        for module, settings in hj_def.items():
            if module in hj:
                for key, value in settings.items():
                    if key in hj[module]:
                        hj_def[module][key] = hj[module][key]

    hjson.dumpJSON(hj_def, outfile, indent=4)

if __name__ == '__main__':
    main()
