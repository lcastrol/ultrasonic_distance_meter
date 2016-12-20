#! /usr/bin/perl

use strict;

open my $fh, "<", "/dev/ttyACM0" or die $!;
binmode($fh);

my $buffer;
my $got;
my $counter = 0;
open my $file, ">","file";

while($counter < 1000){
    $got = read( $fh, $buffer, 1, 0 ); 
    if($got == 0){
        #do nothing
    }
    else{
        my $calibration = 3.9*4;
        my $ticks = (unpack "C", $buffer); 
        if ($ticks != 255){
            my $cm = ($ticks-$calibration) * 256 * 331 / (16000 * 10);
            #my $gray = (unpack "C", $buffer) * 1024  / (16000 );
            #my $gray = (unpack "C", $buffer);
            print $file $cm ;
            print $file " cm\n";
            #print $ticks;
            #print "\n";
        } else {
            print $file "OUT OFF LIMIT\n";
        }
    }

    $counter += 1;
}
