set(tests
    diag_hash32_rabin_karp
    diag_hash64_rabin_karp
    diag_rollinghash32_collect
)
foreach(test ${tests})
  add_executable(${test} "${test}.c")
  target_link_libraries(${test} diagonal)
  get_target_property(path ${test} LOCATION)
  add_test("${test}" ${path})
endforeach(test ${tests})
