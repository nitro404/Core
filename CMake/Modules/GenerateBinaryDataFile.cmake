include_guard()

function(generate_binary_data_file)
	set(OPTIONS)

	set(SINGLE_VALUE_ARGUMENTS
		INPUT_FILE
		TEMPLATE_FILE
		OUTPUT_FILE
		BYTES_PER_LINE
		LINE_INDENTATION
	)

	set(MULTI_VALUE_ARGUMENTS)

	cmake_parse_arguments(
		GENERATE_BINARY_FILE
		"${OPTIONS}"
		"${SINGLE_VALUE_ARGUMENTS}"
		"${MULTI_VALUE_ARGUMENTS}"
		${ARGN}
	)

	if(NOT DEFINED GENERATE_BINARY_FILE_INPUT_FILE)
		message(FATAL_ERROR "generate_binary_file function 'INPUT_FILE' argument is required.")
	endif()

	if(NOT DEFINED GENERATE_BINARY_FILE_TEMPLATE_FILE)
		message(FATAL_ERROR "generate_binary_file function 'TEMPLATE_FILE' argument is required.")
	endif()

	if(NOT DEFINED GENERATE_BINARY_FILE_OUTPUT_FILE)
		message(FATAL_ERROR "generate_binary_file function 'OUTPUT_FILE' argument is required.")
	endif()

	if(NOT IS_ABSOLUTE "${GENERATE_BINARY_FILE_INPUT_FILE}")
		message(FATAL_ERROR "generate_binary_file function INPUT_FILE argument must be an absolute file path: ${GENERATE_BINARY_FILE_INPUT_FILE}")
	endif()

	if(NOT IS_ABSOLUTE "${GENERATE_BINARY_FILE_TEMPLATE_FILE}")
		message(FATAL_ERROR "generate_binary_file function TEMPLATE_FILE argument must be an absolute file path: ${GENERATE_BINARY_FILE_TEMPLATE_FILE}")
	endif()

	if(NOT IS_ABSOLUTE "${GENERATE_BINARY_FILE_OUTPUT_FILE}")
		message(FATAL_ERROR "generate_binary_file function OUTPUT_FILE argument must be an absolute file path: ${GENERATE_BINARY_FILE_OUTPUT_FILE}")
	endif()

	if(NOT EXISTS "${GENERATE_BINARY_FILE_INPUT_FILE}")
		message(FATAL_ERROR "generate_binary_file function failed due to missing input file: ${GENERATE_BINARY_FILE_INPUT_FILE}")
	endif()

	if(NOT EXISTS "${GENERATE_BINARY_FILE_TEMPLATE_FILE}")
		message(FATAL_ERROR "generate_binary_file function failed due to missing template file: ${GENERATE_BINARY_FILE_TEMPLATE_FILE}")
	endif()

	if(NOT DEFINED GENERATE_BINARY_FILE_BYTES_PER_LINE)
		set(GENERATE_BINARY_FILE_BYTES_PER_LINE 0)
	elseif(NOT GENERATE_BINARY_FILE_BYTES_PER_LINE MATCHES "^(0|[1-9][0-9]*)$")
		message(FATAL_ERROR "generate_binary_file function failed due to invalid BYTES_PER_LINE argument, expected a non-negative integer: ${GENERATE_BINARY_FILE_BYTES_PER_LINE}")
	endif()

	if(NOT DEFINED GENERATE_BINARY_FILE_LINE_INDENTATION)
		set(GENERATE_BINARY_FILE_LINE_INDENTATION "")
	endif()

	file(READ "${GENERATE_BINARY_FILE_INPUT_FILE}" INPUT_FILE_DATA HEX)
	string(LENGTH "${INPUT_FILE_DATA}" INPUT_FILE_DATA_LENGTH)
	set(DATA "")
	set(DATA_LENGTH 0)

	if(INPUT_FILE_DATA_LENGTH GREATER 0)
		math(EXPR DATA_LENGTH "${INPUT_FILE_DATA_LENGTH} / 2")

		if(GENERATE_BINARY_FILE_BYTES_PER_LINE EQUAL 0)
			set(BYTES_PER_LINE "${DATA_LENGTH}")
		else()
			set(BYTES_PER_LINE "${GENERATE_BINARY_FILE_BYTES_PER_LINE}")
		endif()

		string(REGEX REPLACE "([0-9a-fA-F][0-9a-fA-F])" "0x\\1, " DATA "${INPUT_FILE_DATA}")
		set(BYTE_PATTERN "0x[0-9a-fA-F][0-9a-fA-F], ")
		string(REPEAT "${BYTE_PATTERN}" "${BYTES_PER_LINE}" LINE_PATTERN)
		string(REGEX REPLACE "(${LINE_PATTERN})" "\\1\n" DATA "${DATA}")
		string(REGEX REPLACE " \n" "\n" DATA "${DATA}")
		string(REGEX REPLACE ", \n$" "" DATA "${DATA}")
		string(REGEX REPLACE ", $" "" DATA "${DATA}")
		string(REPLACE "\n" "\n${GENERATE_BINARY_FILE_LINE_INDENTATION}" DATA "${DATA}")
	endif()

	configure_file("${GENERATE_BINARY_FILE_TEMPLATE_FILE}" "${GENERATE_BINARY_FILE_OUTPUT_FILE}" @ONLY)
endfunction()
