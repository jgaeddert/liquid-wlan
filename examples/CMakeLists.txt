# build configuration for examples

set(EXAMPLES
    wlanframegen_example
    wlanframesync_example
    wlanframesync_performance_example
    )

foreach(example IN LISTS EXAMPLES)

    add_executable(${example} ${example}.c)
    target_include_directories(${example} PRIVATE ${INCLUDE})
    target_link_libraries(${example} ${LIBNAME})

    # add example program as a test
    add_test(NAME ${example} COMMAND ${example})
endforeach()

