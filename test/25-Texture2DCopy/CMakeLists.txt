project( "Test-${FOLDER_NAME}" )

set( ${PROJECT_NAME}_VERSION_MAJOR 0 )
set( ${PROJECT_NAME}_VERSION_MINOR 1 )
set( ${PROJECT_NAME}_VERSION_BUILD 0 )

file( GLOB SOURCE_FILES
	Src/*.cpp
)
set( SOURCE_FILES
	${SOURCE_FILES}
	${Ashes_SOURCE_DIR}/external/imgui/imgui.cpp
	${Ashes_SOURCE_DIR}/external/imgui/imgui_draw.cpp
	${Ashes_SOURCE_DIR}/external/imgui/imgui_tables.cpp
	${Ashes_SOURCE_DIR}/external/imgui/imgui_widgets.cpp
)

file( GLOB HEADER_FILES
	Src/*.hpp
	Src/*.inl
)

file( GLOB SHADER_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*.*
)

source_group( "Shader Files" FILES ${SHADER_FILES} )

add_executable( ${PROJECT_NAME} WIN32
	${SOURCE_FILES}
	${HEADER_FILES}
	${SHADER_FILES}
)
target_include_directories( ${PROJECT_NAME}
	PUBLIC
		${Ashes_SOURCE_DIR}/external/imgui
)
target_link_libraries( ${TARGET_NAME} PRIVATE
	ashes::test::Common
)

foreach( SHADER ${SHADER_FILES} )
	add_custom_command(
		TARGET ${PROJECT_NAME}
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E make_directory
			$<$<CONFIG:Debug>:${PROJECTS_BINARIES_OUTPUT_DIR_DEBUG}/share/${FOLDER_NAME}/Shaders>
			$<$<CONFIG:Release>:${PROJECTS_BINARIES_OUTPUT_DIR_RELEASE}/share/${FOLDER_NAME}/Shaders>
			$<$<CONFIG:RelWithDebInfo>:${PROJECTS_BINARIES_OUTPUT_DIR_RELWITHDEBINFO}/share/${FOLDER_NAME}/Shaders>
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SHADER}
			$<$<CONFIG:Debug>:${PROJECTS_BINARIES_OUTPUT_DIR_DEBUG}/share/${FOLDER_NAME}/Shaders>
			$<$<CONFIG:Release>:${PROJECTS_BINARIES_OUTPUT_DIR_RELEASE}/share/${FOLDER_NAME}/Shaders>
			$<$<CONFIG:RelWithDebInfo>:${PROJECTS_BINARIES_OUTPUT_DIR_RELWITHDEBINFO}/share/${FOLDER_NAME}/Shaders>
	)
endforeach()
