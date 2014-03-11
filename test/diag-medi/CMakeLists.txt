set(tests
    diag-medi-null
    diag-medi-one
    diag-medi-two
    diag-medi-1000
    diag-medi-1001
)
get_target_property(diag-medi-path diag-medi LOCATION)
set(DIAG_MEDI ${diag-medi-path})
foreach(test ${tests})
  configure_file("${DIAGONAL_TEST_DIR}/diag-medi/${test}.sh.in"
                 "${DIAGONAL_BINARY_DIR}/test/diag-medi/${test}.sh"
                 @ONLY)
  add_test(${test} sh "${test}.sh")
endforeach(test ${tests})
add_executable(median-rand median-rand.c)
