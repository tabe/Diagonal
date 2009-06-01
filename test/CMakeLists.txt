set(subdirectories
    datum
    deque
    rbtree
    port
    hamming
    levenshtein
    line
    vcdiff
    imf
    diag-avg
    diag-fix
)
set(DIAGONAL_TEST_DIR "${DIAGONAL_SOURCE_DIR}/test")
include_directories(BEFORE "${DIAGONAL_BINARY_DIR}/test")
configure_file("${DIAGONAL_TEST_DIR}/test.h.cmake" "${DIAGONAL_BINARY_DIR}/test/test.h")
link_directories("${DIAGONAL_BINARY_DIR}")
foreach(subdir ${subdirectories})
  add_subdirectory(${subdir})
endforeach(subdir ${subdirectories})
