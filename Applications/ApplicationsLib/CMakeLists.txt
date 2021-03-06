# Source files
get_source_files(SOURCES_APPLICATIONSLIB)
set(LIB_SOURCES ${SOURCES_APPLICATIONSLIB})

# Library
ogs_add_library(ApplicationsLib ${LIB_SOURCES})

target_link_libraries(ApplicationsLib
                      PUBLIC BaseLib GeoLib NumLib Processes
                      PRIVATE MathLib
                              MeshLib
                              MeshGeoToolsLib
                              ParameterLib
                              ProcessLib
                              spdlog::spdlog)

# Set cpp definitions if the cmake option is enabled for the given process.
foreach(process ${ProcessesList})
    if(OGS_BUILD_PROCESS_${process})
        string(TOUPPER "OGS_BUILD_PROCESS_${process}" EnableProcess)
        set_property(TARGET ApplicationsLib
                     APPEND
                     PROPERTY COMPILE_DEFINITIONS ${EnableProcess})
    endif()
endforeach()

if(OGS_USE_PYTHON)
    target_link_libraries(ApplicationsLib PRIVATE pybind11::pybind11)
endif()
