set(tests
    diag-mode-multi
    diag-mode-null
    diag-mode-one
    diag-mode-three
)
get_target_property(diag-mode-path diag-mode LOCATION)
set(DIAG_MODE ${diag-mode-path})
foreach(test ${tests})
  configure_file("${DIAGONAL_TEST_DIR}/diag-mode/${test}.sh.in"
                 "${DIAGONAL_BINARY_DIR}/test/diag-mode/${test}.sh"
                 @ONLY)
  add_test(${test} sh "${test}.sh")
endforeach(test ${tests})
