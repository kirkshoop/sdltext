cmake_minimum_required(VERSION 2.8)

project(sdltext)

MESSAGE( STATUS "CMAKE_CXX_COMPILER_ID: " ${CMAKE_CXX_COMPILER_ID} )

FIND_PACKAGE(Threads)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    list( APPEND CMAKE_CXX_FLAGS " -std=c++1y -ftemplate-depth=1024 ${CMAKE_CXX_FLAGS}")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    list( APPEND CMAKE_CXX_FLAGS " -std=c++1y ${CMAKE_CXX_FLAGS}")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    list( APPEND CMAKE_CXX_FLAGS " /DUNICODE /D_UNICODE /bigobj ${CMAKE_CXX_FLAGS}")
endif()

find_library(SDL_LIBRARY SDL2)
if (NOT SDL_LIBRARY)
    message(FATAL_ERROR "SDL2 not found")
endif()

find_library(SDL_TTF_LIBRARY SDL2_ttf)
if (NOT SDL_TTF_LIBRARY)
    message(FATAL_ERROR "SDL2_ttf not found")
endif()

# define some folders
get_filename_component(SDLTEXT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" PATH)
get_filename_component(SDLTEXT_DIR "${SDLTEXT_DIR}" PATH)

MESSAGE( STATUS "SDLTEXT_DIR: " ${SDLTEXT_DIR} )


include_directories(${SDLTEXT_DIR}/ext/catch/include ${SDLTEXT_DIR}/ext/rxcpp/Rx/v2/src)


# define the sources of the self test
set(SDLTEXT_TEST_SOURCES
    ${SDLTEXT_DIR}/main.cpp
)

add_executable(circle_test ${SDLTEXT_TEST_SOURCES})
SET_TARGET_PROPERTIES(circle_test PROPERTIES COMPILE_FLAGS "-D TEST")
TARGET_LINK_LIBRARIES(circle_test ${CMAKE_THREAD_LIBS_INIT} ${SDL_LIBRARY} ${SDL_TTF_LIBRARY})

set(SDLTEXT_SOURCES
    ${SDLTEXT_DIR}/main.cpp
)

add_executable(circle ${SDLTEXT_SOURCES})
TARGET_LINK_LIBRARIES(circle ${CMAKE_THREAD_LIBS_INIT} ${SDL_LIBRARY} ${SDL_TTF_LIBRARY})

# configure unit tests via CTest
enable_testing()

add_test(NAME RunTests COMMAND circle_test)

add_test(NAME ListTests COMMAND circle_test --list-tests)
set_tests_properties(ListTests PROPERTIES PASS_REGULAR_EXPRESSION "[0-9]+ test case")

add_test(NAME ListTags COMMAND circle_test --list-tags)
set_tests_properties(ListTags PROPERTIES PASS_REGULAR_EXPRESSION "[0-9]+ tags")
