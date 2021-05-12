INCLUDE(CMakeForceCompiler)

SET(CMAKE_SYSTEM_NAME Linux)     
SET(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)     

SET(CMAKE_C_COMPILER $ENV{OE_BIN}/aarch64-oe-linux/aarch64-oe-linux-gcc)

# this is the file system root of the target

SET(OE_ROOT /usr/local/oecore-x86_64/sysroots/armv7ahf-neon-oe-linux-gnueabi)

SET(CMAKE_FIND_ROOT_PATH $ENV{OE_ROOT})

# do not search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

