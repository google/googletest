Invoke-Item (start powershell ((Split-Path $MyInvocation.InvocationName) + "\file_naming.ps1 googletest/test/BUILD.bazel"))


Invoke-Item (start powershell ((Split-Path $MyInvocation.InvocationName) + "\file_naming.ps1 googletest/CMakeLists.txt"))

Push-Location -Path googletest/test/

(Get-Content gtest-typed-test-test.cc).replace('gtest-typed-test_test.h','gtest-typed-test-test.h') | Set-Content gtest-typed-test-test.cc
(Get-Content gtest-typed-test2-test.cc).replace('gtest-typed-test_test.h','gtest-typed-test-test.h') | Set-Content gtest-typed-test2-test.cc
(Get-Content gtest-param-test-test.cc).replace('googletest-param-test-test.h','gtest-param-test-test.h') | Set-Content gtest-param-test-test.cc
(Get-Content gtest-param-test2-test.cc).replace('googletest-param-test-test.h','gtest-param-test-test.h') | Set-Content gtest-param-test2-test.cc
(Get-Content gtest-all-test.cc).replace('googletest-','gtest-') | Set-Content gtest-all-test.cc
(Get-Content gtest-all-test.cc).replace('_','-') | Set-Content gtest-all-test.cc

Get-ChildItem 'googletest*' | Rename-Item -NewName { $_.Name -Replace 'googletest','gtest' }
Get-ChildItem 'gtest*' | Rename-Item -NewName { $_.Name -Replace '_','-' }
Get-ChildItem 'gtest*' | Rename-Item -NewName { $_.Name -Replace '-.cc','_.cc' }

Pop-Location
