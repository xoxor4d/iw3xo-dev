#include "std_include.hpp"
#include "vector.hpp"
#include "cmath"

namespace glm
{
	// ---------------------------------------------------
	// glm addons

	glm::vec2 to_vec2(const float *value)
	{
		return glm::vec2(value[0], value[1]);
	}

	glm::vec3 to_vec3(const float *value)
	{
		return glm::vec3(value[0], value[1], value[2]);
	}

	glm::vec4 to_vec4(const float *value)
	{
		return glm::vec4(value[0], value[1], value[2], value[3]);
	}

	void set_float2(float *dest, const glm::vec2 &src)
	{
		dest[0] = src.x;
		dest[1] = src.y;
	}

	void set_float3(float *dest, const glm::vec3 &src)
	{
		dest[0] = src.x;
		dest[1] = src.y;
		dest[2] = src.z;
	}

	void set_float4(float *dest, const glm::vec4 &src)
	{
		dest[0] = src.x;
		dest[1] = src.y;
		dest[2] = src.z;
		dest[3] = src.w;
	}
}

namespace utils
{
	namespace vector // https://github.com/id-Software/Quake-III-Arena/blob/master/code/game/q_math.c
	{
		// https://github.com/codereader/DarkRadiant/blob/ba5153ec8f39d2aff1214292f0b94928d434ca4d/libs/math/Matrix4.h

		float rad_to_deg(const float radians)
		{
			return radians * (180.0f / M_PI);
		}

		float deg_to_rad(const float degrees)
		{
			return degrees * M_PI / 180.0f;
		}

		void to_euler_angles(const vec4_t *matrix, vec3_t out)
		{
			const float a = asinf(-matrix[0][2]);
			const float ca = cos(a);

			if (fabsf(ca) > 0.005f)
			{
				out[0] = atan2f(matrix[1][2] / ca, matrix[2][2] / ca);
				out[1] = a;
				out[2] = atan2f(matrix[0][1] / ca, matrix[0][0] / ca);
			}
			else
			{
				out[0] = atan2f(-matrix[2][1] / ca, matrix[1][1] / ca);
				out[1] = a;
				out[2] = 0.0f;
			}
		}

		void to_euler_angles_deg(const vec4_t *matrix, vec3_t out)
		{
			vec3_t euler_rad;
			to_euler_angles(matrix, euler_rad);

			out[0] = rad_to_deg(euler_rad[0]);
			out[1] = rad_to_deg(euler_rad[1]);
			out[2] = rad_to_deg(euler_rad[2]);
		}

		// return true if vec 1 == vec 2
		int compare3(const vec3_t v1, const vec3_t v2) 
		{
			if (v1[0] != v2[0] || v1[1] != v2[1] || v1[2] != v2[2]) 
			{
				return 0;
			}

			return 1;
		}

		// return true if vector 1 +- epsilon equals vector 2
		int compare_epsilon3(const vec3_t v1, const vec3_t v2, float epsilonV1) 
		{
			if ((   v1[0] - epsilonV1 <= v2[0] && v1[0] + epsilonV1 >= v2[0]) 
				&& (v1[1] - epsilonV1 <= v2[1] && v1[1] + epsilonV1 >= v2[1])
				&& (v1[2] - epsilonV1 <= v2[2] && v1[2] + epsilonV1 >= v2[2]) )
			{
				return 1;
			}
			return 0;
		}

		// return true if vec 1 == vec 2
		int compare_int3(const int *v1, const int *v2) 
		{
			if (v1[0] != v2[0] || v1[1] != v2[1] || v1[2] != v2[2]) 
			{
				return 0;
			}

			return 1;
		}

		vec_t length_squared2(const vec2_t v)
		{
			return (v[0] * v[0] + v[1] * v[1]);
		}

		vec_t length_squared3(const vec3_t v)
		{
			return v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
		}

		vec_t length_squared4(const vec4_t v)
		{
			return v[0] * v[0] + v[1] * v[1] + v[2] * v[2] + v[3] * v[3];
		}

