### Test Compile
```
./configure --disable-shared CC=clang --with-sanitizer="-fsanitize=address,fuzzer-no-link" --with-fuzzer-engine="-fsanitize=fuzzer"
```

### local oss-fuzz test
```
python oss-fuzz/infra/helper.py build_fuzzers libmodbus --engine libfuzzer --sanitizer address --architecture x86_64
```
