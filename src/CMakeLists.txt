add_library(diagonal SHARED
            cluster.c
            code.c
            deque.c
            diagonal.c
            hamming.c
            imf.c
            levenshtein.c
            line.c
            port.c
            rbtree.c
            vcdiff.c
            )
add_subdirectory(modules)

add_executable(diag diag.c)
target_link_libraries(diag diagonal)
add_executable(diag-avg diag-avg.c)
target_link_libraries(diag-avg diagonal)
add_executable(diag-dec diag-dec.c)
target_link_libraries(diag-dec diagonal)
add_executable(diag-enc diag-enc.c)
target_link_libraries(diag-enc diagonal)
add_executable(diag-fix diag-fix.c)
target_link_libraries(diag-fix diagonal)
add_executable(diag-imf diag-imf.c)
target_link_libraries(diag-imf diagonal)
add_executable(diag-line diag-line.c)
target_link_libraries(diag-line diagonal)

install(TARGETS diagonal diag diag-avg diag-dec diag-enc diag-fix diag-imf diag-line
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
