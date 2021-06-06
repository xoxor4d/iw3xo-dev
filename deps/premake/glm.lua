glm = {
	source = path.join(dependencies.basePath, "glm"),
}

function glm.import()
	links { "glm" }
	glm.includes()
end

function glm.includes()
	includedirs {
        glm.source,
		path.join(glm.source, "glm"),
	}
end

function glm.project()
	project "glm"
		language "C++"

		glm.includes()

		files {
            path.join(glm.source, "glm/*.cpp"),
			path.join(glm.source, "glm/*.hpp"),
            path.join(glm.source, "glm/detail/*.cpp"),
            path.join(glm.source, "glm/detail/*.hpp"),
            path.join(glm.source, "glm/ext/*.cpp"),
            path.join(glm.source, "glm/ext/*.hpp"),
            path.join(glm.source, "glm/gtc/*.cpp"),
            path.join(glm.source, "glm/gtc/*.hpp"),
            path.join(glm.source, "glm/gtx/*.cpp"),
            path.join(glm.source, "glm/gtx/*.hpp"),
            path.join(glm.source, "glm/simd/*.cpp"),
            path.join(glm.source, "glm/simd/*.h"),
		}

		warnings "Off"
		kind "StaticLib"
end

table.insert(dependencies, glm)
