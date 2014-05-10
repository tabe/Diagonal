set(tests
    diag-times-cat
)
get_target_property(diag-times-path diag-times LOCATION)
set(DIAG_TIMES ${diag-times-path})
foreach(test ${tests})
  configure_file("${DIAGONAL_TEST_DIR}/diag-times/${test}.sh.in"
                 "${DIAGONAL_BINARY_DIR}/test/diag-times/${test}.sh"
                 @ONLY)
  add_test(${test} sh "${test}.sh")
endforeach(test ${tests})
