set(GCOV_PATH ${CMAKE_SOURCE_DIR}/cmake/llvm-gcov.sh)

find_program(LCOV_PATH lcov)
if (NOT LCOV_PATH)
    message(FATAL_ERROR "lcov not found!")
endif ()

find_program(GENHTML_PATH genhtml)
if (NOT GENHTML_PATH)
    message(FATAL_ERROR "genhtml not found!")
endif ()

function(create_coverage_target _target_name)
    set(PREPARE_COVERAGE_ENV ${_target_name}_prepare_env)
    set(RUN_TESTS ${_target_name}_run_tests)
    set(CREATE_COVERAGE_REPORT ${_target_name}_create_coverage_report)

    add_custom_target(${PREPARE_COVERAGE_ENV}
        ${LCOV_PATH} --gcov-tool ${GCOV_PATH} --directory . --zerocounters
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

    add_custom_target(${RUN_TESTS}
        DEPENDS ${PREPARE_COVERAGE_ENV}
    )
    add_custom_target(${CREATE_COVERAGE_REPORT}
        DEPENDS ${RUN_TESTS}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}

        COMMAND ${LCOV_PATH} --gcov-tool ${GCOV_PATH} --directory . --base-directory . --capture --output-file ${_target_name}.info
        COMMAND ${LCOV_PATH} --gcov-tool ${GCOV_PATH} --remove ${_target_name}.info '/usr*' '*/tests/*' '*/ext/*' -o ${_target_name}.cleaned
        COMMAND ${GENHTML_PATH} -o ${_target_name} ${_target_name}.cleaned
    )

    add_custom_target(${_target_name}
        DEPENDS ${PREPARE_COVERAGE_ENV} ${RUN_TESTS} ${CREATE_COVERAGE_REPORT}
    )
endfunction()

function(add_test_target _coverage_target _target_name)
    set(RUNNER_TARGET ${_target_name}_runner)
    add_custom_target(${RUNNER_TARGET}
        DEPENDS ${_coverage_target}_prepare_env ${_target_name}
        COMMAND ${_target_name}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    add_dependencies(${_coverage_target}_run_tests ${RUNNER_TARGET})
endfunction()
