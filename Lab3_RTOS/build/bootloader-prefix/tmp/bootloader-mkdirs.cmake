# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/pablo_kevin/esp/esp-idf/components/bootloader/subproject"
  "/home/pablo_kevin/Sistemas_Embebidos/Lab3_RTOS/build/bootloader"
  "/home/pablo_kevin/Sistemas_Embebidos/Lab3_RTOS/build/bootloader-prefix"
  "/home/pablo_kevin/Sistemas_Embebidos/Lab3_RTOS/build/bootloader-prefix/tmp"
  "/home/pablo_kevin/Sistemas_Embebidos/Lab3_RTOS/build/bootloader-prefix/src/bootloader-stamp"
  "/home/pablo_kevin/Sistemas_Embebidos/Lab3_RTOS/build/bootloader-prefix/src"
  "/home/pablo_kevin/Sistemas_Embebidos/Lab3_RTOS/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/pablo_kevin/Sistemas_Embebidos/Lab3_RTOS/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/pablo_kevin/Sistemas_Embebidos/Lab3_RTOS/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
