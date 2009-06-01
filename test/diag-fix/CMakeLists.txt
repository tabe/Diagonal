set(tests
    diag-fix-cat
)
get_target_property(diag-fix-path diag-fix LOCATION)
set(DIAG_FIX ${diag-fix-path})
foreach(test ${tests})
  configure_file("${DIAGONAL_TEST_DIR}/diag-fix/${test}.sh.in"
                 "${DIAGONAL_BINARY_DIR}/test/diag-fix/${test}.sh"
                 @ONLY)
  add_test(${test} sh "${test}.sh")
endforeach(test ${tests})
