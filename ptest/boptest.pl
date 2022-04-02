#!/usr/bin/perl 
#!/usr/local/bin/perl 
# Test memory instructions

use Getopt::Std;
use lib ".";
use tester;
use File::Copy;

cmdline();

@file = ("leaq", "sarq", "shlq", "shrq", "vecadd");

# Create set of forward tests
foreach $t (@file) {
    copy("./bonus/$t.ys", "./$t.ys")
    &run_test($t);
}

&test_stat();