set(title rbtree)
set(tests
    sanity02
    sanity10
)
foreach(test ${tests})
  add_executable(${test} "${test}.c")
  target_link_libraries(${test} diagonal)
  get_target_property(path ${test} LOCATION)
  add_test("${title}/${test}" ${path})
endforeach(test ${tests})
