cmake_minimum_required (VERSION 2.6)
project (msspeech-gbridge)

set(CMAKE_BUILD_TYPE "Debug")

set(Compiler_Flags_Common_Debug "-std=c++11 -O0 -g -frtti -Wall")
set(Compiler_Flags_Common_Release "-std=c++11 -O3 -DNDEBUG -frtti -Wall")

set(CMAKE_COMPILER_IS_GNUCC TRUE)

set(CMAKE_C_FLAGS_DEBUG "${Compiler_Flags_C} ${Compiler_Flags_Common_Debug} ${Compile_Options_Debug}")
set(CMAKE_C_FLAGS_RELEASE "${Compiler_Flags_C} ${Compiler_Flags_Common_Release} ${Compile_Options_Release}")

set(CMAKE_CXX_FLAGS_DEBUG "${Compiler_Flags_CPP} ${Compiler_Flags_Common_Debug} ${Compile_Options_Debug}")
set(CMAKE_CXX_FLAGS_RELEASE "${Compiler_Flags_CPP} ${Compiler_Flags_Common_Release} ${Compile_Options_Release}")

include_directories("googleapis/output")

FILE(GLOB_RECURSE GoogleOutput googleapis/output *.cc)
add_library(Google STATIC ${GoogleOutput})

FILE(GLOB BridgeFiles *.cpp)

add_executable(msspeech-gbridge ${BridgeFiles})
target_link_libraries(msspeech-gbridge Google pthread grpc grpc++ protobuf msspeech fmt websockets uuid json-c)

