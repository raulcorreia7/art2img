# CMake generated Testfile for 
# Source directory: /mnt/d/projects/art2img/tests
# Build directory: /mnt/d/projects/art2img/build/tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(doctest_tests "/mnt/d/projects/art2img/build/bin/art2img_tests")
set_tests_properties(doctest_tests PROPERTIES  WORKING_DIRECTORY "/mnt/d/projects/art2img/build" _BACKTRACE_TRIPLES "/mnt/d/projects/art2img/tests/CMakeLists.txt;45;add_test;/mnt/d/projects/art2img/tests/CMakeLists.txt;0;")
add_test(functionality_tests "/mnt/d/projects/art2img/tests/test_functionality.sh")
set_tests_properties(functionality_tests PROPERTIES  WORKING_DIRECTORY "/mnt/d/projects/art2img/build" _BACKTRACE_TRIPLES "/mnt/d/projects/art2img/tests/CMakeLists.txt;61;add_test;/mnt/d/projects/art2img/tests/CMakeLists.txt;0;")
add_test(palette_comparison_tests "/mnt/d/projects/art2img/tests/test_palette_comparison.sh")
set_tests_properties(palette_comparison_tests PROPERTIES  WORKING_DIRECTORY "/mnt/d/projects/art2img/build" _BACKTRACE_TRIPLES "/mnt/d/projects/art2img/tests/CMakeLists.txt;69;add_test;/mnt/d/projects/art2img/tests/CMakeLists.txt;0;")
add_test(palette_functionality_tests "/mnt/d/projects/art2img/tests/test_palette_functionality.sh")
set_tests_properties(palette_functionality_tests PROPERTIES  WORKING_DIRECTORY "/mnt/d/projects/art2img/build" _BACKTRACE_TRIPLES "/mnt/d/projects/art2img/tests/CMakeLists.txt;77;add_test;/mnt/d/projects/art2img/tests/CMakeLists.txt;0;")
add_test(default_palette_validation_tests "/mnt/d/projects/art2img/tests/test_default_palette_validation.sh")
set_tests_properties(default_palette_validation_tests PROPERTIES  WORKING_DIRECTORY "/mnt/d/projects/art2img/build" _BACKTRACE_TRIPLES "/mnt/d/projects/art2img/tests/CMakeLists.txt;85;add_test;/mnt/d/projects/art2img/tests/CMakeLists.txt;0;")
