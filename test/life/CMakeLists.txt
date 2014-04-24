set(tests
    6x6-glider
    40x80
)
get_target_property(diag-cycle-path diag-cycle LOCATION)
get_target_property(diag-fix-path diag-fix LOCATION)
set(DIAG_CYCLE ${diag-cycle-path})
set(DIAG_FIX ${diag-fix-path})
foreach(test ${tests})
  configure_file("${DIAGONAL_TEST_DIR}/life/${test}.sh.in"
                 "${DIAGONAL_BINARY_DIR}/test/life/${test}.sh"
                 @ONLY)
  add_test(${test} sh "${test}.sh")
endforeach(test ${tests})

add_executable(life life.c)
