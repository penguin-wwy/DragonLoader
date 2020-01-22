# DragonLoader

### Description

C/CPP source code dynamic loader.

The goal is to dynamically load source code and call functions like Jvm ClassLoader.
Achieving this with LLVM Jit.

```c++
using FuncType = void (*)();

void usage() {
    std::string err;
    DragonLoader loader;
    loader.loadSourceFile("/path/to/source", err);
    FuncType func = (FuncType) loader.getNamedFunction("function_name");
    func();
}
```

For detail, please see the example/unittests directory.

Note: Only support c source code for the time being.

### Build

```text
git clone https://github.com/penguin-wwy/DragonLoader.git
cd DragonLoader
mkdir build
cmake .. -DClang_DIR=/path/to/clang+llvm-9/lib/cmake/clang
make
```

Need clang9 and llvm9-library.

If you want to change clang version. Please modify cmake file ```${llvm_libs}```

### Next Work

* Add compile arguments

* Linking symbolic addresses in progress

* Support complex struct

* Support source code string