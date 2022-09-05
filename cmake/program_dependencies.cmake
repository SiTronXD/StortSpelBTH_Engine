
function(find_program_dependencies)
    set(program_list "")
    find_program(python_path NAMES python python.exe )
    if(NOT ${python_path} MATCHES "python_path-NOTFOUND")
        message("python was found!")
    else()
        message("Could not find pyhon, install python!")
        if(${WIN32})
            message("Make sure no previous version of python is installed (like the one from microsoft store)")
            message("1. Download python: https://www.python.org/downloads/")
            message("2. During installation, make sure to Add Python to PATH!")
            message("3. At the end of the installation, select 'Disable Path Length Limit' if that option is available!")                
            message("4. Make sure to update PIP before installing anything. It's done through this command (using PowerShell):")  
            message("\tpython -m pip install --upgrade pip")  
        endif()
    endif()
    LIST(APPEND program_list ${python_path})

    find_program(conan_path NAMES conan conan.exe )

    if(NOT ${conan_path} MATCHES "conan_path-NOTFOUND")
        message("conan was found!")
    else()
        message("Could not find conan, install conan through PIP!")
        if(${WIN32})
            message("Make sure to update PIP before installing anything. It's done through this command (using PowerShell):")  
        else()
            message("Make sure to update PIP before installing anything. It's done through this command:")  
        endif()
        message("\tpython -m pip install --upgrade pip")  
        message("Install conan from a terminal with this command:")
        message("\tpip install conan")

    endif()
    LIST(APPEND program_list ${conan_path})


    LIST(FILTER program_list INCLUDE REGEX "[^-]-NOTFOUND$")
    LIST(LENGTH program_list program_list_LENGTH)

    if(NOT (${program_list_LENGTH} MATCHES 0))
    message(FATAL_ERROR "Please intall all required dependencies in order to build!")
    endif()

    # Download the conan.cmake wrapper for cmake!
    if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/conan.cmake")
        message("Missing conan.cmake file, fetching it from github.com/conan-io/cmake-conan!")
        file(
            DOWNLOAD 
            "https://raw.githubusercontent.com/conan-io/cmake-conan/0.18.1/conan.cmake" # Input url to file
            "${CMAKE_CURRENT_BINARY_DIR}/conan.cmake"                                   # Output file
            EXPECTED_HASH SHA256=5cdb3042632da3efff558924eecefd580a0e786863a857ca097c3d1d43df5dcd
            STATUS download_status
        )
        if(NOT download_status MATCHES "^0;" ) #if download_status begins with '0;' then no problems...
            message(FATAL_ERROR "Failed to cownload conan.cmake, recieved following error: ${download_status}")
        else()
            message("Fetched conan.cmake correctly! download status: ${download_status}")
        endif()
    endif()

endfunction(find_program_dependencies)
