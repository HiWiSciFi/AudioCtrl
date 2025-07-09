# CMakeList.txt : CMake project for AudioCtrl, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("AudioCtrl")

# Add WIN32 after target
# Add source to this project's executable.
add_executable (AudioCtrl WIN32)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/dep")

add_dependencies(AudioCtrl fmt::fmt-header-only)

set(AudioCtrl_inc_dir "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(AudioCtrl_src_dir "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_sources(AudioCtrl PRIVATE
	"${AudioCtrl_src_dir}/Audio.cpp"
	"${AudioCtrl_src_dir}/AudioCtrl.cpp"
	"${AudioCtrl_src_dir}/Com.cpp"
	"${AudioCtrl_src_dir}/SystemTray.cpp"
	"${AudioCtrl_src_dir}/WinUtil.cpp"
)

target_include_directories(AudioCtrl PRIVATE
	"${AudioCtrl_inc_dir}"
)

target_link_libraries(AudioCtrl PRIVATE
	fmt::fmt-header-only
)

add_compile_options("/utf-8")

if (WIN32)
	target_link_libraries(AudioCtrl PRIVATE
		"Version.lib"
		"OneCore.lib"
	)
endif (WIN32)

if (UNIX)
	target_link_libraries(AudioCtrl PRIVATE
	)
endif (UNIX)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET AudioCtrl PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
