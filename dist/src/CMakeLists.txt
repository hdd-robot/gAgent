cmake_minimum_required(VERSION 3.15)
project(HelloAgent VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Cherche gAgent dans le répertoire dist/ situé un niveau plus haut.
# Si tu as installé gAgent ailleurs, passe le chemin avec :
#   cmake -DCMAKE_PREFIX_PATH=/chemin/vers/dist ..
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../")

find_package(gAgent REQUIRED)

add_executable(hello main.cpp)
target_link_libraries(hello PRIVATE gAgent::gagent)
