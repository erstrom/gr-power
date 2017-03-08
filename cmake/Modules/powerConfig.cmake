INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_POWER power)

FIND_PATH(
    POWER_INCLUDE_DIRS
    NAMES power/api.h
    HINTS $ENV{POWER_DIR}/include
        ${PC_POWER_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    POWER_LIBRARIES
    NAMES gnuradio-power
    HINTS $ENV{POWER_DIR}/lib
        ${PC_POWER_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(POWER DEFAULT_MSG POWER_LIBRARIES POWER_INCLUDE_DIRS)
MARK_AS_ADVANCED(POWER_LIBRARIES POWER_INCLUDE_DIRS)

