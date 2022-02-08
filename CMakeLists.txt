# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#

set(CMAKE_CXX_STANDARD 20)
cmake_minimum_required (VERSION 3.8)


project ("surf_file")

# Include sub-projects.
add_subdirectory ("surf")
configure_file("data/test.txt" "surf/data/test.txt" COPYONLY)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
  hello_test
  "surf/test.cpp"
)
target_link_libraries(
  hello_test
  gtest_main
)

include(GoogleTest)
gtest_discover_tests(hello_test)

