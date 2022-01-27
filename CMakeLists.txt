cmake_minimum_required(VERSION 3.8)

project(experiments)

set(CMAKE_CXX_STANDARD 17)

# Set up OGDF
add_subdirectory(ogdf)

# GCC seems to detect many false positives for the "stringop-overread" warning.
# Since "-Werror" is enabled, we disable this warning for the OGDF. See
# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=99578 for more information.
if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 11.1)
    target_compile_options(OGDF PRIVATE -Wno-stringop-overread)
endif()

# Add sources for apps
add_subdirectory(apps)
