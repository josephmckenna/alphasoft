#!/usr/bin/perl -w

#my $fw = "/home/agdaq/online/firmware/git/adc_firmware/bin/alpha16_one_page_auto.rpd";
#my $fw = "/home/agdaq/online/firmware/alpha16/20180327-ko/alpha16_one_page_auto.rpd";
#my $fw = "/home/agdaq/online/firmware/alpha16/20180411-ko/alpha16_one_page_auto.rpd";
my $fw = "/home/agdaq/online/firmware/alpha16/20180428-ko/alpha16_one_page_auto.rpd";

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
