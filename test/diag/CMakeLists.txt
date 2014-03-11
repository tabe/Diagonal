set(tests
    diag-option-l
    diag-command-cycle
    diag-command-fix
    diag-command-mean
    diag-command-root
)
get_target_property(diag-path diag LOCATION)
set(DIAG_COMMAND ${diag-path})
foreach(test ${tests})
  configure_file("${DIAGONAL_TEST_DIR}/diag/${test}.sh.in"
                 "${DIAGONAL_BINARY_DIR}/test/diag/${test}.sh"
                 @ONLY)
  add_test(${test} sh "${test}.sh")
endforeach(test ${tests})
