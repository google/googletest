#!/bin/bash

# Executes the samples and tests for the Google Test Framework

# Help the dynamic linker find the path to the framework 
export DYLD_FRAMEWORK_PATH=$BUILT_PRODUCTS_DIR

# Create an array of test executables
test_executables=("$BUILT_PRODUCTS_DIR/sample1_unittest"
                  "$BUILT_PRODUCTS_DIR/sample2_unittest"
                  "$BUILT_PRODUCTS_DIR/sample3_unittest"
                  "$BUILT_PRODUCTS_DIR/sample4_unittest"
                  "$BUILT_PRODUCTS_DIR/sample5_unittest"
                  "$BUILT_PRODUCTS_DIR/sample6_unittest"
                  "$BUILT_PRODUCTS_DIR/sample7_unittest"
                  "$BUILT_PRODUCTS_DIR/sample8_unittest"

                  "$BUILT_PRODUCTS_DIR/gtest-death-test_test"
                  "$BUILT_PRODUCTS_DIR/gtest_environment_test"
                  "$BUILT_PRODUCTS_DIR/gtest-filepath_test"
                  "$BUILT_PRODUCTS_DIR/gtest-linked_ptr_test"
                  "$BUILT_PRODUCTS_DIR/gtest_main_unittest"
                  "$BUILT_PRODUCTS_DIR/gtest-message_test"
                  "$BUILT_PRODUCTS_DIR/gtest_no_test_unittest"
                  "$BUILT_PRODUCTS_DIR/gtest-options_test"
                  "$BUILT_PRODUCTS_DIR/gtest-param-test_test"
                  "$BUILT_PRODUCTS_DIR/gtest-port_test"
                  "$BUILT_PRODUCTS_DIR/gtest_pred_impl_unittest"
                  "$BUILT_PRODUCTS_DIR/gtest_prod_test"
                  "$BUILT_PRODUCTS_DIR/gtest_repeat_test"
                  "$BUILT_PRODUCTS_DIR/gtest_sole_header_test"
                  "$BUILT_PRODUCTS_DIR/gtest_stress_test"
                  "$BUILT_PRODUCTS_DIR/gtest_test_part_test"
                  "$BUILT_PRODUCTS_DIR/gtest-typed-test_test"
                  "$BUILT_PRODUCTS_DIR/gtest_unittest"

                  "$BUILT_PRODUCTS_DIR/gtest_break_on_failure_unittest.py"
                  "$BUILT_PRODUCTS_DIR/gtest_color_test.py"
                  "$BUILT_PRODUCTS_DIR/gtest_env_var_test.py"
                  "$BUILT_PRODUCTS_DIR/gtest_filter_unittest.py"
                  "$BUILT_PRODUCTS_DIR/gtest_list_tests_unittest.py"
                  "$BUILT_PRODUCTS_DIR/gtest_output_test.py"
                  "$BUILT_PRODUCTS_DIR/gtest_xml_outfiles_test.py"
                  "$BUILT_PRODUCTS_DIR/gtest_xml_output_unittest.py"
                  "$BUILT_PRODUCTS_DIR/gtest_uninitialized_test.py"
)

# Now execute each one in turn keeping track of how many succeeded and failed. 
succeeded=0
failed=0
failed_list=()
for test in ${test_executables[*]}; do
  "$test"
  result=$?
  if [ $result -eq 0 ]; then
    succeeded=$(( $succeeded + 1 ))
  else
    failed=$(( failed + 1 ))
    failed_list="$failed_list $test"
  fi
done

# Report the successes and failures to the console
echo "Tests complete with $succeeded successes and $failed failures."
if [ $failed -ne 0 ]; then
  echo "The following tests failed:"
  echo $failed_list
fi
exit $failed
