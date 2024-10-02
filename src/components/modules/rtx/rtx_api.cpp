#include "std_include.hpp"
#include "rtx_api.hpp"

#define CHECK_INIT(ret) if (!is_initialized()) { game::Com_PrintMessage(0, "BridgeApi not initialized!", 0); return ret; }
#define CHECK_INIT_NO_RET() if (!is_initialized()) { game::Com_PrintMessage(0, "BridgeApi not initialized!", 0); return; }

namespace components
{
	remixapi_ErrorCode rtx_api::init()
	{
		const auto status = remixapi::bridge_initRemixApi(&bridge);
		if (status == REMIXAPI_ERROR_CODE_SUCCESS)
		{
			m_initialized = true;
		}

		game::Com_PrintMessage(0, utils::va("[BridgeApi] bridgeapi_initialize() : %s ", !status ? "success" : utils::va("error : %d", status)), 0);

		/*if (bridge)
		{
			bridge.RegisterDevice();
		}*/

		return status;
		//return REMIXAPI_ERROR_CODE_GENERAL_FAILURE;
	}

	bool rtx_api::destroy_mesh(uint64_t* in_out_handle)
	{
		CHECK_INIT(false);
		if (!in_out_handle || !*in_out_handle)
		{
			return false;
		}

		bridge.DestroyMesh((remixapi_MeshHandle)*in_out_handle);
		*in_out_handle = 0;
		return true;
	}

	bool rtx_api::create_cod4_mesh(remixapi_MeshHandle* in_out_handle, const char* model_name, uint64_t* material)
	{
		CHECK_INIT(false);
		if (!in_out_handle || !model_name)
		{
			return false;
		}

		if (const auto mdl = game::DB_FindXAssetHeader(game::XAssetType::ASSET_TYPE_XMODEL, model_name).model; mdl)
		{
			std::vector<remixapi_MeshInfoSurfaceTriangles> surfs;
			std::vector<std::vector<remixapi_HardcodedVertex>> verts;
			std::vector<std::vector<uint32_t>> indices;

			const uint32_t surf_count = (uint32_t) mdl->numsurfs;
			for (uint32_t s = 0; s < surf_count; s++)
			{
				const auto& surf = mdl->surfs[s];
				verts.emplace_back(); // add new vector entry for current surface

				const uint32_t vert_count = (uint32_t) surf.vertCount;
				for (uint32_t v = 0; v < vert_count; v++)
				{
					const auto& vert = surf.verts0[v];

					game::vec3_t unpacked_normal;
					rtx_fixed_function::unpack_normal(&vert.normal, unpacked_normal);

					game::vec2_t unpacked_texcoord;
					game::Vec2UnpackTexCoords(vert.texCoord.packed, unpacked_texcoord);

					verts.back().emplace_back(remixapi_HardcodedVertex
					{
						{ vert.xyz[0], vert.xyz[1], vert.xyz[2] },
						{ unpacked_normal[0], unpacked_normal[1], unpacked_normal[2] },
						{ unpacked_texcoord[0], unpacked_texcoord[1] },
						(uint32_t)vert.color.packed,
						0u, 0u, 0u, 0u, 0u, 0u, 0u
					});
				}

				indices.emplace_back(); // add new vector entry for current surface
				const uint32_t index_count = (uint32_t)surf.triCount * 3;
				for (uint32_t i = 0; i < index_count; i++)
				{
					indices.back().emplace_back((uint32_t)surf.triIndices[i]);
				}

				surfs.emplace_back(remixapi_MeshInfoSurfaceTriangles
				{
					verts[s].data(),
					vert_count,
					indices[s].data(),
					index_count,
					FALSE,
					{},
					material ? (remixapi_MaterialHandle)*material : nullptr,
				});
			}

			remixapi_MeshInfo i =
			{
				.sType = REMIXAPI_STRUCT_TYPE_MESH_INFO,
				.hash = 0xDEAD,
				.surfaces_values = surfs.data(),
				.surfaces_count = surf_count,
			};

			if (*in_out_handle)
			{
				rtx_api::bridge.DestroyMesh(*in_out_handle);
				*in_out_handle = nullptr;
			}

			rtx_api::bridge.CreateMesh(&i, in_out_handle);
		}

		return true;
	}

	bool rtx_api::create_sphere_light(uint64_t* in_out_handle, remixapi_LightInfo* l, const remixapi_LightInfoSphereEXT* s)
	{
		CHECK_INIT(false);
		if (!in_out_handle)
		{
			return false;
		}

		if (*in_out_handle)
		{
			bridge.DestroyLight((remixapi_LightHandle)*in_out_handle);
			*in_out_handle = 0u;
		}

		l->pNext = (void*)s;

		//remixapi_LightHandle handle = nullptr;
		bridge.CreateLight(l, (remixapi_LightHandle*)in_out_handle);
		//*in_out_handle = (uint64_t)handle;

		//game::Com_PrintMessage(0, utils::va("bridge.CreateLight handle = %d \n", *in_out_handle), 0);

		return true;
	}

