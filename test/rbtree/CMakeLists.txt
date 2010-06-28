set(title rbtree)
set(tests
    diag_rbtree_for_each_attr
    sanity02
    sanity10
    random
)
foreach(test ${tests})
  add_executable(${test} "${test}.c")
  target_link_libraries(${test} diagonal)
  get_target_property(path ${test} LOCATION)
  add_test("${title}/${test}" ${path})
endforeach(test ${tests})
