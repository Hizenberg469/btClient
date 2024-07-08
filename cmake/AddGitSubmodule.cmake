function(add_git_submodule relative_dir lib link)
    find_package(Git REQUIRED)

    set(FULL_DIR ${CMAKE_SOURCE_DIR}/${relative_dir})

    #For intializing git if not already initialized.
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/.git")
        # If .git directory does not exist, execute git init
        execute_process(
            COMMAND git init
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            RESULT_VARIABLE result
        )

        if(result)
            message(FATAL_ERROR "Failed to initialize git repository.")
        endif()
    endif()

    #For adding the external deps if not added yet
    if (NOT EXISTS ${FULL_DIR}/${lib})
        execute_process(
            COMMAND ${GIT_EXECUTABLE}
            submodule add -- ${link}
            WORKING_DIRECTORY ${FULL_DIR}
        )
    elseif (NOT EXISTS ${FULL_DIR}/CMakeLists.txt)
        execute_process(COMMAND ${GIT_EXECUTABLE}
            submodule update --init  -- ${relative_dir}
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
    endif()

    if (EXISTS ${FULL_DIR}/${lib}/CMakeLists.txt)
        message("Submodule is CMake Project: ${FULL_DIR}/${lib}/CMakeLists.txt")
        add_subdirectory(${FULL_DIR}/${lib})
    else()
        message("Submodule is NO CMake Project: ${FULL_DIR}")
    endif()
endfunction(add_git_submodule)