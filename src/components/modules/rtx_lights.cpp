#include "std_include.hpp"

namespace components
{
	void rtx_lights::spawn_light()
	{
		D3DLIGHT9 light;
		ZeroMemory(&light, sizeof(D3DLIGHT9));


		const auto setup_light_settings = [](D3DLIGHT9* ff_light, rtx_debug_light* setting)
		{
			if (!ff_light || !setting)
			{
				game::Com_Error(game::ERR_DISCONNECT, "spawn_light::setup_light_settings (!ff_light || !setting)");
				return;
			}

			ff_light->Type = setting->type;

			if (setting->enable)
			{
				// used to turn off the light once rtx_lights[i].enable gets set to false
				setting->disable_hack = 0;

				ff_light->Diffuse.r = setting->color[0] * setting->color_scale;
				ff_light->Diffuse.g = setting->color[1] * setting->color_scale;
				ff_light->Diffuse.b = setting->color[2] * setting->color_scale;
			}
			else
			{
				if (setting->type == D3DLIGHT_DIRECTIONAL && setting->disable_hack && setting->disable_hack < 9)
				{
					const float p = setting->color_scale * 0.5f;

					ff_light->Diffuse.r = setting->color[0] * setting->color_scale / (p * (float)setting->disable_hack);
					ff_light->Diffuse.g = setting->color[1] * setting->color_scale / (p * (float)setting->disable_hack);
					ff_light->Diffuse.b = setting->color[2] * setting->color_scale / (p * (float)setting->disable_hack);
				}
				else
				{
					ff_light->Diffuse.r = 0.0f;
					ff_light->Diffuse.g = 0.0f;
					ff_light->Diffuse.b = 0.0f;
				}
			}

			utils::vector::copy(setting->origin, &ff_light->Position.x, 3);

			ff_light->Range = setting->range;

			ff_light->Attenuation0 = 0.0f;    // no constant inverse attenuation
			ff_light->Attenuation1 = 0.125f;  // only .125 inverse attenuation
			ff_light->Attenuation2 = 0.0f;    // no square inverse attenuation

			// if not a pointlight
			if (setting->type > D3DLIGHT_POINT)
			{
				utils::vector::normalize_to(setting->dir, &ff_light->Direction.x);

				if (setting->inner_angle > setting->outer_angle)
				{
					setting->inner_angle = setting->outer_angle;
				}

				ff_light->Phi = utils::vector::deg_to_rad(setting->outer_angle); // 3.14f / 4.0f;
				ff_light->Theta = utils::vector::deg_to_rad(setting->inner_angle); // 3.14f / 8.0f;
			}

			if (setting->attach && setting->enable)
			{
				game::vec3_t fwd, rt, up = {};
				utils::vector::angle_vectors(game::cgs->predictedPlayerState.viewangles, fwd, rt, up);

				setting->origin[0] = game::glob::lpmove_camera_origin.x + (utils::vector::dot3(fwd, setting->dir_offset));
				setting->origin[1] = game::glob::lpmove_camera_origin.y + (utils::vector::dot3(rt, setting->dir_offset));
				setting->origin[2] = game::glob::lpmove_camera_origin.z + (utils::vector::dot3(up, setting->dir_offset));

				utils::vector::copy(setting->origin, &ff_light->Position.x, 3);

				ff_light->Direction.x = fwd[0];
				ff_light->Direction.y = fwd[1];
				ff_light->Direction.z = fwd[2];

				// update gui settings
				utils::vector::copy(fwd, setting->dir, 3);
			}
		};


		for (auto i = 0; i < RTX_DEBUGLIGHT_AMOUNT; i++)
		{
			if (rtx_debug_lights[i].enable)
			{
				setup_light_settings(&light, &rtx_debug_lights[i]);

				game::glob::d3d9_device->SetLight(i, &light);
				game::glob::d3d9_device->LightEnable(i, TRUE);
			}
			else
			{
				// slightly change position/dir over the next 10 frames so that remix realizes that the light was updated
				// we have to use almost the exact same parameters for this to work
				// fun fact: remix duplicates a light when offsetting the light position by a huge amount within 1 frame
				if (rtx_debug_lights[i].disable_hack < 10)
				{
					setup_light_settings(&light, &rtx_debug_lights[i]);

					light.Position.z += 0.001f;
					light.Direction.z += 0.001f;

					game::glob::d3d9_device->SetLight(i, &light);
					game::glob::d3d9_device->LightEnable(i, TRUE);

					rtx_debug_lights[i].disable_hack++;
				}
				else
				{
					// only updates if raytracing is turned off and on again
					game::glob::d3d9_device->LightEnable(i, FALSE);
				}
			}
		}

		game::glob::d3d9_device->SetRenderState(D3DRS_LIGHTING, FALSE);
	}

	rtx_lights::rtx_lights()
	{
		
	}
}