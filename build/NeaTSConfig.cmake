
####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was NeaTSConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../" ABSOLUTE)

####################################################################################

include(CMakeFindDependencyMacro)
# If you depend on other packages, you can find them here.
# For example, if sux or sdsl were installed separately:
# find_dependency(sux REQUIRED)
# find_dependency(sdsl REQUIRED)

include("${CMAKE_CURRENT_LIST_DIR}/NeaTSTargets.cmake")
