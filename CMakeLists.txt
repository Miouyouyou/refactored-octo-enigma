cmake_minimum_required(VERSION 2.8)

# User defined
project(GraphicalAssembler)

include(myy/CMakeCommonOptions.txt)

set (MyyMakeTests_VERSION_MAJOR 2)
set (MyyMakeTests_VERSION_MINOR 0)

set(MyyDumbAssemblerSources
    lib/Assembler/armv7-arm.c
    lib/Assembler/sections/data.c
    lib/Assembler/sections/text.c
    lib/Assembler/helpers/memory.c
    )

set(MyyProjectSources
    myy.c
    src/nodes.c
    src/menus.c
    src/generated/opengl/data_config.c
    ${MyyDumbAssemblerSources}
   )

set(MyyProjectHeaders
    src/nodes.h
    src/menus.h
   )

set(MyyFoldersToCopy
    shaders
    textures
    data)

if (MYY_ANDROID)
	find_library(Freetype REQUIRED)
else (MYY_ANDROID)
	find_package(Freetype REQUIRED)
endif (MYY_ANDROID)
include_directories(${FREETYPE_INCLUDE_DIRS})
set(MyyProjectLibDeps
    ${FREETYPE_LIBRARIES})

set(MyyShaders
    shaders/standard.frag
    shaders/standard.vert
    shaders/node.frag
    shaders/node.vert
    shaders/color_node.frag
    shaders/color_node.vert)
set(MyyData
    data/codepoints.dat
    data/shaders.pack)
set(MyyTextures
    textures/fonts.raw
    textures/cursor.raw
    textures/menus.raw)
set(MyyDocumentation
   )
add_custom_target(Shaders  SOURCES ${MyyShaders})
add_custom_target(Textures SOURCES ${MyyTextures})
add_custom_target(Data     SOURCES ${MyyData})
add_custom_target(Headers  SOURCES ${MyyProjectHeaders})
add_custom_target(Documentation SOURCES ${MyyDocumentation})

include_directories(. myy lib/Assembler)

# Common Section
include(myy/CMakePart.txt)
