[Original instructions] (https://gist.github.com/1duo/38af1abd68a2c7fe5087532ab968574e)


### Download CMake from: https://cmake.org/download/

```
wget https://cmake.org/files/v3.12/cmake-3.12.3.tar.gz
```

### Compile from source and install

```
tar zxvf cmake-3.*
cd cmake-3.*
./bootstrap --prefix=/usr/local
make -j$(nproc)
make install
```

### Validate installation

```
cmake --version

cmake version *.*.*
CMake suite maintained and supported by Kitware (kitware.com/cmake).
```
