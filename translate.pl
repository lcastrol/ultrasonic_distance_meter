#! /usr/bin/perl

use strict;

open my $fh, "<", "/dev/ttyACM0" or die $!;
binmode($fh);
open my $file, ">","file";

my $buffer;
my $got;

my $counter  = 0;

while($counter < 1000){
    $got = read( $fh, $buffer, 1, 0 ); 
    if($got == 0){
        #do nothing
    }
    else{
        my $gray = ((unpack "C", $buffer)-3.9) * 1024 * 331 / (16000 * 10);
        print $file $gray;
        print $file "\n";
        $counter += 1; 
    }
}
close $file;
