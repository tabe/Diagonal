set(tests
    diag_bytevector_new_heap
    diag_bytevector_new_path
    diag_bytevector_to_asciz
)
foreach(test ${tests})
  add_executable(${test} "${test}.c")
  target_link_libraries(${test} diagonal)
  get_target_property(path ${test} LOCATION)
  add_test("${test}" ${path})
endforeach(test ${tests})
