cmake_minimum_required(VERSION 3.14)
project(surf_cpp)
# GoogleTest requires at least C++11
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=haswell -ldl")
option(COZ_PROFILING, "Coz profiling" ON)

if (COZ_PROFILING)
  find_package(coz-profiler REQUIRED)
  target_compile_definitions(${PROJECT_NAME} PRIVATE -DCOZ_PROFILING=1)
  target_include_directories(${PROJECT_NAME} PUBLIC ${COZ_INCLUDE_DIRS})
endif(COZ_PROFILING)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# project (surf_file)
# add_executable(surf_file surf/surf_file.cpp)
# configure_file(data/input data/input COPYONLY)
# configure_file(data/load_randint data/load_randint COPYONLY)
# configure_file(data/load_randint data/load_randint_slow COPYONLY)
configure_file(data/keys_1M data/keys_1M COPYONLY)
configure_file(data/keys_100M data/keys_100M COPYONLY)

enable_testing()

add_executable(
  surf_test
  "surf/test.cpp"
)
target_link_libraries(
  surf_test
  gtest_main
)
target_link_libraries(surf_test ${OPENSSL_LIBRARIES} dl)

add_executable(
  test_bit_vector
  "surf/test/test_bit_vector.cpp"
)

target_link_libraries(test_bit_vector gtest_main)


include(GoogleTest)
gtest_discover_tests(surf_test)
gtest_discover_tests(test_bit_vector)


add_subdirectory(tlx)
add_subdirectory(google_benchmark)
add_subdirectory(surf_paper)
add_executable(my_benchmark benchmark/benchmark.cpp)
target_link_libraries(my_benchmark tlx)
target_link_libraries(my_benchmark benchmark::benchmark)
target_link_libraries(my_benchmark ${OPENSSL_LIBRARIES} dl)

add_executable(benchmark_utils benchmark/utils.cpp)
target_link_libraries(benchmark_utils benchmark::benchmark)
target_link_libraries(my_benchmark ${OPENSSL_LIBRARIES} dl)



