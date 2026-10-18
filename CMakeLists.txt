cmake_minimum_required(VERSION 3.18)
project(perf_boost CXX)

set(CMAKE_CXX_STANDARD 20)

add_library(perf_boost SHARED src/PerfBoost.cpp)
target_link_libraries(perf_boost log)

# Build (arm64-v8a only, as LeviLaunchroid requires):
# cmake -B build -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake \
#       -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-28
# cmake --build build
# Output: build/libperf_boost.so
