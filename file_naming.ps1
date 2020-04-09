$file_name = $args[0]

(Get-Content $file_name).replace('googletest-break-on-failure-unittest','gtest-break-on-failure-unittest') | Set-Content $file_name

(Get-Content $file_name).replace('googletest-catch-exceptions-test','gtest-catch-exceptions-test') | Set-Content $file_name

(Get-Content $file_name).replace('googletest-color-test','gtest-color-test') | Set-Content $file_name

(Get-Content $file_name).replace('googletest-death-test-test','gtest-death-test-test') | Set-Content $file_name
(Get-Content $file_name).replace('googletest-death-test_ex_test','gtest-death-test-ex-test') | Set-Content $file_name
(Get-Content $file_name).replace('googletest-env-var-test','gtest-env-var-test') | Set-Content $file_name

(Get-Content $file_name).replace('googletest-filepath-test','gtest-filepath-test') | Set-Content $file_name
(Get-Content $file_name).replace('googletest-filter-unittest','gtest-filter-unittest') | Set-Content $file_name

(Get-Content $file_name).replace('googletest-json-outfiles-test','gtest-json-outfiles-test') | Set-Content $file_name
(Get-Content $file_name).replace('googletest-json-output-unittest','gtest-json-output-unittest') | Set-Content $file_name
(Get-Content $file_name).replace('googletest-list-tests-unittest','gtest-list-tests-unittest') | Set-Content $file_name

(Get-Content $file_name).replace('googletest-listener-test','gtest-listener-test') | Set-Content $file_name
(Get-Content $file_name).replace('googletest-message-test','gtest-message-test') | Set-Content $file_name
(Get-Content $file_name).replace('googletest-options-test','gtest-options-test') | Set-Content $file_name
(Get-Content $file_name).replace('googletest-output-test-golden-lin.txt','gtest-output-test-golden-lin.txt') | Set-Content $file_name
(Get-Content $file_name).replace('googletest-output-test','gtest-output-test') | Set-Content $file_name

(Get-Content $file_name).replace('googletest-param-test-invalid-name1-test','gtest-param-test-invalid-name1-test') | Set-Content $file_name

(Get-Content $file_name).replace('googletest-param-test-invalid-name2-test','gtest-param-test-invalid-name2-test') | Set-Content $file_name

(Get-Content $file_name).replace('googletest-param-test-test','gtest-param-test-test') | Set-Content $file_name
(Get-Content $file_name).replace('googletest-param-test2-test','gtest-param-test2-test') | Set-Content $file_name
(Get-Content $file_name).replace('googletest-port-test','gtest-port-test') | Set-Content $file_name
(Get-Content $file_name).replace('googletest-printers-test','gtest-printers-test') | Set-Content $file_name
(Get-Content $file_name).replace('googletest-setuptestsuite-test','gtest-setuptestsuite-test') | Set-Content $file_name

(Get-Content $file_name).replace('googletest-shuffle-test','gtest-shuffle-test') | Set-Content $file_name

(Get-Content $file_name).replace('googletest-test-part-test','gtest-test-part-test') | Set-Content $file_name
(Get-Content $file_name).replace('googletest-throw-on-failure-test','gtest-throw-on-failure-test') | Set-Content $file_name

(Get-Content $file_name).replace('googletest-uninitialized-test','gtest-uninitialized-test') | Set-Content $file_name

(Get-Content $file_name).replace('gtest-typed-test2_test','gtest-typed-test2-test') | Set-Content $file_name

(Get-Content $file_name).replace('gtest-typed-test_test','gtest-typed-test-test') | Set-Content $file_name
(Get-Content $file_name).replace('gtest-unittest-api_test','gtest-unittest-api-test') | Set-Content $file_name
(Get-Content $file_name).replace('gtest_all_test','gtest-all-test') | Set-Content $file_name
(Get-Content $file_name).replace('gtest_assert_by_exception_test','gtest-assert-by-exception-test') | Set-Content $file_name
(Get-Content $file_name).replace('gtest_environment_test','gtest-environment-test') | Set-Content $file_name
(Get-Content $file_name).replace('gtest_help_test','gtest-help-test') | Set-Content $file_name

(Get-Content $file_name).replace('gtest_json_test_utils','gtest-json-test-utils') | Set-Content $file_name
(Get-Content $file_name).replace('gtest_list_output_unittest','gtest-list-output-unittest') | Set-Content $file_name

(Get-Content $file_name).replace('gtest_main_unittest','gtest-main-unittest') | Set-Content $file_name
(Get-Content $file_name).replace('gtest_no_test_unittest','gtest-no-test-unittest') | Set-Content $file_name
(Get-Content $file_name).replace('gtest_pred_impl_unittest','gtest-pred-impl-unittest') | Set-Content $file_name
(Get-Content $file_name).replace('gtest_premature_exit_test','gtest-premature-exit-test') | Set-Content $file_name
(Get-Content $file_name).replace('gtest_prod_test','gtest-prod-test') | Set-Content $file_name
(Get-Content $file_name).replace('gtest_repeat_test','gtest-repeat-test') | Set-Content $file_name
(Get-Content $file_name).replace('gtest_skip_check_output_test','gtest-skip-check-output-test') | Set-Content $file_name
(Get-Content $file_name).replace('gtest_skip_environment_check_output_test','gtest-skip-environment-check-output-test') | Set-Content $file_name
(Get-Content $file_name).replace('gtest_skip_in_environment_setup_test','gtest-skip-in-environment-setup-test') | Set-Content $file_name
(Get-Content $file_name).replace('gtest_skip_test','gtest-skip-test') | Set-Content $file_name
(Get-Content $file_name).replace('gtest_sole_header_test','gtest-sole-header-test') | Set-Content $file_name
(Get-Content $file_name).replace('gtest_stress_test','gtest-stress-test') | Set-Content $file_name
(Get-Content $file_name).replace('gtest_testbridge_test','gtest-testbridge-test') | Set-Content $file_name

(Get-Content $file_name).replace('gtest_test_macro_stack_footprint_test','gtest-test-macro-stack-footprint-test') | Set-Content $file_name
(Get-Content $file_name).replace('gtest_test_utils','gtest-test-utils') | Set-Content $file_name
(Get-Content $file_name).replace('gtest_throw_on_failure_ex_test','gtest-throw-on-failure-ex-test') | Set-Content $file_name
(Get-Content $file_name).replace('gtest_unittest','gtest-unittest') | Set-Content $file_name


(Get-Content $file_name).replace('gtest_xml_outfiles_test','gtest-xml-outfiles-test') | Set-Content $file_name
(Get-Content $file_name).replace('gtest_xml_outfile1_test','gtest-xml-outfile1-test') | Set-Content $file_name
(Get-Content $file_name).replace('gtest_xml_outfile2_test','gtest-xml-outfile2-test') | Set-Content $file_name
(Get-Content $file_name).replace('gtest_xml_output_unittest','gtest-xml-output-unittest') | Set-Content $file_name

(Get-Content $file_name).replace('gtest_xml_test_utils','gtest-xml-test-utils') | Set-Content $file_name
(Get-Content $file_name).replace('production','production') | Set-Content $file_name
(Get-Content $file_name).replace('production','production') | Set-Content $file_name