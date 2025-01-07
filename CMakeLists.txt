cmake_minimum_required(VERSION 3.10)
project(data_feeds LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Boost (requires system, thread, etc.)
find_package(Boost 1.70 REQUIRED COMPONENTS system thread)
if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost not found")
endif()

# Find PostgreSQL
find_package(PostgreSQL REQUIRED)
if(NOT PostgreSQL_FOUND)
    message(FATAL_ERROR "PostgreSQL not found")
endif()

include_directories(
    ${Boost_INCLUDE_DIRS}
    ${PostgreSQL_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Source files
set(SOURCES
    src/main.cpp
    src/dotenv.cpp
    src/config.cpp
    src/server_base.cpp
    src/market_server.cpp
    src/momentum_server.cpp
)

add_executable(data_feeds ${SOURCES})
target_link_libraries(data_feeds
    ${Boost_LIBRARIES}
    ${PostgreSQL_LIBRARIES}
)

# ------------------------------------------------------------------------
# Installation rules:
#   1) Put 'data_feeds' into 'bin/'
#   2) Also put '.env' into 'bin/'
# ------------------------------------------------------------------------
install(TARGETS data_feeds
    RUNTIME DESTINATION bin
)

install(FILES .env
    DESTINATION bin
)

# ------------------------------------------------------------------------
# Optional: CPack to create archives (zip, tar.gz, etc.) containing
#           the bin/ folder with both data_feeds and .env
# ------------------------------------------------------------------------
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_GENERATOR "TGZ;ZIP")  # produce .tar.gz and .zip by default
include(CPack)