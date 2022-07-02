function(strip_output_binary TARGET)
  if (${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Linux")
    add_custom_command(
      TARGET ${TARGET}
      POST_BUILD
      COMMAND strip -v -S --strip-unneeded --remove-section=.note.gnu.gold-version --remove-section=.comment --remove-section=.note --remove-section=.note.gnu. --remove-section=.note.ABI-tag --remove-section=.eh_frame --remove-section=.eh_frame_ptr $<TARGET_FILE:${TARGET}>
      VERBATIM
    )
  endif ()
endfunction()