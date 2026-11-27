-- Scroll to Sky: bidirectional camera-to-world-map scrolling for Skyrim SE/AE
-- Targets every flat runtime from 1.5.97 through 1.6.1179 in one DLL via Address Library.

set_config("skyrim_vr", false)

includes("lib/commonlibsse-ng")

set_project("ScrollToSky")
set_version("0.1.3")
set_license("GPL-3.0")
set_languages("c++23")
set_warnings("allextra")

add_rules("mode.debug", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")

target("ScrollToSky")
    add_rules("commonlibsse-ng.plugin", {
        name = "ScrollToSky",
        author = "Thorium AI",
        description = "Scroll the third-person camera out into the world map and back."
    })

    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")
    set_pcxxheader("src/pch.h")
