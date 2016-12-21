#! /usr/bin/perl

use strict;

open my $fh, "<", "/dev/ttyACM0" or die $!;
binmode($fh);

my $buffer;
my $got;
my $counter = 0;
open my $file, ">","file";

while($counter < 1000){

    my $start_symbol;
    while ($start_symbol != 83 ) {
        $got = read($fh, $buffer, 1, 0);
        if($got == 0){
            #do nothing
        } else{
            $start_symbol = (unpack "C", $buffer);
        }
    #print "im here\n";
    }

    $got = read( $fh, $buffer, 2, 0 ); 
    if($got == 0){
        #do nothing
    }
    else{
        my $calibration = 3.9*4*4;
        #my $calibration = 0;
        my $ticks = (unpack "S", $buffer); 
        if ($ticks != 65535){
            my $cm = ($ticks-$calibration) * 64 * 331 / (16000 * 10);
            my $ms = $ticks * 64 / (16000);
            #my $gray = (unpack "C", $buffer);
            print $ms ;
            print " ms\n";
            print $cm ;
            print " cm\n";
            print $ticks;
            print " ticks\n";
            #print "\n";
        } else {
            print "OUT OFF LIMIT\n";
        }
    }

    $counter += 1;
}