		vec_t length2(const vec2_t v)
		{
			return sqrtf(length_squared2(v));
		}

		vec_t length3(const vec3_t v)
		{
			return sqrtf(length_squared3(v));
		}

		

		vec_t distance_squared3(const vec3_t p1, const vec3_t p2)
		{
			vec3_t v = {};

			m_vector_subtract(p2, p1, v);
			return length_squared3(v);
		}

		vec_t distance3(const vec3_t p1, const vec3_t p2) 
		{
			vec3_t v = {};

			m_vector_subtract(p2, p1, v);
			return length3(v);
		}

		// zero input vector
		void zero3(vec3_t v1)
		{
			v1[0] = 0.0f;
			v1[1] = 0.0f;
			v1[2] = 0.0f;
		}

		// zero input vector
		void zero4(vec4_t v1)
		{
			v1[0] = 0.0f;
			v1[1] = 0.0f;
			v1[2] = 0.0f;
			v1[3] = 0.0f;
		}

		// inverse input vector
		void inverse3(vec3_t v)
		{
			v[0] = -v[0];
			v[1] = -v[1];
			v[2] = -v[2];
		}

		// inverse input vector
		void inverse4(vec4_t v)
		{
			v[0] = -v[0];
			v[1] = -v[1];
			v[2] = -v[2];
			v[3] = -v[3];
		}

		// cross product of v1 / v2, result stored in cross
		void cross3(const vec3_t v1, const vec3_t v2, vec3_t cross)
		{
			cross[0] = v1[1] * v2[2] - v1[2] * v2[1];
			cross[1] = v1[2] * v2[0] - v1[0] * v2[2];
			cross[2] = v1[0] * v2[1] - v1[1] * v2[0];
		}

		// normalize input vector
		// returns length of input vector
		vec_t normalize3(vec3_t v)
		{
			//zero3(v);

			const float length = length3(v);
			if (length != 0.0f) 
			{
				const float ilength = 1.0f / length;
				v[0] *= ilength;
				v[1] *= ilength;
				v[2] *= ilength;
			}

			return length;
		}

		// glm 
		float normalize3_glm(glm::vec3 &v)
		{
			//v.x = 0.0f; v.y = 0.0f; v.z = 0.0f;

			const float length = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
			if (length != 0.0f) 
			{
				const float ilength = 1.0f / length;
				v.x *= ilength;
				v.y *= ilength;
				v.z *= ilength;
			}

			return length;
		}

		// normalize input vector and store result in "to" if length != 0
		// returns length of input vector
		vec_t normalize_to(const vec3_t v, vec3_t to)
		{
			zero3(to);

			const float length = length3(v);
			if (length != 0.0f)
			{
				const float ilength = 1.0f / length;
				to[0] = v[0] * ilength;
				to[1] = v[1] * ilength;
				to[2] = v[2] * ilength;
			}

			return length;

		}

		// out = v1 + (scalar * scaled_vec)
		void multiply_add(const vec3_t v1, float scalar, const vec3_t scaled_vec, vec3_t out)
		{
			out[0] = v1[0] + scalar * scaled_vec[0];
			out[1] = v1[1] + scalar * scaled_vec[1];
			out[2] = v1[2] + scalar * scaled_vec[2];
		}

		// dot product
		vec_t dot3(const vec3_t v1, const vec3_t v2)
		{
			return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
		}

		// dot product
		vec_t dot4(const vec4_t v1, const vec4_t v2)
		{
			return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2] + v1[3] * v2[3];
		}

		// out = v1 + v2
		void add3(const vec3_t v1, const vec3_t v2, vec3_t out)
		{
			out[0] = v1[0] + v2[0];
			out[1] = v1[1] + v2[1];
			out[2] = v1[2] + v2[2];
		}

		// out = v1 - v2
		void subtract3(const vec3_t v1, const vec3_t v2, vec3_t out)
		{
			out[0] = v1[0] - v2[0];
			out[1] = v1[1] - v2[1];
			out[2] = v1[2] - v2[2];
		}

