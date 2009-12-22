set(title vcdiff)
set(tests
    diag_vcdiff_read
    diag_vcdiff_decode
    diag_vcdiff_expand
    diag_vcdiff_contract
)
foreach(test ${tests})
  add_executable(${test} "${test}.c")
  target_link_libraries(${test} diagonal)
  get_target_property(path ${test} LOCATION)
  add_test("${title}/${test}" ${path})
endforeach(test ${tests})
