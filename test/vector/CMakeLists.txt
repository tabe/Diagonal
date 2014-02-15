set(tests
    diag_vector_length
    diag_vector_copy
    diag_vector_copy_from
    diag_vector_pop_back
    diag_vector_push_back
)
foreach(test ${tests})
  add_executable(${test} "${test}.c")
  target_link_libraries(${test} diagonal)
  get_target_property(path ${test} LOCATION)
  add_test("${test}" ${path})
endforeach(test ${tests})
