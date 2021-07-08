include_guard()

function(print_cmake_variables)
	get_cmake_property(_VARIABLE_NAMES VARIABLES)
	list(SORT _VARIABLE_NAMES)

	foreach(_VARIABLE_NAME ${_VARIABLE_NAMES})
		if(ARGV0)
			unset(MATCHED)
			string(REGEX MATCH ${ARGV0} MATCHED ${_VARIABLE_NAME})

			if(NOT MATCHED)
				continue()
			endif()
		endif()

		message(STATUS "${_VARIABLE_NAME}=${${_VARIABLE_NAME}}")
	endforeach()
endfunction()
