#!/usr/bin/perl 
#!/usr/local/bin/perl 
# Test memory instructions

use Getopt::Std;
use lib ".";
use tester;
use File::Copy;

cmdline();

@configs = ("-s 1 -E 1 -b 3 -d 5", "-s 0 -E 16 -b 4 -d 7", "-s 4 -E 2 -b 4 -d 3", "-s 5 -E 1 -b 5 -d 10", "-s 2 -E 4 -b 3 -d 8", "-s 2 -E 2 -b 3 -d 2", "-s 2 -E 1 -b 3 -d 6", "-s 2 -E 1 -b 4 -d 9");
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