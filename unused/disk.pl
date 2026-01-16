#!/bin/perl

use warnings "all";
use strict; 

my $gb_green  = "#98971a";
my $gb_yellow = "#d79921";
my $gb_orange = "#d65d0e";
my $gb_red    = "#cc241d";

my $disk = $ARGV[0] || '/';
my $thresh1 = $ARGV[1] || 75;
my $thresh2 = $ARGV[2] || 50;

# /dev/nvme1n1p1  1.8T  1.7T   44G  98% /mnt/alias/m2
my $df = `df -h $disk | tail -1`;
chomp($df);

(
  my $disk_path,
  my $total_size,
  my $used_size,
  my $avail_size,
  my $used_perc,
  my $mount_point
) = split(/\s+/, $df);

my @fields = split(/\s+/, $df);

printf("   %s %s", $used_perc, $avail_size);
$used_perc = $used_perc =~ s/%//r;

my $color = $gb_green;

if ($used_perc >= $thresh1) {
    $color = $gb_red;
} elsif ($used_perc >= $thresh2) {
    $color = $gb_orange;
}

print "\n";
print $color;
