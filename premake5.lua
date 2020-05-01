newaction {
	trigger = "generate-buildinfo",
	description = "Sets up build information file like version.h.",
	onWorkspace = function(wks)

		local oldRevNumber = "(none)"
		print ("Reading :: " .. path.getdirectory(wks.location) .. "/src/version.hpp")
		local oldVersionHeader = io.open(path.getdirectory(wks.location) .. "/src/version.hpp", "r")
		if oldVersionHeader ~=nil then
			local oldVersionHeaderContent = assert(oldVersionHeader:read('*a'))
			oldRevNumber = string.match(oldVersionHeaderContent, "#define IW3X_BUILDNUMBER (%d+)")
			if oldRevNumber == nil then
				oldRevNumber = 0
			end
		end

		-- generate version.hpp with a revision number if not equal
		local revNumber = oldRevNumber + 1
			print ("Update " .. oldRevNumber .. " -> " .. revNumber)
			local versionHeader = assert(io.open(path.getdirectory(wks.location) .. "/src/version.hpp", "w"))
			versionHeader:write("/* Automatically generated by premake5. */\n")
			versionHeader:write("\n")
			versionHeader:write("#define IW3X_BUILDNUMBER " .. revNumber .. "\n")
			versionHeader:close()
	end
}

workspace "iw3xo-dev"
	location "./build"
	objdir "%{wks.location}/obj"
	targetdir "%{wks.location}/bin/%{cfg.buildcfg}"
	buildlog "%{wks.location}/obj/%{cfg.architecture}/%{cfg.buildcfg}/%{prj.name}/%{prj.name}.log"
	configurations { "Debug", "Release" }
	architecture "x32"
	platforms "x86"
	systemversion "10.0.17763.0"
	startproject "iw3x"

	disablewarnings {
		"4100",
		"4189",
		"26812",
		"26451",
	}

	buildoptions {
		"/std:c++latest"
	}
	--systemversion "10.0.17134.0"
    systemversion "10.0.17763.0"
	defines { "_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS" }

	configuration "windows"
		defines { "_WINDOWS", "WIN32" }
		staticruntime "On"
		
		if symbols ~= nil then
			symbols "On"
		else
			flags { "Symbols" }
		end

	configuration "Release"
		defines { "NDEBUG" }
		flags { "MultiProcessorCompile", "LinkTimeOptimization", "No64BitChecks" }
		optimize "Full"

	configuration "Debug"
		defines { "DEBUG", "_DEBUG" }
		flags { "MultiProcessorCompile", "No64BitChecks" }
		optimize "Debug"

	project "iw3x"
		kind "SharedLib"
		language "C++"
		files {
			"./src/**.rc",
			"./src/**.hpp",
			"./src/**.cpp",
		}
        links { 
			"glm",
			"imgui"
		}
		includedirs {
			"%{prj.location}/src",
			"./src",
			"./deps/glm/glm", 
			"./deps/imgui",
		}
		resincludedirs {
			"$(ProjectDir)src" -- fix for VS IDE
		}

		-- Pre-compiled header
		pchheader "STDInclude.hpp" -- must be exactly same as used in #include directives
		pchsource "src/STDInclude.cpp" -- real path
		buildoptions { "/Zm100 -Zm100" }

		-- Virtual paths
		if not _OPTIONS["no-new-structure"] then
			vpaths {
				["Headers/*"] = { "./src/**.hpp" },
				["Sources/*"] = { "./src/**.cpp" },
				["Resource/*"] = { "./src/**.rc" },
			}
		end

		vpaths {
			["Docs/*"] = { "**.txt","**.md" },
		}
		
		-- Specific configurations
		flags { "UndefinedIdentifiers" }
		warnings "Extra"

		configuration "Release"
			flags { "FatalCompileWarnings" }
			
			-- Pre-build
			prebuildcommands {
			"cd %{_MAIN_SCRIPT_DIR}",
			"tools\\premake5 generate-buildinfo"
			}
		configuration {}

	group "External dependencies"

        -- glm
		project "glm"
			language "C++"

			files
			{
				"./deps/glm/glm/*.cpp",
				"./deps/glm/glm/*.hpp",
                "./deps/glm/glm/detail/*.cpp",
                "./deps/glm/glm/detail/*.hpp",
                "./deps/glm/glm/ext/*.cpp",
                "./deps/glm/glm/ext/*.hpp",
                "./deps/glm/glm/gtc/*.cpp",
                "./deps/glm/glm/gtc/*.hpp",
                "./deps/glm/glm/gtx/*.cpp",
                "./deps/glm/glm/gtx/*.hpp",
                "./deps/glm/glm/simd/*.cpp",
                "./deps/glm/glm/simd/*.hpp"
			}
            --links { "glm" }
		    includedirs {
			"./deps/glm;" 
		    }

			-- not our code, ignore POSIX usage warnings for now
			warnings "Off"

			-- always build as static lib, as glm doesn't export anything
			kind "StaticLib"

		-- imgui
		project "imgui"
			language "C++"

			files
			{
				"./deps/imgui/*.cpp",
				"./deps/imgui/*.hpp",
                "./deps/imgui/*.h"
			}
            --links { "glm" }
		    includedirs {
			"./deps/imgui;" 
		    }

			-- not our code, ignore POSIX usage warnings for now
			warnings "Off"

			-- always build as static lib, as glm doesn't export anything
			kind "StaticLib"