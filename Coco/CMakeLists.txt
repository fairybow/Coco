# Coco — Common code for Qt projects
# Copyright (C) 2025-2026 fairybow
#
# Consumed as a submodule via add_subdirectory + target_link_libraries(app Coco::Coco).
# Qt Network and Qt Xml are optional: the consumer decides whether to pull them
# in, and the headers self-gate on the COCO_HAS_* macros defined below.

cmake_minimum_required(VERSION 3.21)

project(Coco
    VERSION 0.1.0
    DESCRIPTION "Common code for Qt projects"
    LANGUAGES CXX
)

# Only impose language settings globally when Coco is the top-level build; as a
# subproject we stay out of the parent's way and set these per-target instead.
if(PROJECT_IS_TOP_LEVEL)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

# --- Qt dependencies -------------------------------------------------------
#
# Core / Gui / Widgets are required today: Path, Fx, Concepts, etc. use them
# unconditionally. Network and Xml are optional and gated. The same
# option + PUBLIC-macro pattern extends to Gui/Widgets later, once the matching
# #if guards exist in Path.h / Fx.h / Concepts.h.
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
find_package(Qt6 QUIET OPTIONAL_COMPONENTS Network Xml)

if(Qt6Network_FOUND)
    set(_coco_network_default ON)
else()
    set(_coco_network_default OFF)
endif()
if(Qt6Xml_FOUND)
    set(_coco_xml_default ON)
else()
    set(_coco_xml_default OFF)
endif()

# Consumer-facing switches. Default follows availability; the consumer can force
# either way (set() before add_subdirectory works via CMP0077, or -D on the CLI):
#   -DCOCO_WITH_NETWORK=OFF   never links Network; StartCop compiles to a stub
#   -DCOCO_WITH_XML=ON        requires Xml; hard-errors below if it is missing
option(COCO_WITH_NETWORK
    "Build Coco features needing Qt Network (StartCop)" ${_coco_network_default})
option(COCO_WITH_XML
    "Build Coco features needing Qt Xml (QDomElement in ToQString)" ${_coco_xml_default})

if(COCO_WITH_NETWORK AND NOT Qt6Network_FOUND)
    message(FATAL_ERROR "COCO_WITH_NETWORK is ON but Qt6::Network was not found.")
endif()
if(COCO_WITH_XML AND NOT Qt6Xml_FOUND)
    message(FATAL_ERROR "COCO_WITH_XML is ON but Qt6::Xml was not found.")
endif()

# --- Target ----------------------------------------------------------------
#
# OBJECT (not STATIC) on purpose: Path.cpp's only content is a file-scope static
# initializer that registers Coco::Path with Qt's meta-type system. In a STATIC
# archive the linker discards that TU because nothing references a symbol in it,
# and the registration silently never runs. An OBJECT library links every object
# into the consumer, so the initializer always fires.

if(MSVC)
    add_compile_options(/EHsc)
endif()

add_library(Coco OBJECT
    src/Debug.cpp
    src/Path.cpp

    include/Coco/Bool.h
    include/Coco/Concepts.h
    include/Coco/Debug.h
    include/Coco/Disk.h
    include/Coco/Fmt.h
    include/Coco/Fx.h
    include/Coco/Path.h
    include/Coco/Time.h
    include/Coco/ToQString.h
    include/Coco/Utility.h
)
add_library(Coco::Coco ALIAS Coco)

