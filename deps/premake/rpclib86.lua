rpclib86 = {
	source = path.join(dependencies.basePath, "rpclib"),
}

function rpclib86.import()
	links { "rpclib86" }
	filter { "configurations:Debug" }
		libdirs { path.join(rpclib86.source, "lib/debug") }

	filter { "configurations:Release" }
		libdirs { path.join(rpclib86.source, "lib/release") }

	filter {}

	rpclib86.includes()
end

function rpclib86.includes()
	includedirs {
		rpclib86.source,
		path.join(rpclib86.source, "include")
	}
end

function rpclib86.project()
	project "rpclib86"
		language "C++"

		rpclib86.includes()

		files {
			path.join(rpclib86.source, "include/**"),
			path.join(rpclib86.source, "src/**")
		}

		warnings "Off"
		kind "None"
end

table.insert(dependencies, rpclib86)
