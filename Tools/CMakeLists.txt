if (EXIB_TEXT)
    add_executable( excc excc.c )
    target_link_libraries(excc PRIVATE EXIB)

    add_executable( exdc exdc.c )
    target_link_libraries(exdc PRIVATE EXIB)
endif ()