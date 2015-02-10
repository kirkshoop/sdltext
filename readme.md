rxcpp v2 sample using SDL
=============================

Builds
======

XCode
-----
```
mkdir projects/build
cd projects/build
cmake -G"Xcode" ../CMake -B.
```

OSX
---
```
mkdir projects/build
cd projects/build
cmake -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=RelWithDebInfo -B. ../CMake
make
```

