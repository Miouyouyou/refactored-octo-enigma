cmake_minimum_required(VERSION 2.8)

# User defined
project(GraphicalAssembler)

include(myy/CMakeCommonOptions.txt)

set (MyyMakeTests_VERSION_MAJOR 2)
set (MyyMakeTests_VERSION_MINOR 0)

set(MyyDumbAssemblerSources
    lib/Assembler/dumbelflib.c
    lib/Assembler/armv7-arm.c
    lib/Assembler/sections/data.c
    lib/Assembler/sections/text.c
    )

set(MyyMainHooks
    myy.c
    )

set(MyyTestsMainHooks
    tests/myy.c
   )

set(MyyTestsSources
    tests/buffers.c
    tests/comarrays.c
    tests/binary_generator_menu.c
    tests/tests_defs.h
    tests/common.h
    tests/hitaction.c
   )

set(MyyProjectSources
    src/nodes.c
    src/nodes_asm.c
    src/nodes_mem.c
    src/menus.c
    src/text.c
    src/generated/opengl/data_config.c
    src/program_builder_menu.c
    ${MyyDumbAssemblerSources}
   )

set(MyyProjectHeaders
    src/nodes.h
    src/nodes_asm.h
    src/nodes_mem.h
    src/menus.h
    src/text.h
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
    shaders/lines.frag
    shaders/lines.vert
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
add_custom_target(Shaders       SOURCES ${MyyShaders})
add_custom_target(Textures      SOURCES ${MyyTextures})
add_custom_target(Data          SOURCES ${MyyData})
add_custom_target(Headers       SOURCES ${MyyProjectHeaders})
add_custom_target(Documentation SOURCES ${MyyDocumentation})

include_directories(. myy lib/Assembler)

# Common Section
include(myy/CMakePart.txt)
