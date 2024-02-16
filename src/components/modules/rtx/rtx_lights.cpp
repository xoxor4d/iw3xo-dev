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


			// #
			// attach light to head / weapon (tag_flash)

			if (setting->attach_to_head && setting->enable)
			{
				game::vec3_t fwd, rt, up = {};
				utils::vector::angle_vectors(game::cgs->predictedPlayerState.viewangles, fwd, rt, up);

				setting->origin[0] = game::cgs->predictedPlayerState.origin[0] + (utils::vector::dot3(fwd, setting->dir_offset));
				setting->origin[1] = game::cgs->predictedPlayerState.origin[1] + (utils::vector::dot3(rt, setting->dir_offset));
				setting->origin[2] = game::cgs->predictedPlayerState.origin[2] + game::cgs->predictedPlayerState.viewHeightCurrent + (utils::vector::dot3(up, setting->dir_offset));
				utils::vector::copy(setting->origin, &ff_light->Position.x, 3);

				setting->dir[0] = fwd[0];
				setting->dir[1] = fwd[1];
				setting->dir[2] = fwd[2];
				utils::vector::copy(setting->dir, &ff_light->Direction.x, 3);
			}
			else if (setting->attach_to_weapon && setting->enable)
			{
				game::vec3_t forward_vec = {};
				game::orientation_t orient = {};
				std::uint8_t bone = 254;

				// we need to handle view and world separately
				// ignore thirdperson if thirdperson-hack is enabled

				if (game::cgs->renderingThirdPerson && !flags::has_flag("thirdperson"))
				{
					auto obj = game::objBuf[game::clientObjMap[game::cgs->predictedPlayerEntity.nextState.number]];
					if (game::DObjGetBoneIndex(&obj, game::scr_const->tag_flash, &bone))
					{
						// get orientation of tag_flash
						game::CG_DObjGetWorldBoneMatrix(&game::cgs->viewModelPose, bone, orient.axis[0], &obj, orient.origin);

						setting->origin[0] = orient.origin[0];
						setting->origin[1] = orient.origin[1];
						setting->origin[2] = orient.origin[2];
						utils::vector::copy(setting->origin, &ff_light->Position.x, 3);

						game::AxisToAngles(setting->dir, orient.axis);
						utils::vector::angle_vectors(setting->dir, forward_vec, nullptr, nullptr);

						ff_light->Direction.x = forward_vec[0];
						ff_light->Direction.y = forward_vec[1];
						ff_light->Direction.z = forward_vec[2];
						utils::vector::copy(forward_vec, setting->dir, 3);
					}
				}
				else // viewweapon (if not thirdperson)
				{
					std::uint32_t weapon_num = game::cgs->predictedPlayerState.offHandIndex;
					if ((game::cgs->predictedPlayerState.weapFlags & 2) == 0)
					{
						weapon_num = game::cgs->predictedPlayerState.weapon;
					}

					if (weapon_num > 0)
					{
						const auto obj = game::cg_weaponsArray[weapon_num].viewModelDObj;
						if (game::DObjGetBoneIndex(obj, game::scr_const->tag_flash, &bone))
						{
							// get orientation of tag_flash
							game::CG_DObjGetWorldBoneMatrix(&game::cgs->viewModelPose, bone, orient.axis[0], obj, orient.origin);

							setting->origin[0] = orient.origin[0];
							setting->origin[1] = orient.origin[1];
							setting->origin[2] = orient.origin[2];
							utils::vector::copy(setting->origin, &ff_light->Position.x, 3);

							game::AxisToAngles(setting->dir, orient.axis);
							utils::vector::angle_vectors(setting->dir, forward_vec, nullptr, nullptr);

							ff_light->Direction.x = forward_vec[0];
							ff_light->Direction.y = forward_vec[1];
							ff_light->Direction.z = forward_vec[2];
							utils::vector::copy(forward_vec, setting->dir, 3);
						}
					}
				}
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

		// needed for fixed-function skysphere (unlit rendering)
		game::glob::d3d9_device->SetRenderState(D3DRS_LIGHTING, FALSE);
	}

	rtx_lights::rtx_lights()
	{
		
	}
}