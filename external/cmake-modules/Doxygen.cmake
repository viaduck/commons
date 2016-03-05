find_package(Doxygen)

# include git revision module
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)

function(setup_doxygen _targetname _doxyfile)
    if(NOT DOXYGEN_FOUND)
        message(FATAL_ERROR "Doxygen not found! Aborting...")
    endif()

    configure_file(${_doxyfile} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(${_targetname}
            ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen" VERBATIM
            )
endfunction()
