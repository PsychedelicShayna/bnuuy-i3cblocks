#!/bin/perl

use warnings;
use strict;




my $current;

open(my $fh_stat, "<", "/proc/stat");

while (<$fh_stat>) {
    if (/^cpu /) {
        $current = $_;
        last;
    }
}

open(my $fh_tmp, "<", "/tmp/.i3blocks_cpu");
my $prev_line = <$fh_tmp>;
close($fh_tmp);

open(my $fh_tmp_out, ">", "/tmp/.i3blocks_cpu");
print $fh_tmp_out $current;
close($fh_tmp_out);

chomp($current);

my $prev_data = `cat /tmp/.i3blocks_cpu`;

open(my $fh_prev_out, ">", "/tmp/.i3blocks_cpu");
print $fh_prev_out $current . "\n";
close($fh_prev_out);

# my $rev = `cat /tmp/.i3blocks_cpu`;

if (length($prev_data) == 0) {
    open(my $fh_out, ">", "/tmp/.i3blocks_cpu");
    print $fh_out $current . "\n";
    close($fh_out);
    exit(0);
}


my @curr_fields = split(/\s+/, $current);
my @prev_fields = split(/\s+/, $prev_data);

my $idle_now = $curr_fields[4];
my $total_now = 0;

for my $field (@curr_fields[1..$#curr_fields]) {
    $total_now += $field;
}

my $idle_prev = $prev_fields[4];
my $total_prev = 0;

for my $field (@prev_fields[1..$#prev_fields]) {
    $total_prev += $field;
}

my $diff_idle = $idle_now - $idle_prev;
my $diff_total = $total_now - $total_prev;
my $diff_usage = (1000 * ($diff_total - $diff_idle) / $diff_total + 5) / 10;




my $freq_raw = `cat '/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq'`;
my $freq_mhz = $freq_raw / 1000.0;
my $freq = sprintf("%.02fGHz", $freq_mhz / 1000.0);

# Use of uninitialized value $prev_data in scalar chomp at ./cpu.pl statlines 13.
# Use of uninitialized value length($prev_data) in numeric eq (==) at ./cpu.pl statlines 20.

printf("%d%% %s\n", $diff_usage, $freq);
print "\n";

# my $gb_green  = "#98971a";
# my $gb_yellow = "#d79921";
# my $gb_orange = "#d65d0e";
# my $gb_red    = "#cc241d";
#
# if ($diff_usage >= 75) {
#     print $gb_red;
# } elsif ($diff_usage >= 50) {
#     print $gb_orange;
# } else {
#     print $gb_green;
# }
