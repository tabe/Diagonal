# Mac OS X's RPATH support, started with CMake 2.8.12
# See http://www.kitware.com/blog/home/post/510
set(CMAKE_MACOSX_RPATH 1)

add_library(diagonal SHARED
            bytevector.c
            cluster.c
            code.c
            dataset.c
            datum.c
            deque.c
            diagonal.c
            filesystem.c
            hamming.c
            hash.c
            hashtable.c
            levenshtein.c
            lcs.c
            line.c
            list.c
            memory.c
            pair.c
            port.c
            qselect.c
            rbtree.c
            set.c
            singlelinkage.c
            system.c
            temporary-file.c
            trie.c
            vcdiff.c
            vector.c
            )

add_executable(diag diag.c)
target_link_libraries(diag diagonal)
add_executable(diag-cycle diag-cycle.c)
target_link_libraries(diag-cycle diagonal)
add_executable(diag-dec diag-dec.c)
target_link_libraries(diag-dec diagonal)
add_executable(diag-enc diag-enc.c)
target_link_libraries(diag-enc diagonal)
add_executable(diag-file diag-file.c)
target_link_libraries(diag-file diagonal)
add_executable(diag-fix diag-fix.c)
target_link_libraries(diag-fix diagonal)
add_executable(diag-hash diag-hash.c)
target_link_libraries(diag-hash diagonal)
add_executable(diag-line diag-line.c)
target_link_libraries(diag-line diagonal)
add_executable(diag-mean diag-mean.c)
target_link_libraries(diag-mean diagonal)
add_executable(diag-medi diag-medi.c)
target_link_libraries(diag-medi diagonal)
add_executable(diag-mode diag-mode.c)
target_link_libraries(diag-mode diagonal -lm)
add_executable(diag-pool diag-pool.c)
target_link_libraries(diag-pool diagonal)
add_executable(diag-rep diag-rep.c)
target_link_libraries(diag-rep diagonal)
add_executable(diag-root diag-root.c)
target_link_libraries(diag-root diagonal)
add_executable(diag-times diag-times.c)
target_link_libraries(diag-times diagonal)
add_executable(diag-uniq diag-uniq.c)
target_link_libraries(diag-uniq diagonal)

# set RPATH for each executable, cf. http://www.cmake.org/Wiki/CMake_RPATH_handling
set_target_properties(
  diag
  diag-cycle
  diag-dec
  diag-enc
  diag-file
  diag-fix
  diag-hash
  diag-line
  diag-mean
  diag-medi
  diag-mode
  diag-pool
  diag-rep
  diag-root
  diag-times
  diag-uniq
  PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

install(TARGETS diagonal
                diag
                diag-cycle
                diag-dec
                diag-enc
                diag-file
                diag-fix
                diag-hash
                diag-line
                diag-mean
                diag-medi
                diag-mode
                diag-pool
                diag-rep
                diag-root
                diag-times
                diag-uniq
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
