#!/usr/bin/perl 
#!/usr/local/bin/perl 
# Test memory instructions

use Getopt::Std;
use lib ".";
use tester;
use File::Copy;

cmdline();

@configs = ("-s 8 -E 4 -b 2 -d 9", "-s 4 -E 7 -b 1 -d 3", "-s 2 -E 3 -b 0 -d 6");
@file = ("m1", "m2", "m3", "m4", "m5", "m6", "m7", "m8");

# Create set of forward tests
foreach $t (@file) {
    if ($testcache) {
        foreach $c (@configs) {
            copy("./memory/$t.ys", "./$t.ys")
            &run_test($t, $c);
        }
    } else {
        copy("./memory/$t.ys", "./$t.ys")
        &run_test($t);
    }
}

&test_stat();