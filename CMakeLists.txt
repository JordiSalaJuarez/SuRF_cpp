cmake_minimum_required(VERSION 3.14)
project(surf_cpp)

# Add local libraries path to the build 
link_directories(/usr/local/lib)

find_package(benchmark REQUIRED)
find_package(fmt REQUIRED)

# find_package(dl REQUIRED)
# find_package(boost_system REQUIRED)
# find_package(art REQUIRED)
# find_package(tlx REQUIRED)

# GoogleTest requires at least C++11
set(CMAKE_CXX_STANDARD 20) 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=haswell -ftree-vectorize -ffast-math")

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
# configure_file(data/keys_1M data/keys_1M COPYONLY)
configure_file(data/insert_uniform data/insert_uniform COPYONLY)
configure_file(data/insert_uniform data/insert_zipfian COPYONLY)
configure_file(data/query_zipfian data/query_zipfian COPYONLY)
configure_file(data/query_uniform data/query_uniform COPYONLY)
configure_file(data/insert_words data/insert_words COPYONLY)
configure_file(data/query_words data/query_words COPYONLY)
configure_file(data/dna.txt data/dna.txt COPYONLY)
configure_file(data/wiki.txt data/wiki.txt COPYONLY)
configure_file(data/geo_names.txt data/geo_names.txt COPYONLY)


enable_testing()



add_executable(
  surf_test
  "surf/test.cpp"
)
target_link_libraries(surf_test gtest_main)
target_link_libraries(surf_test ${OPENSSL_LIBRARIES} dl)
target_link_libraries(surf_test boost_system)
add_executable(
  test_bit_vector
  "surf/test/test_bit_vector.cpp"
)

target_link_libraries(test_bit_vector gtest_main)


include(GoogleTest)
gtest_discover_tests(surf_test)
gtest_discover_tests(test_bit_vector)

add_subdirectory(tlx)

add_subdirectory(surf_paper)

add_subdirectory(benchmark)

# add_subdirectory(surf/bitvector)