	bool rtx_api::create_rect_light(uint64_t* in_out_handle, remixapi_LightInfo* l, const remixapi_LightInfoRectEXT* r)
	{
		CHECK_INIT(false);
		if (!in_out_handle)
		{
			return false;
		}

		if (*in_out_handle)
		{
			bridge.DestroyLight((remixapi_LightHandle)*in_out_handle);
		}

		l->pNext = (void*)r;
		bridge.CreateLight(l, (remixapi_LightHandle*)in_out_handle);
		//game::Com_PrintMessage(0, utils::va("bridge.CreateLight handle = %d \n", *in_out_handle), 0);

		return true;
	}

	bool rtx_api::create_disk_light(uint64_t* in_out_handle, remixapi_LightInfo* l, const remixapi_LightInfoDiskEXT* d)
	{
		CHECK_INIT(false);
		if (!in_out_handle)
		{
			return false;
		}

		if (*in_out_handle)
		{
			bridge.DestroyLight((remixapi_LightHandle)*in_out_handle);
		}

		l->pNext = (void*)d;
		bridge.CreateLight(l, (remixapi_LightHandle*)in_out_handle);
		//game::Com_PrintMessage(0, utils::va("bridge.CreateLight handle = %d \n", *in_out_handle), 0);

		return true;
	}

	bool rtx_api::create_cylinder_light(uint64_t* in_out_handle, remixapi_LightInfo* l, const remixapi_LightInfoCylinderEXT* cy)
	{
		CHECK_INIT(false);
		if (!in_out_handle)
		{
			return false;
		}

		if (*in_out_handle)
		{
			bridge.DestroyLight((remixapi_LightHandle)*in_out_handle);
		}

		l->pNext = (void*)cy;
		bridge.CreateLight(l, (remixapi_LightHandle*)in_out_handle);
		//game::Com_PrintMessage(0, utils::va("bridge.CreateLight handle = %d \n", *in_out_handle), 0);

		return true;
	}

	bool rtx_api::create_distant_light(uint64_t* in_out_handle, remixapi_LightInfo* l, const remixapi_LightInfoDistantEXT* d)
	{
		CHECK_INIT(false);
		if (!in_out_handle)
		{
			return false;
		}

		if (*in_out_handle)
		{
			bridge.DestroyLight((remixapi_LightHandle)*in_out_handle);
		}

		l->pNext = (void*)d;
		bridge.CreateLight(l, (remixapi_LightHandle*)in_out_handle);
		//game::Com_PrintMessage(0, utils::va("bridge.CreateLight handle = %d \n", *in_out_handle), 0);

		return true;
	}

	bool rtx_api::destroy_light(uint64_t* in_out_handle)
	{
		CHECK_INIT(false);
		if (!in_out_handle || !*in_out_handle)
		{
			return false;
		}

		bridge.DestroyLight((remixapi_LightHandle)*in_out_handle);
		*in_out_handle = 0;
		return true;
	}

	void rtx_api::create_quad(remixapi_HardcodedVertex* v_out, uint32_t* i_out, const float scale)
	{
		if (!v_out || !i_out)
		{
			return;
		}

		auto makeVertex = [&](float x, float y, float z, float u, float v) {
			const remixapi_HardcodedVertex vert =
			{
			  .position = {x,y,z},
			  .normal = {0,0,-1},
			  .texcoord = { u, v },
			  .color = 0xFFFFFFFF,
			};
			return vert;
		};

		v_out[0] = makeVertex(-1.0f * scale, 1, -1.0f * scale, 0.0f, 0.0f); // b l
		v_out[1] = makeVertex(-1.0f * scale, 1, 1.0f * scale, 0.0f, 1.0f); // t l
		v_out[2] = makeVertex(1.0f * scale, 1, -1.0f * scale, 1.0f, 0.0f); // b r
		v_out[3] = makeVertex(1.0f * scale, 1, 1.0f * scale, 1.0f, 1.0f); // t r

		i_out[0] = 0;
		i_out[1] = 1;
		i_out[2] = 2;
		i_out[3] = 3;
		i_out[4] = 2;
		i_out[5] = 1;
	}

