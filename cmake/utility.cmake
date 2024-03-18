# Macro to search for files with given file ending.
# call:
#   FILE_DIRECTORIES(H_FILES *.h)
#
# all *.h files will be saved in the H_FILES variable
MACRO(FILE_DIRECTORIES return_list ending)
    FILE(GLOB_RECURSE new_list ${ending})
    SET(dir_list "")
    FOREACH(file_path ${new_list})
        GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)
        SET(dir_list ${dir_list} ${file_path})
    ENDFOREACH()
    LIST(REMOVE_DUPLICATES dir_list)
    SET(${return_list} ${dir_list})
ENDMACRO()


function(get_filename_from_path FILE_PATH FILE_NAME_VAR)
    get_filename_component(${FILE_NAME_VAR} ${FILE_PATH} NAME)
endfunction()


# Function name: DEPLOY_QT
# Params: QT_PATH           Path to the QT installation. C:\Qt\5.15.2\msvc2015_64
#         targetExePath     Path to the exe file: "$<TARGET_FILE_DIR:profiler_gui>/$<TARGET_FILE_NAME:profiler_gui>"
#     
 function(DEPLOY_QT targetName outputPath)
    
 # check if QT_PATH is empty
    if (NOT QT_PATH)
		message("QT_PATH is not set. include QtLocator.cmake first, to find a qt installation or assign a 
                 QT pat to it. example: set(QT_PATH \"C:/Qt/5.14.2\")")
        return()
    endif()

    set(DEPLOY_COMMAND  "${QT_PATH}/bin/windeployqt.exe"
                --no-compiler-runtime
                --no-translations
                --no-system-d3d-compiler
                --no-opengl-sw
                --no-angle
                --no-webkit2
                --pdb)


   set(targetExePath "$<TARGET_FILE_DIR:${targetName}>/$<TARGET_FILE_NAME:${targetName}>")

   string(MD5 TARGETPATH_HASH ${outputPath})
   set(UNIQUE_TARGET_NAME "deploy_${TARGETPATH_HASH}_${targetName}")
   message("DeploymentTargetName: ${UNIQUE_TARGET_NAME}")

   if(TARGET ${UNIQUE_TARGET_NAME})
	   message("Target ${UNIQUE_TARGET_NAME} already exists")
	   return()
   endif()
   add_custom_target(${UNIQUE_TARGET_NAME} ALL
       DEPENDS "${targetExePath}"
   )
    # Deploy easy_profiler gui to bin folder
     add_custom_command(TARGET ${UNIQUE_TARGET_NAME}
         COMMAND ${DEPLOY_COMMAND} 
             --dir "${outputPath}"
            "${targetExePath}"
 	    COMMENT "Running windeployqt..." "${QT_PATH}/bin/windeployqt.exe" "${outputPath}"
        DEPENDS UNIQUE_TARGET_NAME
     )
 endfunction()


function(copyLibraryHeaders headerRootFolder destinationPath destinationFolderName)
     # Copy the folder
    #message("COPY ${headerRootFolder} DESTINATION ${CMAKE_BINARY_DIR}")
    file(COPY ${headerRootFolder}
         DESTINATION ${CMAKE_BINARY_DIR})

    
    get_filename_component(FOLDER_NAME ${headerRootFolder} NAME)
    #message("FOLDER_NAME ${FOLDER_NAME}")


    #message("REMOVE_RECURSE ${CMAKE_BINARY_DIR}/${destinationFolderName}")
    file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/${destinationFolderName}")



    #message("RENAME ${CMAKE_BINARY_DIR}/${FOLDER_NAME}
    #            ${CMAKE_BINARY_DIR}/${destinationFolderName}")

    # Rename the copied folder
    file(RENAME ${CMAKE_BINARY_DIR}/${FOLDER_NAME}
                ${CMAKE_BINARY_DIR}/${destinationFolderName})

    #message("DIRECTORY ${CMAKE_BINARY_DIR}/${destinationFolderName}
    #        DESTINATION ${destinationPath}")
    # Install the modified folder
    install(DIRECTORY ${CMAKE_BINARY_DIR}/${destinationFolderName}
            DESTINATION ${destinationPath})

    message("Installing headers from: ${headerRootFolder} to ${destinationPath}/${destinationFolderName}")

endfunction()
