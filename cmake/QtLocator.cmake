# Credits: 
# https://stackoverflow.com/questions/15639781/how-to-find-the-qt5-cmake-module-on-windows
#
# Note:
# The variable QT_INSTALL_BASE must be set before this script gets executed
# The variable points to the base directory where all versions of QT are installed. Default C:\Qt
# If the variable is empty or not defined, the script try's to search for the installation of the QtCreator to find the 
# installation folder.
#

function(get_newest_msvc_compiler_path out rootPath)
    FILE(GLOB CompilerPaths "${rootPath}/msvc*")
    set(NewestYear 0)
    foreach(CompilerPath ${CompilerPaths})
        get_filename_component(CompilerVersion ${CompilerPath} NAME) # Extract the version from the path
    
        # Extract year from the version string
        string(REGEX MATCH "[0-9][0-9][0-9][0-9]" YearMatch ${CompilerVersion})
        if (YearMatch)
            set(Year ${CMAKE_MATCH_0})
            if (Year GREATER NewestYear)
                set(NewestYear ${Year})
                set(newest ${CompilerPath})
            endif()
        endif()
    endforeach()
    set(${out} ${newest} PARENT_SCOPE)
endfunction()

# Function to extract the version number from a path
function(get_version_number out path)
    string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" version ${path})
    set(${out} ${version} PARENT_SCOPE)
endfunction()

set_if_not_defined(QT_MISSING True)

if(NOT QT_VERSION STREQUAL "autoFind" AND DEFINED QT_VERSION)
    if(NOT EXISTS ${QT_INSTALL_BASE}/${QT_VERSION})
        message(FATAL_ERROR "Can't find QT installation. Path: ${QT_INSTALL_BASE}/${QT_VERSION} does not exist")
    endif()

    message("Using predefined Qt Version: ${QT_VERSION}")
    SET(QT_MISSING False)
    # Extract the major version number using a regular expression
    string(REGEX MATCH "([0-9]+)" QT_MAJOR_VERSION ${QT_VERSION})

    # Convert the extracted version number to an integer
    math(EXPR QT_MAJOR_VERSION "${QT_MAJOR_VERSION}")

    if(NOT DEFINED QT_COMPILER OR QT_COMPILER STREQUAL "autoFind")
        get_newest_msvc_compiler_path(QT_PATH ${QT_INSTALL_BASE}/${QT_VERSION})
    else()
        SET(QT_PATH "${QT_INSTALL_BASE}/${QT_VERSION}/${QT_COMPILER}")
    endif()

    
endif()

if(NOT DEFINED QT_MAJOR_VERSION)
    SET(QT_MAJOR_VERSION 5) # Default Qt5 version
endif()

SET(QT_PACKAGE_NAME Qt${QT_MAJOR_VERSION})
SET(QT_WIDGET_PACKAGE_NAME Qt${QT_MAJOR_VERSION}Widgets)





# msvc only; mingw will need different logic
IF(MSVC AND QT_MISSING)
    MESSAGE("Searching for QT installs...")

    if(NOT DEFINED QT_INSTALL_BASE OR "${QT_INSTALL_BASE}" STREQUAL "")
        # look for user-registry pointing to qtcreator
        GET_FILENAME_COMPONENT(QT_BIN [HKEY_CURRENT_USER\\Software\\Classes\\Applications\\QtProject.QtCreator.cpp\\shell\\Open\\Command] PATH)
         # get root path so we can search for 5.3, 5.4, 5.5, etc
        STRING(REPLACE "/Tools" ";" QT_BIN "${QT_BIN}")
        LIST(GET QT_BIN 0 QT_INSTALL_BASE)
    endif()

    # if(NOT DEFINED QT_VERSION OR QT_VERSION STREQUAL "autoFind") 

    # get root path so we can search for 5.3, 5.4, 5.5, etc
    FILE(GLOB QT_VERSIONS "${QT_INSTALL_BASE}/${QT_MAJOR_VERSION}.*")
    
    # Create a list of version numbers
	set(version_numbers )
    foreach(path ${QT_VERSIONS})
        get_version_number(version ${path})
        #message("Extracted version: "${version})
        list(APPEND version_numbers ${version})
    endforeach()

    # Sort the list of paths in descending order based on their version number
    list(LENGTH version_numbers num_versions)

    # Compare versions and find the newest one
    math(EXPR last_index "${num_versions} - 1")
    foreach(i RANGE ${last_index})
        foreach(j RANGE ${last_index})
            list(GET version_numbers ${i} version_i)
            list(GET version_numbers ${j} version_j)

            if(version_i VERSION_GREATER version_j)
                list(GET QT_VERSIONS ${i} path_i)
                list(GET QT_VERSIONS ${j} path_j)
                list(REMOVE_AT QT_VERSIONS ${i})
                list(INSERT QT_VERSIONS ${i} ${path_j})
                list(REMOVE_AT QT_VERSIONS ${j})
                list(INSERT QT_VERSIONS ${j} ${path_i})
                list(REMOVE_AT version_numbers ${i})
                list(INSERT version_numbers ${i} ${version_j})
                list(REMOVE_AT version_numbers ${j})
                list(INSERT version_numbers ${j} ${version_i})
            endif()
        endforeach()
    endforeach()
    list(GET QT_VERSIONS 0 newestQtVersionPath)

    # fix any double slashes which seem to be common
    STRING(REPLACE "//" "/"  newestQtVersionPath "${newestQtVersionPath}")



    # Initialize variables to store the newest compiler version and path
    
    set(NewestCompilerPath "${newestQtVersionPath}/${QT_COMPILER}")

    if(NOT DEFINED QT_COMPILER OR NOT EXISTS ${NewestCompilerPath} OR QT_COMPILER STREQUAL "autoFind")
        get_newest_msvc_compiler_path(NewestCompilerPath ${newestQtVersionPath})
    endif()

    

    if (EXISTS ${NewestCompilerPath})
        set(QT_PATH ${NewestCompilerPath})
        SET(QT_MISSING False)
    endif()