	void rtx_api::to_remix_transform(remixapi_Transform* transform, game::vec3_t position, game::vec3_t rotation, game::vec3_t scale)
	{
		if (!transform || !position || !rotation || !scale)
		{
			return;
		}

		const glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), glm::to_vec3(position));

		glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(rotation[2]), glm::vec3(0, 0, 1));
		rotationMatrix = glm::rotate(rotationMatrix, glm::radians(rotation[1]), glm::vec3(0, 1, 0));
		rotationMatrix = glm::rotate(rotationMatrix, glm::radians(rotation[0]), glm::vec3(1, 0, 0));

		const glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::to_vec3(scale));
		const glm::mat4 transformMatrix = glm::transpose(translationMatrix * rotationMatrix * scaleMatrix); // column to row-major

		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				transform->matrix[i][j] = glm::value_ptr(transformMatrix)[i * 4 + j];
			}
		}
	}

	rtx_api::rtx_api()
	{
		p_this = this;

		scheduler::once([]
		{
			rtx_api::init();
		}, scheduler::main);

#if DEBUG
		command::add("api_init", "", "calls the x64 process func foo", [this]([[maybe_unused]] command::params parms)
		{
			init();
		});
#endif

		command::add("api_set_config_var", "<var> <value>", "RemixApi: sets config variable 'var' to 'value'", [this]([[maybe_unused]] command::params parms)
		{
			CHECK_INIT_NO_RET();
			if (parms.length() <= 2)
			{
				game::Com_PrintMessage(0, "Usage: api_set_config_var <var> <value> :: eg: /api_set_config_var rtx.enableAlphaBlend 0", 0);
				return;
			}

			bridge.SetConfigVariable(parms[1], parms[2]);
			//game::Com_PrintMessage(0, utils::va("Command was: %s %s", parms[1], parms[2]), 0);
		});

		command::add("api_sky_brightness", "value", "RemixApi: sets config variable 'rtx.skyBrightness'", [this]([[maybe_unused]] command::params parms) 
		{
			CHECK_INIT_NO_RET();
			if (parms.length() == 1)
			{
				game::Com_PrintMessage(0, "Usage: api_sky_brightness <brightness value>", 0);
				return;
			}
				
			bridge.SetConfigVariable("rtx.skyBrightness", parms[1]);
		});

		command::add("api_create_light", "[opt:position] <x> <y> <z>   [opt:radius] <radius>   [optional:radiance] <r> <g> <b>", "RemixApi: Create a light with the 'CreateLight' func\neg: api_create_light 0 0 100 500 250 300", [this]([[maybe_unused]] command::params parms)
		{
			remixapi_LightInfo l = {};
			{
				l.sType = REMIXAPI_STRUCT_TYPE_LIGHT_INFO;
				l.hash = 0x1337;
				l.radiance =
				{
					parms.length() >= 5 ? utils::try_stof(parms[5]) : 1000,  // rad r
					parms.length() >= 6 ? utils::try_stof(parms[6]) : 1200,  // rad g
					parms.length() >= 7 ? utils::try_stof(parms[7]) : 1500   // rad b
				};
			}

			remixapi_LightInfoSphereEXT s = {};
			{
				s.sType = REMIXAPI_STRUCT_TYPE_LIGHT_INFO_SPHERE_EXT;
				s.position = 
				{
					parms.length() >= 1 ? utils::try_stof(parms[1]) : 0,	 // x
					parms.length() >= 2 ? utils::try_stof(parms[2]) : 200,	 // y
					parms.length() >= 3 ? utils::try_stof(parms[3]) : -15,	 // z
				};

				s.radius = parms.length() >= 4 ? utils::try_stof(parms[4]) : 1;
				s.shaping_hasvalue = false;
			}

			rtx_api::create_sphere_light(&g_light_handle, &l, &s);
		});

		command::add("api_create_light2", "[optional:position] <x> <y> <z>  [optional:radiance] <r> <g> <b>", "RemixApi: Create a light with the 'CreateLight' func\neg: api_create_light 0 0 100 500 250 300", [this]([[maybe_unused]] command::params parms)
		{
			remixapi_LightInfo l = {};
			{
				l.sType = REMIXAPI_STRUCT_TYPE_LIGHT_INFO;
				l.hash = 0x1338;
				l.radiance =
				{
					parms.length() >= 5 ? utils::try_stof(parms[5]) : 1000,  // rad r
					parms.length() >= 6 ? utils::try_stof(parms[6]) : 1200,  // rad g
					parms.length() >= 7 ? utils::try_stof(parms[7]) : 1500   // rad b
				};
			}

			remixapi_LightInfoSphereEXT s = {};
			{
				s.sType = REMIXAPI_STRUCT_TYPE_LIGHT_INFO_SPHERE_EXT;
				s.position =
				{
					parms.length() >= 1 ? utils::try_stof(parms[1]) : 0,	 // x
					parms.length() >= 2 ? utils::try_stof(parms[2]) : 200,	 // y
					parms.length() >= 3 ? utils::try_stof(parms[3]) : -15,	 // z
				};

				s.radius = parms.length() >= 4 ? utils::try_stof(parms[4]) : 1;
				s.shaping_hasvalue = false;
			}

			rtx_api::create_sphere_light(&g_light_handle2, &l, &s);
		});

		command::add("api_destroy_light", "<handle>", "RemixApi: Destroy light with given handle", [this]([[maybe_unused]] command::params parms)
		{
			if(parms.length() == 1)
			{
				game::Com_PrintMessage(0, "Usage: api_destroy_light <handle>", 0);
				return;
			}

			uint64_t handle = utils::try_stoi(parms[1]);
			game::Com_PrintMessage(0, utils::va("Trying to destroy light with handle: %d \n", handle), 0);
			destroy_light(&handle);
		});
	}

	rtx_api::~rtx_api()
	{
		
	}
}