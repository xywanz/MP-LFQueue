cmake_minimum_required(VERSION 2.8)
project(lfqueue)

include_directories(src)

add_executable(cli
    src/cli.c
    src/queue.c
)
target_link_libraries(cli pthread)