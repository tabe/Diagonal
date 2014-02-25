set(tests
    diag-cycle-cat
    diag-cycle-add2
    diag-cycle-collatz
    diag-cycle-zero
)
get_target_property(diag-cycle-path diag-cycle LOCATION)
set(DIAG_CYCLE ${diag-cycle-path})
foreach(test ${tests})
  configure_file("${DIAGONAL_TEST_DIR}/diag-cycle/${test}.sh.in"
                 "${DIAGONAL_BINARY_DIR}/test/diag-cycle/${test}.sh"
                 @ONLY)
  add_test(${test} sh "${test}.sh")
endforeach(test ${tests})

add_executable(collatz collatz.c)
