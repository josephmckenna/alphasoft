#!/usr/bin/perl -w

#my $confDisks = "/mnt/xenon01_*";
#my $confSelectedDisk = $ENV{"HOME"}."/.lazy_copy.dat";

#my $confCastorPath = "/castor/cern.ch/alpha/2007/data";
#my $confCastorPath = "/castor/cern.ch/alpha/2009/data";
#my $confCastorPath = "/castor/cern.ch/alpha/2010/data";
#my $confCastorPath = "/castor/cern.ch/alpha/2011/data";
#my $confCastorPath = "/castor/cern.ch/alpha/2014/data";
#my $confCastorPath = "/castor/cern.ch/alpha/2015/data";
#my $confCastorPath = "/castor/cern.ch/alpha/2016/data";

# staring 2017, use xrdcp instead of rfcp:
# xrdcp /alpha/data/alpha/current/run49194sub00000.mid.gz root://castorpublic.cern.ch//castor/cern.ch/alpha/2017/data/run49194sub00000.mid.gz
my $confCastorPath = "/castor/cern.ch/alpha/alphag_2018/data";
my $confXrdcpPath = "root://castorpublic.cern.ch/" . $confCastorPath;

$| = 1;

my $KiB = 1024;
my $MiB = 1024*1024;
my $GiB = 1024*1024*1024;

my $nsls = "/usr/bin/nsls"; # note - overwritten below
die "Program nsls [$nsls] does not exist: $!\n" unless -x $nsls;

# nsls should happen from alphacdr
$nsls = "ssh -x alphacdr\@alphagdaq /usr/bin/nsls";

#my $rfcp = "/usr/bin/rfcp"; # note - overwritten below
#die "Program rfcp [$rfcp] does not exist: $!\n" unless -x $rfcp;

my $xrdcp = "/usr/bin/xrdcp";
die "Program xrdcp [$xrdcp] does not exist: $!\n" unless -x $xrdcp;

# rfcp has to run from alphacdr to satisfy castor access permission - files are owned by alphacdr, not alpha.
$xrdcp = "ssh -x alphacdr\@alphagdaq /usr/bin/xrdcp --nopbar";

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
    print "File $fname already in castor!\n";
    exit 0;
  }

my $dfile = $confXrdcpPath . "/" . $fname;

print "Backup $fname $size bytes $file to $dfile\n";

my $cmd = "/usr/bin/time $xrdcp $file $dfile";
print "Run $cmd\n";
system $cmd;

my $check = checkFile($fname, $size);

if (!$check)
  {
    print "Cannot confirm that file was copied to castor!\n";
    exit 1;
  }

exit 0;

sub checkFile
  {
    my $file = shift @_;
    my $size = shift @_;

    my $cmd = "$nsls -l $confCastorPath/$file";
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
