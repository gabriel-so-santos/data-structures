file(GLOB TEST_FILES CONFIGURE_DEPENDS *.c)

add_executable(test_runner ${TEST_FILES})

target_link_libraries(test_runner PRIVATE ds)

add_test(NAME Tests COMMAND test_runner)