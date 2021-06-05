imgui = {
	source = path.join(dependencies.basePath, "imgui"),
}

function imgui.import()
	links { "imgui" }
	imgui.includes()
end

function imgui.includes()
	includedirs {
		imgui.source
	}
end

function imgui.project()
	project "imgui"
		language "C++"

		imgui.includes()

		files {
            path.join(imgui.source, "*.cpp"),
			path.join(imgui.source, "*.hpp"),
            path.join(imgui.source, "*.h"),
		}

		warnings "Off"
		kind "StaticLib"
end

table.insert(dependencies, imgui)
