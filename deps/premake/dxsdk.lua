dxsdk = {
	source = path.join(dependencies.basePath, "dxsdk"),
}

function dxsdk.import()
	filter "platforms:*32"
		libdirs { path.join(dxsdk.source, "Lib/x86") }
		
	filter "platforms:*64"
		libdirs { path.join(dxsdk.source, "Lib/x64") }
		
	filter {}
	
	dxsdk.includes()
end

function dxsdk.includes()
	includedirs {
		path.join(dxsdk.source, "Include")
	}
end

function dxsdk.project()

end

table.insert(dependencies, dxsdk)
