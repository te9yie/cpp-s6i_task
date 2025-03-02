cmake_minimum_required(VERSION 3.20)
project(s6i_task)


# オプション
option(S6I_TASK_BUILD_TESTS "テストをビルドする" OFF)


include(FetchContent)


# t9
FetchContent_Declare(
    t9
    GIT_REPOSITORY https://github.com/te9yie/cpp-t9.git
    GIT_TAG main
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(t9)

# t9_mem
FetchContent_Declare(
    t9_mem
    GIT_REPOSITORY https://github.com/te9yie/cpp-t9_mem.git
    GIT_TAG main
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(t9_mem)

# s6i_common
FetchContent_Declare(
    s6i_common
    GIT_REPOSITORY https://github.com/te9yie/cpp-s6i_common.git
    GIT_TAG main
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(s6i_common)

# s6i_task
add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE include)
target_link_libraries(${PROJECT_NAME} INTERFACE
    t9
    t9_mem
    s6i_common
)


# テストを有効にする場合
if(S6I_TASK_BUILD_TESTS)
    message(STATUS "enable tests")
    enable_testing()

    # Google Test
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.16.x
        GIT_SHALLOW TRUE
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    # テスト実行ファイルを作成
    add_executable(${PROJECT_NAME}_tests
        tests/resources_test.cpp
    )
    target_precompile_headers(${PROJECT_NAME}_tests PRIVATE tests/pch.h)
    target_link_libraries(${PROJECT_NAME}_tests PRIVATE
        ${PROJECT_NAME}
        GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(${PROJECT_NAME}_tests)
endif()
