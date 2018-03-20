#!/usr/bin/perl -w

#my $fw = "/home/agdaq/online/firmware/pwb_rev1/feam-2018-01-24/feam_auto.rpd";
my $fw = "/home/agdaq/online/firmware/git/adc_firmware/bin/alpha16_one_page_auto.rpd";

foreach my $x (@ARGV)
{
    print "update adc [$x]\n";
    update($fw, $x);
}

exit 0;

sub update
{
   my $fw = shift @_;
   my $pwb = shift @_;
   my $cmd = sprintf("esper-tool -v upload -f %s http://%s update file_rpd", $fw, $pwb);
   print $cmd,"\n";
   system $cmd." &";
}

# end
