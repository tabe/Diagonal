set(subdirectories
    bytevector
    datum
    deque
    rbtree
    set
    vector
    trie
    hash
    hashtable
    port
    hamming
    levenshtein
    singlelinkage
    line
    vcdiff
    imf
    diag-avg
    diag-fix
    diag
)
set(DIAGONAL_TEST_DIR "${DIAGONAL_SOURCE_DIR}/test")
include_directories(BEFORE "${DIAGONAL_BINARY_DIR}/test")
configure_file("${DIAGONAL_TEST_DIR}/test.h.cmake" "${DIAGONAL_BINARY_DIR}/test/test.h")
link_directories("${DIAGONAL_BINARY_DIR}")
foreach(subdir ${subdirectories})
  add_subdirectory(${subdir})
endforeach(subdir ${subdirectories})
