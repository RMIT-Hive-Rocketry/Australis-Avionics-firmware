# Shamelessly taken from this webpage:
# https://dev.to/khozaei/automating-semver-with-git-and-cmake-2hji

function(get_version_from_git)
  find_package(Git QUIET)
  if(NOT Git_FOUND)
    message(WARNING "Git not found")
    return()
  endif()

  execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --always
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_TAG
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE GIT_RESULT
    )

  if(NOT GIT_RESULT EQUAL 0)
    message(WARNING "Failed to get git tag")
    return()
  endif()

  string(REGEX REPLACE "^v" "" CLEAN_TAG "${GIT_TAG}")
  if(CLEAN_TAG MATCHES "^([0-9]+)\\.([0-9]+)\\.([0-9]+)(-.*)?$")

    set(PROJECT_VERSION_MAJOR ${CMAKE_MATCH_1})
    set(PROJECT_VERSION_MAJOR ${CMAKE_MATCH_1} PARENT_SCOPE)
    set(PROJECT_VERSION_MINOR ${CMAKE_MATCH_2})
    set(PROJECT_VERSION_MINOR ${CMAKE_MATCH_2} PARENT_SCOPE)
    set(PROJECT_VERSION_PATCH ${CMAKE_MATCH_3})
    set(PROJECT_VERSION_PATCH ${CMAKE_MATCH_3} PARENT_SCOPE)

    set(PROJECT_VERSION_PRERELEASE ${PRERELEASE_VER})
    set(PROJECT_VERSION_PRERELEASE ${PRERELEASE_VER} PARENT_SCOPE)

    set(PROJECT_VERSION "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${CMAKE_MATCH_3}")
    set(PROJECT_VERSION "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${CMAKE_MATCH_3}" PARENT_SCOPE)

    string(REGEX REPLACE "^-" "" PRERELEASE_VER "${CMAKE_MATCH_4}")
    set(FULL_VERSION "${PROJECT_VERSION}-${PRERELEASE_VER}")
    set(FULL_VERSION "${PROJECT_VERSION}-${PRERELEASE_VER}" PARENT_SCOPE)

  else()
    message(WARNING "Tag '${CLEAN_TAG}' does not match semver format")
  endif()
endfunction()
