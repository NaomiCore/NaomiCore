# This file is part of the NaomiCore Project. Powered by GetRight 2026
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

########### authserver ###############

CollectSourceFiles(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE_SOURCES
  # Exclude
  ${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeaders)

if(WIN32)
  if(MSVC)
    list(APPEND PRIVATE_SOURCES authserver.rc)
  endif()
endif()

if(USE_COREPCH)
  set(PRIVATE_PCH_HEADER PrecompiledHeaders/authPCH.h)
endif()

GroupSources(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(authserver
  ${PRIVATE_SOURCES}
)

if(NOT WIN32)
  target_compile_definitions(authserver PRIVATE
    _TRINITY_REALM_CONFIG="${CONF_DIR}/AuthServer.conf"
    _TRINITY_REALM_CONFIG_DIR="${CONF_DIR}/AuthServer.conf.d"
  )
endif()

target_link_libraries(authserver
  PRIVATE
    trinity-core-interface
  PUBLIC
    shared)

CollectIncludeDirectories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC_INCLUDES
  # Exclude
  ${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeaders)

target_include_directories(authserver
  PUBLIC
    ${PUBLIC_INCLUDES}
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR})

set_target_properties(authserver
  PROPERTIES
    COMPILE_WARNING_AS_ERROR ${WITH_WARNINGS_AS_ERRORS}
    FOLDER "server")

if(UNIX)
  install(TARGETS authserver DESTINATION bin)
  if(COPY_CONF)
    add_custom_command(TARGET authserver
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/AuthServer.conf.dist ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../etc/AuthServer.conf.dist
    )
    install(FILES  AuthServer.conf.dist DESTINATION ${CONF_DIR})
  endif()
elseif(WIN32)
  install(TARGETS authserver DESTINATION "${CMAKE_INSTALL_PREFIX}")
  if(COPY_CONF)
    add_custom_command(TARGET authserver
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/AuthServer.conf.dist ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/AuthServer.conf.dist
    )
    install(FILES AuthServer.conf.dist DESTINATION "${CMAKE_INSTALL_PREFIX}")
  endif()
endif()

# Generate precompiled header
if(USE_COREPCH)
  add_cxx_pch(authserver ${PRIVATE_PCH_HEADER})
endif()
