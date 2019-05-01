#!/usr/bin/env python

import argparse
from ROOT import TFile, TString, TObjString
import json
from pprint import pprint

from os import environ
from ROOT import gSystem
basedir=environ["AGRELEASE"]
gSystem.Load(basedir+"/recolib/libAGTPC")
gSystem.Load(basedir+"/analib/libagana")

def _decode_list(data):
    rv = []
    for item in data:
        if isinstance(item, unicode):
            item = item.encode('utf-8')
        elif isinstance(item, list):
            item = _decode_list(item)
        elif isinstance(item, dict):
            item = _decode_dict(item)
        rv.append(item)
    return rv

def _decode_dict(data):
    rv = {}
    for key, value in data.iteritems():
        if isinstance(key, unicode):
            key = key.encode('utf-8')
        if isinstance(value, unicode):
            value = value.encode('utf-8')
        elif isinstance(value, list):
            value = _decode_list(value)
        elif isinstance(value, dict):
            value = _decode_dict(value)
        rv[key] = value
    return rv

def get_conf(rootfile):
    data_string=rootfile.Get('ana_settings').GetString().Data()
    data = json.loads(data_string, object_hook=_decode_dict)
    return data


if __name__=='__main__':
 
    parser = argparse.ArgumentParser(description='Read Analysis Configuration')
    parser.add_argument('fname', type=str,
                        help='rootfile name')
    parser.add_argument('-m', '--module', type=str,
                        help='select one module')
    parser.add_argument('-v', '--variable', type=str,
                        help='select one variable')
    
    args = parser.parse_args()

    conf = get_conf(TFile(args.fname))

    if args.module != None:
        mod = args.module
    else:
        mod = ''

    if args.variable != None:
        var = args.variable
    else:
        var = ''

    if len(var) > 0 and len(mod) == 0:
        for m in conf:
            for k in conf[m]:
                if var == k:
                    mod=m
                    break
            if len(mod) > 0:
                break
      
    if len(var) > 0 and len(mod) == 0:
        print "Couldn't find Variable", args.variable, 'in Modules:', ', '.join([k for k in conf.keys()])
    elif len(var) > 0 and len(mod) > 0:
        try:
            conf[mod]
            try:
                pprint( conf[mod][var] )
            except KeyError:
                print 'Unknown Variable', args.variable
                print 'Available Variables in', mod, ':', ', '.join([k for k in conf[mod].keys()])
        except KeyError:
            print 'Unknown Module', args.module
            print 'Available Modules:', ', '.join([k for k in conf.keys()])
    elif len(var) == 0 and len(mod) > 0:
        try:
            pprint( conf[mod] )
        except KeyError:
            print 'Unknown Module', args.module
            print 'Available Modules:', ', '.join([k for k in conf.keys()])
    else:
        pprint( conf )
