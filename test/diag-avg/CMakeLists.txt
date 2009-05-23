set(tests
    diag-avg-true
    diag-avg-cat
)
get_target_property(diag-avg-path diag-avg LOCATION)
set(DIAG_AVG ${diag-avg-path})
foreach(test ${tests})
  configure_file("${DIAGONAL_TEST_DIR}/diag-avg/${test}.sh.in"
                 "${DIAGONAL_BINARY_DIR}/test/diag-avg/${test}.sh"
                 @ONLY)
  add_test(${test} sh "${test}.sh")
endforeach(test ${tests})