ENDIF()

# use Qt_DIR approach so you can find Qt after cmake has been invoked
IF(NOT QT_MISSING)
    if (EXISTS ${QT_PATH})
        message("Using compiler: ${QT_PATH}")
	    
        
        SET(Qt5_DIR "${QT_PATH}/lib/cmake/Qt${QT_MAJOR_VERSION}")
        SET(Qt${QT_MAJOR_VERSION}Widgets_DIR  "${QT_PATH}/lib/cmake/Qt${QT_MAJOR_VERSION}Widgets")
        SET(Qt${QT_MAJOR_VERSION}Test_DIR "${QT_PATH}/lib/cmake/Qt${QT_MAJOR_VERSION}Test")
        SET(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${QT_PATH}/lib/cmake")
        # SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${QT_PATH}/lib/cmake/Qt${QT_MAJOR_VERSION}Config.cmake")
        #SET(CMAKE_PREFIX_PATH "${QT_PATH}")
        
        MESSAGE("Qt${QT_MAJOR_VERSION}Config.cmake path:  ${Qt${QT_MAJOR_VERSION}_DIR}")
    else()
        message(FATAL_ERROR "No QT${QT_MAJOR_VERSION} installation found. \n"
                            "Searching for compiler: ${QT_PATH}")
    endif()
ENDIF()


function(qt_wrap_internal_cpp outFiles inFiles)
    # Get the length of the list
    list(LENGTH inFiles listLength)
    if(listLength GREATER  0)
        # Call qt_wrap_cpp with the parsed arguments
        if(${QT_MAJOR_VERSION} EQUAL 5)
            qt5_wrap_cpp(${outFiles} ${inFiles})
        elseif(${QT_MAJOR_VERSION} EQUAL 6)
            qt6_wrap_cpp(${outFiles} ${inFiles})
        endif()

        # Export the output files variable for parent scope
        set(${outFiles} ${${outFiles}} PARENT_SCOPE)
    endif()
endfunction()


function(qt_wrap_internal_ui outFiles inFiles)
    # Get the length of the list
    list(LENGTH inFiles listLength)
    if(listLength GREATER  0)
        # Call qt_wrap_cpp with the parsed arguments
        if(${QT_MAJOR_VERSION} EQUAL 5)
            qt5_wrap_ui(${outFiles} ${inFiles})
        elseif(${QT_MAJOR_VERSION} EQUAL 6)
            qt6_wrap_ui(${outFiles} ${inFiles})
        endif()

        # Export the output files variable for parent scope
        set(${outFiles} ${${outFiles}} PARENT_SCOPE)
    endif()
endfunction()

function(qt_add_internal_resources outFiles inFiles)
    # Get the length of the list
    list(LENGTH inFiles listLength)
    if(listLength GREATER  0)
        # Call qt_wrap_cpp with the parsed arguments
        if(${QT_MAJOR_VERSION} EQUAL 5)
            qt5_add_resources(${outFiles} ${inFiles})
        elseif(${QT_MAJOR_VERSION} EQUAL 6)
            qt6_add_resources(${outFiles} ${inFiles})
        endif()

        # Export the output files variable for parent scope
        set(${outFiles} ${${outFiles}} PARENT_SCOPE)
    endif()
endfunction()
