set(tests
    diag-root-cat
)
get_target_property(diag-root-path diag-root LOCATION)
set(DIAG_ROOT ${diag-root-path})
foreach(test ${tests})
  configure_file("${DIAGONAL_TEST_DIR}/diag-root/${test}.sh.in"
                 "${DIAGONAL_BINARY_DIR}/test/diag-root/${test}.sh"
                 @ONLY)
  add_test(${test} sh "${test}.sh")
endforeach(test ${tests})
