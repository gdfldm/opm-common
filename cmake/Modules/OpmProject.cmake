# - Helper routines for opm-core like projects

function (configure_cmake_file name variant version)
  # declarative list of the variable names that are used in the template
  # and that must be defined in the project to be exported
  set (variable_suffices
	DESCRIPTION
	VERSION
	DEFINITIONS
	INCLUDE_DIRS
	LIBRARY_DIRS
	LINKER_FLAGS
	CONFIG_VARS
	LIBRARY
	LIBRARIES
	TARGET
	)

  # set these variables temporarily (this is in a function scope) so
  # they are available to the template (only)
  foreach (suffix IN LISTS variable_suffices)
	set (opm-project_${suffix} "${${name}_${suffix}}")
  endforeach (suffix)

  if (BUILD_SHARED_LIBS)
    # No need to list shared libraries as the linker information is alread
    # in the shared lib
    string(REGEX REPLACE ";?[^;]*.so" "" opm-project_LIBRARIES "${opm-project_LIBRARIES}")
  endif()
  set (opm-project_NAME "${${name}_NAME}")
  set (opm-project_NAME_UC "${${name}_NAME}")
  string(TOUPPER "${opm-project_NAME}" opm-project_NAME_UC)
  string(REPLACE "-" "_" opm-project_NAME_UC "${opm-project_NAME_UC}")

  # make the file substitutions
  configure_file (
	${template_dir}/opm-project-config${version}.cmake.in
	${PROJECT_BINARY_DIR}/${${name}_NAME}-${variant}${version}.cmake
	@ONLY
	)
endfunction (configure_cmake_file name)

# installation of CMake modules to help user programs locate the library
function (opm_cmake_config name)
  # assume that the template is located in cmake/Templates (cannot use
  # the current directory since this is in a function and the directory
  # at runtime not at definition will be used
  set (template_dir "${OPM_MACROS_ROOT}/cmake/Templates")

  # write configuration file to locate library
  set(DUNE_PREFIX ${PROJECT_SOURCE_DIR})
  set(OPM_PROJECT_EXTRA_CODE ${OPM_PROJECT_EXTRA_CODE_INTREE})
  set(PREREQ_LOCATION "${PROJECT_SOURCE_DIR}")
  configure_cmake_file (${name} "config" "")
  configure_cmake_file (${name} "config" "-version")
  configure_vars (
	FILE CMAKE "${PROJECT_BINARY_DIR}/${${name}_NAME}-config.cmake"
	APPEND "${${name}_CONFIG_VARS}"
	)

  # The next replace will result in bogus entries if install directory is
  # a subdirectory of source tree,
  # and we have existing entries pointing to that install directory.
  # Since they will yield a duplicate in next replace anyways, we filter them out first
  # to get avoid those the bogus entries.
  # First with trailing / to change /usr/include/package to package and not /package
  # Fixes broken pkg-config files for Debian/Ubuntu packaging.
  string(FIND "${${name}_INCLUDE_DIRS}" "${PROJECT_SOURCE_DIR}" _source_in_include)

  if(_source_in_include GREATER "-1")
    string(REGEX REPLACE "${CMAKE_INSTALL_PREFIX}/include${${name}_VER_DIR}[;$]" "" ${name}_INCLUDE_DIRS "${${name}_INCLUDE_DIRS}")
    # Get rid of empty entries
    string(REPLACE ";;" ";" ${name}_INCLUDE_DIRS "${${name}_INCLUDE_DIRS}")

    # replace the build directory with the target directory in the
    # variables that contains build paths
    string (REPLACE
      "${PROJECT_SOURCE_DIR}"
      "${CMAKE_INSTALL_PREFIX}/include${${name}_VER_DIR}"
      ${name}_INCLUDE_DIRS
      "${${name}_INCLUDE_DIRS}"
      )
  endif()
  string (REPLACE
	"${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
	"${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}${${name}_VER_DIR}"
	${name}_LIBRARY
	"${${name}_LIBRARY}"
	)
  set (CMAKE_LIBRARY_OUTPUT_DIRECTORY
	"${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}${${name}_VER_DIR}"
	)
  # create a config mode file which targets the install directory instead
  # of the build directory (using the same input template)
  set(OPM_PROJECT_EXTRA_CODE ${OPM_PROJECT_EXTRA_CODE_INSTALLED})
  set(PREREQ_LOCATION "${CMAKE_INSTALL_PREFIX}/share/opm/cmake/Modules")
  set(DUNE_PREFIX ${CMAKE_INSTALL_PREFIX})
  configure_cmake_file (${name} "install" "")
  configure_vars (
	FILE CMAKE "${PROJECT_BINARY_DIR}/${${name}_NAME}-install.cmake"
	APPEND "${${name}_CONFIG_VARS}"
	)
  # this file gets copied to the final installation directory
  install (
	FILES ${PROJECT_BINARY_DIR}/${${name}_NAME}-install.cmake
	DESTINATION share/cmake${${name}_VER_DIR}/${${name}_NAME}
	RENAME ${${name}_NAME}-config.cmake
	)
  # assume that there exists a version file already
  install (
	FILES ${PROJECT_BINARY_DIR}/${${name}_NAME}-config-version.cmake
	DESTINATION share/cmake${${name}_VER_DIR}/${${name}_NAME}
	)
endfunction (opm_cmake_config name)