		// out = v1 * scalar
		void scale3(const vec3_t v1, float scalar, vec3_t out)
		{
			out[0] = v1[0] * scalar;
			out[1] = v1[1] * scalar;
			out[2] = v1[2] * scalar;
		}

		// out = v1 * scalar
		void scale4(const vec4_t v1, vec_t scalar, vec4_t out)
		{
			out[0] = v1[0] * scalar;
			out[1] = v1[1] * scalar;
			out[2] = v1[2] * scalar;
			out[3] = v1[3] * scalar;
		}

		// calculating trace->endpos (T5)
		// endpos = start + fraction * (end - start)
		void lerp3(const float* start, const float* end, const float fraction, float* endpos)
		{
			if (fraction == 1.0f)
			{
				m_vector_copy3(end, endpos);
			}
			else
			{
				endpos[0] = start[0] + fraction * (end[0] - start[0]);
				endpos[1] = start[1] + fraction * (end[1] - start[1]);
				endpos[2] = start[2] + fraction * (end[2] - start[2]);
			}
		}

		void unpack_unit_vec3(const game::PackedUnitVec v, float* out)
		{
			const float decode_scale = (static_cast<float>((v.packed >> 24)) + 192.0f) / 32385.0f;

			out[0] = static_cast<float>( static_cast<std::uint8_t>( v.array[0] )) - 127.0f * decode_scale;
			out[1] = static_cast<float>( static_cast<std::uint8_t>( v.array[1] )) - 127.0f * decode_scale;
			out[2] = static_cast<float>( static_cast<std::uint8_t>( v.array[2] )) - 127.0f * decode_scale;
		}

		// copy x-sized vector
		void copy(const vec_t* in, vec_t* out, int size = 3)
		{
			for (auto i = 0; i < size; i++)
			{
				out[i] = in[i];
			}
		}

		// out = v1
		void vec4_to_vec3(const vec4_t v1, vec3_t out)
		{
			out[0] = v1[0];
			out[1] = v1[1];
			out[2] = v1[2];
		}

		float angle_normalize180(const float angle)
		{
			return (angle * 0.0027777778f - floorf(angle * 0.0027777778f + 0.5f)) * 360.0f;
		}

		// returns normalized angle in range [-PI <= angle < PI]
		float angle_normalize_pi(const float angle)
		{
			const float t_angle = fmodf(angle + M_PI, 2 * M_PI);
			return t_angle < 0 ? t_angle + M_PI : t_angle - M_PI;
		}

		void angle_vectors(const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up)
		{
			float angle;
			static float sr, sp, sy, cr, cp, cy;

			angle = angles[YAW] * (M_PI * 2.0f / 360.0f);
			sy = sin(angle);
			cy = cos(angle);

			angle = angles[PITCH] * (M_PI * 2.0f / 360.0f);
			sp = sin(angle);
			cp = cos(angle);

			angle = angles[ROLL] * (M_PI * 2.0f / 360.0f);
			sr = sin(angle);
			cr = cos(angle);

			if (forward)
			{
				forward[0] = cp * cy;
				forward[1] = cp * sy;
				forward[2] = -sp;
			}

			if (right)
			{
				right[0] = -1 * sr * sp * cy + -1 * cr * -sy;
				right[1] = -1 * sr * sp * sy + -1 * cr * cy;
				right[2] = -1 * sr * cp;
			}

			if (up)
			{
				up[0] = cr * sp * cy + -sr * -sy;
				up[1] = cr * sp * sy + -sr * cy;
				up[2] = cr * cp;
			}
		}

		void angle_to_forward(const vec3_t angles, vec3_t forward)
		{
			float angle;
			static float sp, sy, cr, cp, cy;

			angle = angles[YAW] * (M_PI * 2.0f / 360.0f);
			sy = sin(angle);
			cy = cos(angle);

			angle = angles[PITCH] * (M_PI * 2.0f / 360.0f);
			sp = sin(angle);
			cp = cos(angle);

			if (forward)
			{
				forward[0] = cp * cy;
				forward[1] = cp * sy;
				forward[2] = -sp;
			}
		}

