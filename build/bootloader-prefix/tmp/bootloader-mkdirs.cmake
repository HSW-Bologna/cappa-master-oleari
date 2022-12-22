# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/xgampx/Desktop/UNI/TIROCINIO/esp-idf/components/bootloader/subproject"
  "/home/xgampx/Desktop/UNI/TIROCINIO/kprogetto/kmaster/build/bootloader"
  "/home/xgampx/Desktop/UNI/TIROCINIO/kprogetto/kmaster/build/bootloader-prefix"
  "/home/xgampx/Desktop/UNI/TIROCINIO/kprogetto/kmaster/build/bootloader-prefix/tmp"
  "/home/xgampx/Desktop/UNI/TIROCINIO/kprogetto/kmaster/build/bootloader-prefix/src/bootloader-stamp"
  "/home/xgampx/Desktop/UNI/TIROCINIO/kprogetto/kmaster/build/bootloader-prefix/src"
  "/home/xgampx/Desktop/UNI/TIROCINIO/kprogetto/kmaster/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/xgampx/Desktop/UNI/TIROCINIO/kprogetto/kmaster/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/xgampx/Desktop/UNI/TIROCINIO/kprogetto/kmaster/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
