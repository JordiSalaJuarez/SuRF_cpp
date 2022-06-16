find_package(benchmark REQUIRED)

add_library(bitvector INTERFACE)
target_include_directories (bitvector INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(         benchmark_bitvector         benchmark_bitvector.cpp )
target_link_libraries(  benchmark_bitvector         benchmark::benchmark    )   
