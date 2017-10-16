add_executable (test_deployment test_deployment.cpp)
add_executable (test_large_scale test_large_scale.cpp)

target_link_libraries (test_deployment LINK_PUBLIC Src)
target_link_libraries (test_large_scale LINK_PUBLIC Src)
