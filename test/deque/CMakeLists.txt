set(tests
    diag_deque_push_and_pop
    diag_deque_to_array
)
foreach(test ${tests})
  add_executable(${test} "${test}.c")
  target_link_libraries(${test} diagonal)
  get_target_property(path ${test} LOCATION)
  add_test("${test}" ${path})
endforeach(test ${tests})
