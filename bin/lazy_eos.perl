#!/usr/bin/perl -w

#my $confDisks = "/mnt/xenon01_*";
#my $confSelectedDisk = $ENV{"HOME"}."/.lazy_copy.dat";

#my $confCastorPath = "/castor/cern.ch/alpha/2007/data";
#my $confCastorPath = "/castor/cern.ch/alpha/2009/data";
#my $confCastorPath = "/castor/cern.ch/alpha/2010/data";
#my $confCastorPath = "/castor/cern.ch/alpha/2011/data";
#my $confCastorPath = "/castor/cern.ch/alpha/2014/data";
#my $confCastorPath = "/castor/cern.ch/alpha/2015/data";
my $confEOSPath = "/eos/experiment/ALPHAg/midasdata";

$| = 1;

my $KiB = 1024;
my $MiB = 1024*1024;
my $GiB = 1024*1024*1024;

my $eos = "/usr/bin/eos"; # note - overwritten below
die "Program eos [$eos] does not exist: $!\n" unless -x $eos;

# eosls should happen from alphacdr
$eosls = "ssh -x alphacdr\@alphagdaq \"export EOS_MGM_URL=root://eospublic.cern.ch && /usr/bin/eos ls";

my $eoscp = "/usr/bin/eoscp"; # note - overwritten below
die "Program eoscp [$eoscp] does not exist: $!\n" unless -x $eoscp;

# rfcp has to run from alphacdr to satisfy castor access permission - files are owned by alphacdr, not alpha.
$eoscp = "ssh -x alphacdr\@alphagdaq \"export EOS_MGM_URL=root://eospublic.cern.ch && eos cp -n ";

#$ENV{"STAGE_SVCCLASS"} = "default";
#$ENV{"STAGE_HOST"} = "castorpublic";

#$rfcp = "/usr/bin/strace -f $rfcp";

# Uncomment following two lines to run castor on alphacpc09:
#$nsls = "ssh alphacdr\@alphacpc09 /usr/bin/nsls";
#$rfcp = "ssh alphacdr\@alphacpc09 /usr/bin/rfcp";


my $file = shift @ARGV;
my @file = split(/\//, $file);
my $fname = pop @file;

die "File [$file] is not readable: $!\n" unless -r $file;

my $size = -s $file;

print "Backup $fname $size bytes $file\n";

open(MYOUTFILE, ">>/tmp/file.out");
print MYOUTFILE "Backup $fname $size bytes $file\n";

my $isThere = checkFile($fname, $size);
 
if ($isThere)
  {
    # nothing to do
    print "File $fname already in EOS!\n";
    exit 0;
  }

my $dfile = $confEOSPath . "/" . $fname;

print "Backup $fname $size bytes $file to $dfile\n";

#my $cmd = "ssh alphacdr\@alphacpc09 /usr/bin/time rfcp $file $dfile";
my $cmd = "/usr/bin/time $eoscp $file $dfile \"";
print "Run $cmd\n";
system $cmd;

my $check = checkFile($fname, $size);

if (!$check)
  {
    print "Cannot confirm that file was copied to EOS!\n";
    exit 1;
  }

exit 0;

sub checkFile
  {
    my $file = shift @_;
    my $size = shift @_;

    my $cmd = "$eosls -l $confEOSPath/$file \"";
    print "Run $cmd\n";
    my $ls = `$cmd 2>&1`;
#    print "[$ls]\n";

    return 0 if ($ls =~ /No such file or directory/);

    my ($perm, $nlink, $uid, $gid, $xsize) = split(/\s+/, $ls);
#    print "size [$size] [$xsize]\n";
    return 1 if ($size == $xsize);
    
    # file not found, or wrong size
    return 0;
  }

#end