		void angle_to_right(const vec3_t angles, vec3_t right)
		{
			float angle;
			static float sr, sp, sy, cr, cp, cy;

			angle = angles[YAW] * (M_PI * 2.0f / 360.0f);
			sy = sin(angle);
			cy = cos(angle);

			angle = angles[PITCH] * (M_PI * 2.0f / 360.0f);
			sp = sin(angle);
			cp = cos(angle);

			angle = angles[ROLL] * (M_PI * 2.0f / 360.0f);
			sr = sin(angle);
			cr = cos(angle);

			if (right)
			{
				right[0] = -1 * sr * sp * cy + -1 * cr * -sy;
				right[1] = -1 * sr * sp * sy + -1 * cr * cy;
				right[2] = -1 * sr * cp;
			}
		}

		void vector_to_angles(const vec3_t v1, vec3_t angles) 
		{
			float yaw, pitch;

			if (v1[1] == 0.0f && v1[0] == 0.0f)
			{
				yaw = 0.0f;
				if (v1[2] > 0) 
				{
					pitch = 90.0f;
				}
				else 
				{
					pitch = 270.0f;
				}
			}
			else 
			{
				if (v1[0]) 
				{
					yaw = atan2(v1[1], v1[0]) * 180.0f / M_PI;
				}
				else if (v1[1] > 0) 
				{
					yaw = 90.0f;
				}
				else 
				{
					yaw = 270.0f;
				}

				if (yaw < 0) 
				{
					yaw += 360.0f;
				}

				const float forward = length2(v1);
				pitch = atan2(v1[2], forward) * 180.0f / M_PI;

				if (pitch < 0) 
				{
					pitch += 360.0f;
				}
			}

			angles[PITCH] = -pitch;
			angles[YAW] = yaw;
			angles[ROLL] = 0.0f;
		}

		// https://github.com/xonotic/darkplaces/blob/188aebc4a0ab517ad9f276a20d8e464d1d1ff9a6/mathlib.c#L649
		// LadyHavoc: calculates pitch/yaw/roll angles from forward and up vectors (world forward 1 0 0; world up = 0 0 1) or is forward our normal?
		void vector_to_angles_with_roll(vec3_t angles, const vec3_t forward, const vec3_t up, bool flip_pitch)
		{
			if (forward[0] == 0.0f && forward[1] == 0.0f)
			{
				if (forward[2] > 0.0f)
				{
					angles[PITCH] = -M_PI * 0.5f;
					angles[YAW] = up ? atan2(-up[1], -up[0]) : 0.0f;
				}
				else
				{
					angles[PITCH] = M_PI * 0.5f;
					angles[YAW] = up ? atan2(up[1], up[0]) : 0.0f;
				}

				angles[ROLL] = 0.0f;
			}
			else
			{
				angles[YAW] = atan2(forward[1], forward[0]);
				angles[PITCH] = -atan2(forward[2], sqrt(forward[0] * forward[0] + forward[1] * forward[1]));

				if (up)
				{
					const vec_t cp = cos(angles[PITCH]), sp = sin(angles[PITCH]);
					const vec_t cy = cos(angles[YAW]), sy = sin(angles[YAW]);

					const vec3_t tleft =
					{
						-sy, cy, 0.0f
					};

					const vec3_t tup =
					{
						(sp * cy),
						(sp * sy),
						(cp)
					};

					angles[ROLL] = -atan2(m_dot_product(up, tleft), m_dot_product(up, tup));
				}
				else
				{
					angles[ROLL] = 0;
				}
			}

			// now convert radians to degrees, and make all values positive
			m_vector_scale(angles, 180.0f / M_PI, angles);

			if (flip_pitch)
			{
				angles[PITCH] *= -1.0f;
			}

			if (angles[PITCH] < 0)
			{
				angles[PITCH] += 360.0f;
			}

			if (angles[YAW] < 0)
			{
				angles[YAW] += 360.0f;
			}

			if (angles[ROLL] < 0)
			{
				angles[ROLL] += 360.0f;
			}
		}


