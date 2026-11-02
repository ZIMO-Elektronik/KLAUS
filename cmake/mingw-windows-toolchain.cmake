set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR AMD64)
set(CMAKE_LIBRARY_ARCHITECTURE x86_64-w64-mingw32)

# Set mingw as compiler
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_OBJDUMP x86_64-w64-mingw32-objdump)
set(CMAKE_GET_RUNTIME_DEPENDENCIES_PLATFORM "windows+pe")

set(CMAKE_LIBRARY_PATH /usr/x86_64-w64-mingw32/lib
                       /usr/lib/gcc/x86_64-w64-mingw32/13-win32)

# Corrosion Rust traget triple
set(Rust_CARGO_TARGET x86_64-pc-windows-gnu)
