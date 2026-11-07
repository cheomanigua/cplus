```
cmake --preset default
cmake --build --preset debug
./build/Debug/navalsim
```


./build/Debug/navalsim
./build/Debug/navalsim_clock_tests
./build/Debug/navalsim_scheduler_tests
ctest --test-dir build --output-on-failure