		void angles_to_axis(const vec3_t angles, vec3_t axis[3]) 
		{
			const vec3_t vec3_zero = {};
			vec3_t right;

			// angle vectors returns "right" instead of "y axis"
			angle_vectors(angles, axis[0], right, axis[2]);
			m_vector_subtract(vec3_zero, right, axis[1]);
		}

		void create_rotation_matrix(const vec3_t angles, vec3_t matrix[3])
		{
			angle_vectors(angles, matrix[0], matrix[1], matrix[2]);
			inverse3(matrix[1]);
		}

		void rotate_point(vec3_t point, vec3_t matrix[3]) 
		{
			vec3_t tvec;

			m_vector_copy3(point, tvec);
			point[0] = m_dot_product(matrix[0], tvec);
			point[1] = m_dot_product(matrix[1], tvec);
			point[2] = m_dot_product(matrix[2], tvec);
		}

		void unit_quat_to_axis(const float* quat, float(*axis)[3])
		{
			float xx, xy, xz, xw;
			float yy, yz, yw;
			float zz, zw;

			const float scaledX = quat[0] + quat[0];
			xx = scaledX * quat[0];
			xy = scaledX * quat[1];
			xz = scaledX * quat[2];
			xw = scaledX * quat[3];

			const float scaledY = quat[1] + quat[1];
			yy = scaledY * quat[1];
			yz = scaledY * quat[2];
			yw = scaledY * quat[3];

			const float scaledZ = quat[2] + quat[2];
			zz = scaledZ * quat[2];
			zw = scaledZ * quat[3];

			(*axis)[0] = 1.0f - (yy + zz);
			(*axis)[1] = xy + zw;
			(*axis)[2] = xz - yw;
			(*axis)[3] = xy - zw;
			(*axis)[4] = 1.0f - (xx + zz);
			(*axis)[5] = yz + xw;
			(*axis)[6] = xz + yw;
			(*axis)[7] = yz - xw;
			(*axis)[8] = 1.0f - (xx + yy);
		}

		void axis_to_quat(const float(*mat)[3], float* out)
		{
			
			float test[4][4] = {};

			test[0][0] = (*mat)[5] - (*mat)[7];
			test[0][1] = (*mat)[6] - (*mat)[2];
			test[0][2] = (*mat)[1] - (*mat)[3];
			test[0][3] = (*mat)[0] + (*mat)[4] + (*mat)[8] + 1.0f;

			float test_size_sq = utils::vector::length_squared4(test[0]);
			int best = 0;

			if (test_size_sq < 1.0f)
			{
				test[1][0] = (*mat)[6] + (*mat)[2];
				test[1][1] = (*mat)[7] + (*mat)[5];
				test[1][2] = (*mat)[8] - (*mat)[4] - (*mat)[0] + 1.0f;
				test[1][3] = test[0][2];

				test_size_sq = utils::vector::length_squared4(test[1]);
				best = 1;

				if (test_size_sq < 1.0f)
				{
					test[2][0] = (*mat)[0] - (*mat)[4] - (*mat)[8] + 1.0f;
					test[2][1] = (*mat)[3] + (*mat)[1];
					test[2][2] = test[1][0];
					test[2][3] = test[0][0];

					test_size_sq = utils::vector::length_squared4(test[2]);
					best = 2;

					if (test_size_sq < 1.0f)
					{
						test[3][0] = test[2][1];
						test[3][1] = (*mat)[4] - (*mat)[0] - (*mat)[8] + 1.0f;
						test[3][2] = test[1][1];
						test[3][3] = test[0][1];
						test_size_sq = utils::vector::length_squared4(test[3]);
						best = 3;
					}
				}
			}

			const auto inv_length = 1.0f / sqrt(test_size_sq);
			utils::vector::scale4(test[best], inv_length, out);
		}
	}
}