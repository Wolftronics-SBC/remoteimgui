cmake_minimum_required(VERSION 3.5)

file(GLOB_RECURSE lz4 lz4/*.c lz4/*.h)
file(GLOB_RECURSE webby webby/*.c webby/*.h)
set(imgui_header_files imgui/imgui.h imgui/imgui_internal.h imgui/stb_rect_pack.h imgui/stb_textedit.h imgui/stb_truetype.h)
set(imgui_src_files imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_demo.cpp)
file(GLOB_RECURSE imgui_example_files imgui/examples/*.cpp imgui/examples/*.h)
file(GLOB_RECURSE example_files example/*.cpp example/*.h)
set(header_files imgui_remote.h imgui_remote_webby.h)
set(src_files imgui_remote.cpp imgui_remote_webby.cpp)

add_library(remoteimgui ${src_files} ${header_files} ${lz4} ${webby} ${imgui_src_files} ${imgui_header_files} ${imgui_example_files} ${example_files})

set_source_files_properties(${imgui_example_files} PROPERTIES HEADER_FILE_ONLY true)
set_source_files_properties(${example_files} PROPERTIES HEADER_FILE_ONLY true)

source_group(lz4 FILES ${lz4})
source_group(webby FILES ${webby})
source_group(src FILES ${imgui_src_files})
source_group(include FILES ${header_files} ${imgui_header_files})

foreach(file IN LISTS imgui_example_files)
    get_filename_component(file_dir "${file}" DIRECTORY)
	file(RELATIVE_PATH relative_path "${CMAKE_CURRENT_SOURCE_DIR}" "${file_dir}")
    string(REPLACE "/" "\\" path_msvc "${relative_path}")
    source_group("${path_msvc}" FILES ${file})
endforeach()

foreach(file IN LISTS example_files)
    get_filename_component(file_dir "${file}" DIRECTORY)
	file(RELATIVE_PATH relative_path "${CMAKE_CURRENT_SOURCE_DIR}" "${file_dir}")
    string(REPLACE "/" "\\" path_msvc "${relative_path}")
    source_group("${path_msvc}" FILES ${file})
endforeach()