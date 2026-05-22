# Set variables to Windows on AMD64
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR AMD64)

# Set clang as compiler
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)

# set(CMAKE_EXE_LINKER_FLAGS_INIT "-static -static-libgcc -static-libstdc++")
# set(CMAKE_SHARED_LINKER_FLAGS_INIT "-static -static-libgcc -static-libstdc++")

set(CMAKE_EXE_LINKER_FLAGS_INIT
    "-static -static-libgcc -static-libstdc++ -Wl,--allow-multiple-definition")
set(CMAKE_SHARED_LINKER_FLAGS_INIT
    "-static -static-libgcc -static-libstdc++ -Wl,--allow-multiple-definition")

# Corrosion Rust traget triple
set(Rust_CARGO_TARGET x86_64-pc-windows-gnu)
