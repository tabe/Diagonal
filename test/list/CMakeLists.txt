set(tests
    diag_list_length
    diag_list_reverse
    diag_list_to_vector
)
foreach(test ${tests})
  add_executable(${test} "${test}.c")
  target_link_libraries(${test} diagonal)
  get_target_property(path ${test} LOCATION)
  add_test("${test}" ${path})
endforeach(test ${tests})
