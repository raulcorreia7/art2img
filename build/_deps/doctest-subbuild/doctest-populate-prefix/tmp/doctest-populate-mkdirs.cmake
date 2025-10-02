# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/mnt/d/projects/art2img/build/_deps/doctest-src"
  "/mnt/d/projects/art2img/build/_deps/doctest-build"
  "/mnt/d/projects/art2img/build/_deps/doctest-subbuild/doctest-populate-prefix"
  "/mnt/d/projects/art2img/build/_deps/doctest-subbuild/doctest-populate-prefix/tmp"
  "/mnt/d/projects/art2img/build/_deps/doctest-subbuild/doctest-populate-prefix/src/doctest-populate-stamp"
  "/mnt/d/projects/art2img/build/_deps/doctest-subbuild/doctest-populate-prefix/src"
  "/mnt/d/projects/art2img/build/_deps/doctest-subbuild/doctest-populate-prefix/src/doctest-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/mnt/d/projects/art2img/build/_deps/doctest-subbuild/doctest-populate-prefix/src/doctest-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/mnt/d/projects/art2img/build/_deps/doctest-subbuild/doctest-populate-prefix/src/doctest-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
