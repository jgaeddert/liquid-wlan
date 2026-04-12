# build configuration for autotests

set(AUTOTESTS
    annexg_datascramble_autotest
    annexg_framegen_autotest
    datascrambler_autotest
    interleaver_data_autotest
    signalfield_encoder_autotest
    signalfield_interleaver_autotest
    signalfield_pack_autotest
    signalfield_symbolgen_autotest
    wlan_modem_autotest
    wlanframesync_autotest
    )

foreach(example IN LISTS AUTOTESTS)

    add_executable(${example} ${example}.c)
    target_include_directories(${example} PRIVATE ${INCLUDE})
    target_link_libraries(${example} ${LIBNAME})

    # add program as a test
    add_test(NAME ${example} COMMAND ${example})
endforeach()

