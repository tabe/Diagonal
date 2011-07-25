set(title set)
set(tests
    diag_set_insert
    diag_set_erase
    diag_set_contains
)
foreach(test ${tests})
  add_executable(${test} "${test}.c")
  target_link_libraries(${test} diagonal)
  get_target_property(path ${test} LOCATION)
  add_test("${title}/${test}" ${path})
endforeach(test ${tests})
