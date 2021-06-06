WinHttpClient = {
	source = path.join(dependencies.basePath, "WinHttpClient"),
}

function WinHttpClient.import()
	links { "WinHttpClient" }
	WinHttpClient.includes()
end

function WinHttpClient.includes()
	includedirs {
		path.join(WinHttpClient.source, "WinHttpClient"),
	}
end

function WinHttpClient.project()
	project "WinHttpClient"
		language "C++"

		WinHttpClient.includes()

		files {
            path.join(WinHttpClient.source, "WinHttpClient/*.cpp"),
			path.join(WinHttpClient.source, "WinHttpClient/*.h"),
		}

		warnings "Off"
		kind "StaticLib"

        -- build with 2014
        removebuildoptions "/std:c++latest"
end

table.insert(dependencies, WinHttpClient)
