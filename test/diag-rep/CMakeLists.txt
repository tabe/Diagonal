set(tests
    diag-rep-head
    diag-rep-head0
)
get_target_property(diag-rep-path diag-rep LOCATION)
set(DIAG_REP ${diag-rep-path})
foreach(test ${tests})
  configure_file("${DIAGONAL_TEST_DIR}/diag-rep/${test}.sh.in"
                 "${DIAGONAL_BINARY_DIR}/test/diag-rep/${test}.sh"
                 @ONLY)
  add_test(${test} sh "${test}.sh")
endforeach(test ${tests})
