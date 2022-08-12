#### Export flags
```
export CC=clang
export CXX=clang++
export CFLAGS="-fsanitize=fuzzer-no-link,address -g -O2"
export LIB_FUZZING_ENGINE=-fsanitize=fuzzer
export LDFLAGS=-fsanitize=address 
```

#### Compile application
```
./autogen.sh
./configure --disable-shared CC="$CC" CFLAGS="$CFLAGS" LDFLAGS="$CFLAGS"
make -j$(nproc)
```

#### Compile Fuzzer 
```
cd fuzzer
make all
```

#### Run Fuzzer 
```
mkdir coverage
unzip input.zip

FuzzClient coverage/ input/ 
FuzzServer coverage/ input/
```