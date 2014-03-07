set(tests
    diag-repeat-head
)
get_target_property(diag-repeat-path diag-repeat LOCATION)
set(DIAG_REPEAT ${diag-repeat-path})
foreach(test ${tests})
  configure_file("${DIAGONAL_TEST_DIR}/diag-repeat/${test}.sh.in"
                 "${DIAGONAL_BINARY_DIR}/test/diag-repeat/${test}.sh"
                 @ONLY)
  add_test(${test} sh "${test}.sh")
endforeach(test ${tests})
