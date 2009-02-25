add_library(diagonal SHARED
            cluster.c
            code.c
            deque.c
            diagonal.c
            hamming.c
            imf.c
            levenshtein.c
            port.c
            rbtree.c
            vcdiff.c
            )
add_subdirectory(modules)

add_executable(diag diag.c)
target_link_libraries(diag diagonal)
add_executable(diag-dec diag-dec.c)
target_link_libraries(diag-dec diagonal)
add_executable(diag-enc diag-enc.c)
target_link_libraries(diag-enc diagonal)
add_executable(diag-imf diag-imf.c)
target_link_libraries(diag-imf diagonal)
add_executable(diag-log diag-log.c)
target_link_libraries(diag-log diagonal)

install(TARGETS diagonal diag diag-enc diag-imf diag-log
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