set_target_properties(Coco PROPERTIES
    AUTOMOC ON
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_include_directories(Coco PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Required modules propagate PUBLIC: anyone including Coco headers needs them.
target_link_libraries(Coco PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets)

# --- Assertions and diagnostics --------------------------------------------
#
# Tri-state rather than option(): option() is a single configure-time boolean,
# but "is this a Debug build" varies per config in multi-config generators
# (Visual Studio). AUTO keeps that config-sensitivity; ON/OFF force it.
#
# Consumers set these before add_subdirectory(), or with -D on the CLI:
#   set(COCO_WITH_ASSERTIONS "OFF")
#   set(COCO_WITH_DIAGNOSTICS "ON")
#
# PUBLIC is required: ASSERT expands in the consumer's own TUs, so a PRIVATE
# definition would leave every consumer assertion silently dead.

# TODO: DIAGNOSTICS maybe can be all qDebug (or DEBUG, after QApp is constructed) output

set(COCO_WITH_ASSERTIONS "AUTO" CACHE STRING
    "Coco ASSERT: ON, OFF, or AUTO (Debug configs only)")
set_property(CACHE COCO_WITH_ASSERTIONS PROPERTY STRINGS ON OFF AUTO)

set(COCO_WITH_DIAGNOSTICS "AUTO" CACHE STRING
    "Coco startup qDebug output: ON, OFF, or AUTO (Debug configs only)")
set_property(CACHE COCO_WITH_DIAGNOSTICS PROPERTY STRINGS ON OFF AUTO)

if(COCO_WITH_ASSERTIONS STREQUAL "ON")
    target_compile_definitions(Coco PUBLIC COCO_HAS_ASSERTIONS)
elseif(COCO_WITH_ASSERTIONS STREQUAL "AUTO")
    target_compile_definitions(Coco PUBLIC $<$<CONFIG:Debug>:COCO_HAS_ASSERTIONS>)
elseif(NOT COCO_WITH_ASSERTIONS STREQUAL "OFF")
    message(FATAL_ERROR "COCO_WITH_ASSERTIONS must be ON, OFF, or AUTO.")
endif()

if(COCO_WITH_DIAGNOSTICS STREQUAL "ON")
    target_compile_definitions(Coco PUBLIC COCO_HAS_DIAGNOSTICS)
elseif(COCO_WITH_DIAGNOSTICS STREQUAL "AUTO")
    target_compile_definitions(Coco PUBLIC $<$<CONFIG:Debug>:COCO_HAS_DIAGNOSTICS>)
elseif(NOT COCO_WITH_DIAGNOSTICS STREQUAL "OFF")
    message(FATAL_ERROR "COCO_WITH_DIAGNOSTICS must be ON, OFF, or AUTO.")
endif()

# --- Optional: Qt Network (StartCop) ---------------------------------------
if(COCO_WITH_NETWORK)
    target_sources(Coco PRIVATE
        src/StartCop.cpp
        include/Coco/StartCop.h)  # only in the build when enabled -> AUTOMOC sees it here
    target_link_libraries(Coco PUBLIC Qt6::Network)
    # PUBLIC: StartCop.h is included by consumer TUs and self-gates on this.
    target_compile_definitions(Coco PUBLIC COCO_HAS_NETWORK)
endif()

# --- Optional: Qt Xml (ToQString's QDomElement support) --------------------
if(COCO_WITH_XML)
    target_link_libraries(Coco PUBLIC Qt6::Xml)
    # PUBLIC: ToQString.h is header-only; the consumer compiles the gated code.
    target_compile_definitions(Coco PUBLIC COCO_HAS_XML)
endif()

# --- Smoke test ------------------------------------------------------------
# Built only when Coco is opened on its own (top-level), so a consumer that
# does add_subdirectory(Coco) never builds it. This is the target you run from
# Visual Studio's green play button.
if(PROJECT_IS_TOP_LEVEL)
    enable_testing()

    add_executable(CocoSmoke src/Smoke.cpp)
    target_link_libraries(CocoSmoke PRIVATE Coco::Coco)

    # Encode what configure asked for, so Smoke.cpp's static_asserts can confirm
    # Coco's PUBLIC COCO_HAS_* macros actually reached this executable.
    target_compile_definitions(CocoSmoke PRIVATE
        COCO_TEST_EXPECT_XML=$<IF:$<BOOL:${COCO_WITH_XML}>,1,0>
        COCO_TEST_EXPECT_NET=$<IF:$<BOOL:${COCO_WITH_NETWORK}>,1,0>
    )

    add_test(NAME CocoSmoke COMMAND CocoSmoke)
endif()