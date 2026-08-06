# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************/

function(embed_webui TARGET)
    # Defaults
    set(WEBTOOLKIT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/WebToolkit)

    # Optional overlay from client
    if(DEFINED CLIENT_OVERLAY_DIR AND IS_DIRECTORY "${CLIENT_OVERLAY_DIR}")
        set(WEBTOOLKIT_OVERLAY_DIR ${CLIENT_OVERLAY_DIR})
    endif()

    set(WEBTOOLKIT_OUTPUT_DIR ${CMAKE_BINARY_DIR}/WebServerHome)

    # Create output directory
    add_custom_command(
        OUTPUT ${WEBTOOLKIT_OUTPUT_DIR}/.stamp_created
        COMMAND ${CMAKE_COMMAND} -E rm -rf ${WEBTOOLKIT_OUTPUT_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${WEBTOOLKIT_OUTPUT_DIR}
        COMMAND ${CMAKE_COMMAND} -E touch ${WEBTOOLKIT_OUTPUT_DIR}/.stamp_created
    )

    # Default Webtoolkit files
    set(webtoolkit_files ${webtoolkit_files} "${WEBTOOLKIT_DIR}/index.html")
    set(webtoolkit_files ${webtoolkit_files} "${WEBTOOLKIT_DIR}/favicon.ico")
    FILE(GLOB ojl_js_files "${WEBTOOLKIT_DIR}/OJL/JavaScript/*.js")
    set(webtoolkit_files ${webtoolkit_files} ${ojl_js_files})
    FILE(GLOB ojl_css_files "${WEBTOOLKIT_DIR}/OJL/StyleSheets/*.css")
    set(webtoolkit_files ${webtoolkit_files} ${ojl_css_files})
    FILE(GLOB ojl_ttf_files "${WEBTOOLKIT_DIR}/OJL/StyleSheets/*.ttf")
    set(webtoolkit_files ${webtoolkit_files} ${ojl_ttf_files})    
    FILE(GLOB ojl_image_files "${WEBTOOLKIT_DIR}/OJL/Images/*.png")
    set(webtoolkit_files ${webtoolkit_files} ${ojl_image_files})

    set(webtoolkit_dependencies)
    foreach(web_file IN LISTS webtoolkit_files)
        string(REPLACE "${WEBTOOLKIT_DIR}" "${WEBTOOLKIT_OUTPUT_DIR}" web_file_bin ${web_file})
        list(APPEND webtoolkit_dependencies ${web_file_bin})
        get_filename_component(web_file_bin_dir ${web_file_bin} DIRECTORY)
        add_custom_command(OUTPUT ${web_file_bin}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${web_file_bin_dir}
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${web_file} ${web_file_bin}
            DEPENDS ${web_file} ${WEBTOOLKIT_OUTPUT_DIR}/.stamp_created
            VERBATIM)
    endforeach()

    if(WEBTOOLKIT_OVERLAY_DIR)
        FILE(GLOB_RECURSE overlay_dependencies "${WEBTOOLKIT_OVERLAY_DIR}/*")
        add_custom_command(
            OUTPUT ${WEBTOOLKIT_OUTPUT_DIR}/.stamp_updated
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${WEBTOOLKIT_OVERLAY_DIR}
                ${WEBTOOLKIT_OUTPUT_DIR}            
            COMMAND ${CMAKE_COMMAND} -E touch ${WEBTOOLKIT_OUTPUT_DIR}/.stamp_updated
            DEPENDS ${webtoolkit_dependencies} ${overlay_dependencies} ${WEBTOOLKIT_OUTPUT_DIR}/.stamp_created
        )
    else()
        add_custom_command(
            OUTPUT ${WEBTOOLKIT_OUTPUT_DIR}/.stamp_updated
            COMMAND ${CMAKE_COMMAND} -E touch ${WEBTOOLKIT_OUTPUT_DIR}/.stamp_updated
            DEPENDS ${webtoolkit_dependencies} ${WEBTOOLKIT_OUTPUT_DIR}/.stamp_created
        )
    endif()

    if(WEBTOOLKIT_MINIFY_ASSETS)
        include(cmake/minify_webui.cmake)

        add_custom_command(
            OUTPUT ${WEBTOOLKIT_OUTPUT_DIR}/.stamp_minified
            COMMAND ${CMAKE_COMMAND} -DWEBTOOLKIT_OUTPUT_DIR=${WEBTOOLKIT_OUTPUT_DIR} -DESBUILD_BIN=${ESBUILD_BIN} -P ${MINIFY_SCRIPT}
            COMMAND ${CMAKE_COMMAND} -E touch ${WEBTOOLKIT_OUTPUT_DIR}/.stamp_minified
            DEPENDS ${WEBTOOLKIT_OUTPUT_DIR}/.stamp_updated
            VERBATIM)
    else()
        add_custom_command(
            OUTPUT ${WEBTOOLKIT_OUTPUT_DIR}/.stamp_minified
            COMMAND ${CMAKE_COMMAND} -E touch ${WEBTOOLKIT_OUTPUT_DIR}/.stamp_minified
            DEPENDS ${WEBTOOLKIT_OUTPUT_DIR}/.stamp_updated
            VERBATIM)
    endif()

    if (WEBTOOLKIT_PYTHON_PACKER)
        set(file_packer_exe ${CMAKE_CURRENT_SOURCE_DIR}/file-packer/FilePacker.py)
    
        # Set FilePacker as executable
        add_custom_command(OUTPUT .stamp_filepacker
                        COMMAND chmod +x ${file_packer_exe}
                        COMMAND touch .stamp_filepacker
                        VERBATIM)
    else()
        if (UNIX)
            set(file_packer_exe ${CMAKE_CURRENT_SOURCE_DIR}/file-packer/linux/FilePacker)
        
            # Set FilePacker as executable
            add_custom_command(OUTPUT .stamp_filepacker
                            COMMAND chmod +x ${file_packer_exe}
                            COMMAND touch .stamp_filepacker
                            VERBATIM)
                            
        elseif (WIN32)
            set(file_packer_exe ${CMAKE_CURRENT_SOURCE_DIR}/file-packer/windows/FilePacker.exe)
        endif()
    endif()    

        # Add a custom command to pack all the files in WebToolkit into a single binary file
    add_custom_command(OUTPUT WebServerHome.bin
                       COMMAND ${file_packer_exe} "${WEBTOOLKIT_OUTPUT_DIR}" WebServerHome.bin
                       DEPENDS ${WEBTOOLKIT_OUTPUT_DIR}/.stamp_minified .stamp_filepacker
                       VERBATIM)

    if (UNIX)
        # Add a custom command to convert the binary file into an object file for linking
        add_custom_command(OUTPUT WebServerHome.o
                           COMMAND ${CMAKE_LINKER} -z noexecstack -r -b binary -o WebServerHome.o WebServerHome.bin
                           DEPENDS WebServerHome.bin
                           VERBATIM)


        # Add the object file as a source item so it is linked into the library
        target_sources(${TARGET} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/WebServerHome.o")

    elseif (WIN32)
        # Add a custom command to convert the binary file into an object file for linking
        set(echo_line_1 \#define ID_WSB 100)
        set(echo_line_2 "ID_WSB RCDATA \"WebServerHome.bin\"")
        add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/WebToolkit.rc
                           COMMAND (echo ${echo_line_1} & echo ${echo_line_2}) > ${CMAKE_CURRENT_SOURCE_DIR}/WebToolkit.rc
                           DEPENDS WebServerHome.bin)

        if (WIN32)
            target_sources(${TARGET} PRIVATE WebToolkit.rc)
            target_compile_definitions(${TARGET} PRIVATE _WINSOCK_DEPRECATED_NO_WARNINGS)
            target_compile_definitions(${TARGET} PRIVATE _CRT_SECURE_NO_WARNINGS)
        endif()
    endif()
    
    # Export webtoolkit_files to parent scope so it can be used by cpack.cmake
    set(webtoolkit_files ${webtoolkit_files} PARENT_SCOPE)
    
endfunction()
