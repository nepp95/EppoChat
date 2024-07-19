project "EppoApp"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "Off"

    targetdir ("%{wks.location}/Bin/" .. OutputDir .. "/%{prj.name}")
    objdir ("%{wks.location}/Bin-Int/" .. OutputDir .. "/%{prj.name}")

    files {
        "Source/**.h",
        "Source/**.cpp"
    }

    includedirs {
        "Source",
        "%{wks.location}/EppoCore/EppoCore/Source",
        "%{wks.location}/EppoCore/EppoCore/Vendor",

        "%{IncludeDir.glm}",
        "%{IncludeDir.imgui}",
        "%{IncludeDir.spdlog}"
    }

    links {
        "EppoCore"
    }

    filter "system:linux"
        links {
            "glfw",
            "glad",
            "imgui",
            "spdlog",
            "GLU",
            "GL",
            "X11",
            "dl",
            "pthread"
        }

    filter "configurations:Debug"
        defines "EPPO_DEBUG"
        runtime "Debug"
        symbols "On"
    
    filter "configurations:Release"
        defines "EPPO_RELEASE"
        runtime "Release"
        optimize "On"
