#! /usr/bin/perl

use strict;

open my $fh, "<", "/dev/ttyACM0" or die $!;
binmode($fh);

my $buffer;
my $got;
my $counter = 0;
#open my $file, ">","file";

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
        #my $calibration = 3.9*4*4; #Heuristics 
        my $calibration = 157; #Heuristics 
        my $delay = (60 * 16); #60 ticks of slave board are *16 ticks of master board  
        #my $calibration = 0;
        my $ticks = (unpack "S", $buffer); 
        if ($ticks != 65535){
            my $mm = ($ticks-$calibration-$delay) * 10 * 64 * 331 / (16000 * 10);
            my $mm_round = int(($mm/2) + 0.5);
            my $ms = ($ticks-$calibration-$delay) * 64 / (16000);
            #my $gray = (unpack "C", $buffer);

            #print "\033[2J";    #clear the screen
            #print "\033[0;0H"; #jump to 0,0

            print $ms ;
            print " ms\n";
            print "$mm_round mm\n" ;
            print $ticks;
            print " ticks\n";
            print  "\n";


        } else {
            print "OUT OFF LIMIT\n";
        }
    }

    $counter += 1;
}
#close $file
