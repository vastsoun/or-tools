#[=======================================================================[.rst:
FindGLPK
--------

This module determines the GLPK library of the system.

IMPORTED Targets
^^^^^^^^^^^^^^^^

This module defines :prop_tgt:`IMPORTED` target ``GLPK::GLPK``, if
GLPK has been found.

Result Variables
^^^^^^^^^^^^^^^^

This module defines the following variables:

::

GLPK_FOUND          - True if GLPK found.

#]=======================================================================]
include(FindPackageHandleStandardArgs)

# Unset found flag to ensure correct package configuration
unset(GLPK_FOUND)

#==
# Library headers and binaries
#==

find_path(GLPK_INCLUDE_DIRS
  NAMES glpk.h
  HINTS /usr/include
)

find_library(GLPK_LIBRARIES SHARED IMPORTED
  NAMES libglpk.so
  HINTS /usr/local/lib /usr/lib/x86_64-linux-gnu/
)

#==
# Check package contents
#==

find_package_handle_standard_args(GLPK
  REQUIRED_VARS
    GLPK_INCLUDE_DIRS
    GLPK_LIBRARIES
)

if(GLPK_FOUND)
  message(STATUS "GLPK:")
  message(STATUS "  Includes: ${GLPK_INCLUDE_DIRS}")
  message(STATUS "  Libraries: ${GLPK_LIBRARIES}")
else()
  message(FATAL_ERROR "Failed to locate GLPK: Make sure it has been installed via:\nsudo apt install libglpk-dev glpk-utils\n")
endif()

mark_as_advanced(GLPK_FOUND GLPK_INCLUDE_DIRS GLPK_LIBRARIES)

#==
# Package targets
#==

if(GLPK_FOUND AND NOT TARGET GLPK::GLPK)
  add_library(GLPK::GLPK INTERFACE IMPORTED)
  set_target_properties(GLPK::GLPK PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${GLPK_INCLUDE_DIRS}"
    INTERFACE_LINK_LIBRARIES "${GLPK_LIBRARIES}"
  )
endif()

# EOF
