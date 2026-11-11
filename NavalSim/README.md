```
cmake --preset default
cmake --build --preset debug
ctest --test-dir build --output-on-failure
```


./build/Debug/navalsim
./build/Debug/navalsim_clock_tests
./build/Debug/navalsim_scheduler_tests
ctest --test-dir build --output-on-failure
ctest --test-dir build -N



git merge --no-ff branchname


zip -r navalsim-5.1-review.zip . \
  -x ".git/*" "build/*" "cmake-build-*/*"

