set(tests
    diag_port_new_fd
    diag_port_new_fp
    diag_port_new_bm
    diag_port_new_path
)
foreach(test ${tests})
  add_executable(${test} "${test}.c")
  target_link_libraries(${test} diagonal)
  get_target_property(path ${test} LOCATION)
  add_test("${test}" ${path})
endforeach(test ${tests})
