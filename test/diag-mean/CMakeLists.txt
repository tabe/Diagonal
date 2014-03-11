set(tests
    diag-mean-null
    diag-mean-one
    diag-mean-two
)
get_target_property(diag-mean-path diag-mean LOCATION)
set(DIAG_MEAN ${diag-mean-path})
foreach(test ${tests})
  configure_file("${DIAGONAL_TEST_DIR}/diag-mean/${test}.sh.in"
                 "${DIAGONAL_BINARY_DIR}/test/diag-mean/${test}.sh"
                 @ONLY)
  add_test(${test} sh "${test}.sh")
endforeach(test ${tests})
