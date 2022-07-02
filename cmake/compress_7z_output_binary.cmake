function(compress_7z_output_binary TARGET)
  if (${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Linux")
    add_custom_target(${TARGET}_compress_7z_output_binary
      COMMAND 7z a -mx9 ${TARGET}.7z $<TARGET_FILE:${TARGET}>
      )
    add_dependencies(${TARGET}_compress_7z_output_binary ${TARGET})
  endif ()
endfunction()