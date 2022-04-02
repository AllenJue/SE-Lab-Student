make
cd ptest
make SIM=../pipe/psim
cd ../cache
./test-csim
cd ../ptest
make test-pipe-cache
cd ..
