# DragonLoader

### Description

C/CPP source code dynamic loader.

The goal is to dynamically load source code and call functions like Jvm ClassLoader.
Achieving this with LLVM Jit.

```cpp
void usage() {
    std::string err;
    DragonLoader loader;
    loader.loadSourceFile("/path/to/source", err);
    FuncType func = (FuncType) loader.getNamedFunction("function name");
    func();
}
```

For detail, please see the example/unittests directory.

Note: Only support c source code for the time being.

### Next Work

* Add compile arguments

* Linking symbolic addresses in progress

* Support complex struct

* Support source code string