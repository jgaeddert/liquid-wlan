# build configuration for sandbox programs

set(SANDBOX
    annexg_fec_test
    datascrambler_test
    interleaver_table_test
    interleaver_test
    packet_codec_test
    signalfield_fec_test
    signalfield_symbolgen_test
    )

foreach(example IN LISTS SANDBOX)

    add_executable(${example} ${example}.c)
    target_include_directories(${example} PRIVATE ${INCLUDE})
    target_link_libraries(${example} ${LIBNAME})

    # add program as a test
    add_test(NAME ${example} COMMAND ${example})
endforeach()

