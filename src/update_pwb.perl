#!/usr/bin/perl -w

my $help = grep(/-h/, @ARGV);
my $dryrun = grep(/-n/, @ARGV);
my $factory = grep(/-f/, @ARGV);
my $program = grep(/-p/, @ARGV);

$help = 1 unless @ARGV > 0;

die "Usage: $0 [-h] [-n] [-p] {all|pwb01 pwb02 ...}\n" if $help;

#my $fw = "/home/agdaq/online/firmware/pwb_rev1/feam-2018-01-24/feam_auto.rpd";
#my $fw = "/home/agdaq/online/firmware/pwb_rev1/feam-2018-03-12-test/feam_rev1_auto.rpd";
#my $fw = "/home/agdaq/online/firmware/pwb_rev1/pwb_rev1_20180531_cabf9d3d_bryerton/feam_rev1_auto.rpd";
#my $fw = "/home/agdaq/online/firmware/pwb_rev1/pwb_rev1_20180613_test/feam_rev1_auto.rpd";
#my $fw = "/home/agdaq/online/firmware/pwb_rev1/pwb_rev1_20180628_ae04285d/feam_rev1_auto.rpd";
#my $fw = "/home/agdaq/online/firmware/pwb_rev1/pwb_rev1_20180808_0f5edf1b_bryerton/feam_rev1_auto.rpd";
#my $fw = "/home/agdaq/online/firmware/pwb_rev1/pwb_rev1_20180912_6c3810a7_bryerton/feam_rev1_auto.rpd";
#my $fw = "/home/agdaq/online/firmware/pwb_rev1/pwb_rev1_20180913_a8b51569_bryerton/feam_rev1_auto.rpd";
#my $fw = "/home/agdaq/online/firmware/pwb_rev1/pwb_rev1_20190920_ko/feam_rev1_auto.rpd";
#my $fw = "/home/agdaq/online/firmware/pwb_rev1/pwb_rev1_20191007_ko/feam_rev1_auto.rpd";
#my $fw = "/home/agdaq/online/firmware/pwb_rev1/pwb_rev1_20191101_ko/feam_rev1_auto.rpd";
#my $fw = "/home/agdaq/online/firmware/pwb_rev1/pwb_rev1_20191125_ko/feam_rev1_auto.rpd";
#my $fw = "/home/agdaq/online/firmware/pwb_rev1/pwb_rev1_20200128_ko/feam_rev1_auto.rpd";
#my $fw = "/home/agdaq/online/firmware/pwb_rev1/pwb_rev1_20200221_ko/feam_rev1_auto.rpd";
#my $fw = "/home/agdaq/online/firmware/pwb_rev1/pwb_rev1_20200706_ko/feam_rev1_auto.rpd";
my $fw = "/home/agdaq/online/firmware/pwb_rev1/pwb_rev1_20200902_ko/feam_rev1_auto.rpd";
#my $fw = "/home/olchansk/git/pwb_rev1_firmware/bin/feam_rev1_auto.rpd";

die "Cannot read RPD file $fw: $!\n" if ! -r $fw;

foreach my $x (@ARGV)
{
    next if $x =~ /^-/;
    print "update pwb [$x]\n";
    if ($factory) {
      if ($program) {
	print "UPDATING FACTORY IMAGE OF [$x]. ARE YOU SURE? PRESS CTRL-C TO ABORT!\n";
	sleep 10;
      }
      update_factory($fw, $x);
    } else {
      update($fw, $x);
    }
}

if ($ARGV[0] eq "all") {
    update($fw, "pwb12");
    update($fw, "pwb13");
    update($fw, "pwb14");
    update($fw, "pwb02");
    update($fw, "pwb11");
    update($fw, "pwb17");
    update($fw, "pwb18");
    update($fw, "pwb19");

    update($fw, "pwb20");
    update($fw, "pwb21");
    update($fw, "pwb22");
    update($fw, "pwb23");
    update($fw, "pwb24");
    update($fw, "pwb25");
    update($fw, "pwb26");
    update($fw, "pwb27");

    update($fw, "pwb28");
    update($fw, "pwb29");
    update($fw, "pwb08");
    update($fw, "pwb77");
    update($fw, "pwb10");
    update($fw, "pwb33");
    update($fw, "pwb34");
    update($fw, "pwb35");

    update($fw, "pwb36");
    update($fw, "pwb37");
    update($fw, "pwb38");
    update($fw, "pwb39");
    update($fw, "pwb76");
    update($fw, "pwb41");
    update($fw, "pwb42");
    update($fw, "pwb40");

    update($fw, "pwbxx");
    update($fw, "pwbxx");
    update($fw, "pwbxx");
    update($fw, "pwb78");
    update($fw, "pwb03");
    update($fw, "pwb04");
    update($fw, "pwb07");
    update($fw, "pwb15");

    update($fw, "pwb52");
    update($fw, "pwb53");
    update($fw, "pwb54");
    update($fw, "pwb55");
    update($fw, "pwb56");
    update($fw, "pwb57");
    update($fw, "pwb58");
    update($fw, "pwb01");

    update($fw, "pwb60");
    update($fw, "pwb00");
    update($fw, "pwb06");
    update($fw, "pwb63");
    update($fw, "pwb64");
    update($fw, "pwb65");
    update($fw, "pwb66");
    update($fw, "pwb67");

    update($fw, "pwb68");
    update($fw, "pwb69");
    update($fw, "pwb70");
    update($fw, "pwb71");
    update($fw, "pwb72");
    update($fw, "pwb73");
    update($fw, "pwb74");
    update($fw, "pwb75");
}

exit 0;

sub update_factory
{
   my $fw = shift @_;
   my $pwb = shift @_;

   if ($program) {
     print "programming factory page [$pwb] ...\n";
     die "PWB FACTORY PAGE UPDATE IS DISABLED HERE. PLEASE COMMENT-OUT THIS LINE TO ENABLE IT!";
     my $cmd = sprintf("esper-tool -v write -d true http://%s update allow_write", $pwb);
     print $cmd,"\n";
     system $cmd unless $dryrun;
     $cmd = sprintf("esper-tool -v write -d true http://%s update allow_factory_write", $pwb);
     print $cmd,"\n";
     system $cmd unless $dryrun;
   } else {
     print "verifying factory page [$pwb] ...\n";
     my $cmd = sprintf("esper-tool -v write -d false http://%s update allow_write", $pwb);
     print $cmd,"\n";
     system $cmd unless $dryrun;
     $cmd = sprintf("esper-tool -v write -d false http://%s update allow_factory_write", $pwb);
     print $cmd,"\n";
     system $cmd unless $dryrun;
   }

   $cmd = sprintf("esper-tool -v upload -f %s http://%s update factory_rpd", $fw, $pwb);
   print $cmd,"\n";
   system $cmd." &" unless $dryrun;
}

sub update
{
   my $fw = shift @_;
   my $pwb = shift @_;

   if ($program) {
     print "programming user page [$pwb] ...\n";
     my $cmd = sprintf("esper-tool -v write -d true http://%s update allow_write", $pwb);
     print $cmd,"\n";
     system $cmd unless $dryrun;
   } else {
     print "verifying user page [$pwb] ...\n";
     my $cmd = sprintf("esper-tool -v write -d false http://%s update allow_write", $pwb);
     print $cmd,"\n";
     system $cmd unless $dryrun;
   }

   $cmd = sprintf("esper-tool -v upload -f %s http://%s update file_rpd", $fw, $pwb);
   print $cmd,"\n";
   system $cmd." &" unless $dryrun;
}

# end
