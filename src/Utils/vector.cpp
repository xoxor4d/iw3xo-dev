#include "STDInclude.hpp"
#include "vector.hpp"
#include "cmath"

namespace glm
{
	// ---------------------------------------------------
	// glm addons

	glm::vec2 toVec2(const float *value)
	{
		return glm::vec2(value[0], value[1]);
	}

	glm::vec3 toVec3(const float *value)
	{
		return glm::vec3(value[0], value[1], value[2]);
	}

	glm::vec4 toVec4(const float *value)
	{
		return glm::vec4(value[0], value[1], value[2], value[3]);
	}

	void setFloat2(float *dest, const glm::vec2 &src) {
		dest[0] = src.x;
		dest[1] = src.y;
	}

	void setFloat3(float *dest, const glm::vec3 &src) {
		dest[0] = src.x;
		dest[1] = src.y;
		dest[2] = src.z;
	}

	void setFloat4(float *dest, const glm::vec4 &src) {
		dest[0] = src.x;
		dest[1] = src.y;
		dest[2] = src.z;
		dest[3] = src.w;
	}
}

namespace Utils
{
	namespace vector // https://github.com/id-Software/Quake-III-Arena/blob/master/code/game/q_math.c
	{
		// https://github.com/codereader/DarkRadiant/blob/ba5153ec8f39d2aff1214292f0b94928d434ca4d/libs/math/Matrix4.h

		float _RadiansToDegrees(const float radians)
		{
			return radians * (180.0f / M_PI);
		}

		float _DegreesToRadians(const float degrees)
		{
			return degrees * M_PI / 180.0f;
		}

		void _ToEulerAngles(vec4_t *matrix, vec3_t out)
		{
			float a = asinf(-matrix[0][2]);
			float ca = cos(a);

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

		void _ToEulerAnglesDegrees(vec4_t *matrix, vec3_t out)
		{
			vec3_t eulerRad;
			_ToEulerAngles(matrix, eulerRad);

			out[0] = _RadiansToDegrees(eulerRad[0]);
			out[1] = _RadiansToDegrees(eulerRad[1]);
			out[2] = _RadiansToDegrees(eulerRad[2]);
		}

		vec3_t	vec3_origin = { 0,0,0 };

		float Q_rsqrt(float number)
		{
			long i;
			float x2, y;
			const float threehalfs = 1.5F;

			x2 = number * 0.5F;
			y = number;
			i = *(long *)&y;						// evil floating point bit level hacking
			i = 0x5f3759df - (i >> 1);              // what the fuck?
			y = *(float *)&i;
			y = y * (threehalfs - (x2 * y * y));	// 1st iteration
			return y;
		}

		// return true if vec 1 == vec 2
		int _VectorCompare(const vec3_t v1, const vec3_t v2) 
		{
			if (v1[0] != v2[0] || v1[1] != v2[1] || v1[2] != v2[2]) 
			{
				return 0;
			}

			return 1;
		}

		// return true if vector 1 +- epsilon equals vector 2
		int _VectorCompareEpsilon(const vec3_t v1, const vec3_t v2, float epsilonV1) 
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
		int _VectorCompareInt(const int *v1, const int *v2) 
		{
			if (v1[0] != v2[0] || v1[1] != v2[1] || v1[2] != v2[2]) 
			{
				return 0;
			}

			return 1;
		}

		vec_t _VectorLength(const vec3_t v)
		{
			return (vec_t)sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
		}

		vec_t _VectorLengthSquared(const vec3_t v) 
		{
			return (v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
		}

		vec_t _VectorLengthSquared2(const vec2_t v)
		{
			return (v[0] * v[0] + v[1] * v[1]);
		}

		vec_t _Distance(const vec3_t p1, const vec3_t p2) 
		{
			vec3_t	v;

			VectorSubtract(p2, p1, v);
			return _VectorLength(v);
		}

		vec_t _DistanceSquared(const vec3_t p1, const vec3_t p2) 
		{
			vec3_t	v;

			VectorSubtract(p2, p1, v);
			return v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
		}

		// zero's input vector
		void _VectorZero(vec3_t v1)
		{
			v1[0] = 0.0f;
			v1[1] = 0.0f;
			v1[2] = 0.0f;
		}

		// zero's input vector
		void _VectorZero4(vec4_t v1)
		{
			v1[0] = 0.0f;
			v1[1] = 0.0f;
			v1[2] = 0.0f;
			v1[3] = 0.0f;
		}

		// inverse input vector
		void _VectorInverse(vec3_t v)
		{
			v[0] = -v[0];
			v[1] = -v[1];
			v[2] = -v[2];
		}

		// cross product of v1 / v2, result stored in cross
		void _CrossProduct(const vec3_t v1, const vec3_t v2, vec3_t cross)
		{
			cross[0] = v1[1] * v2[2] - v1[2] * v2[1];
			cross[1] = v1[2] * v2[0] - v1[0] * v2[2];
			cross[2] = v1[0] * v2[1] - v1[1] * v2[0];
		}

		// fast vector normalize routine that does not check to make sure
		// that length != 0, nor does it return length, uses rsqrt approximation
		void _VectorNormalizeFast(vec3_t v)
		{
			float ilength;

			ilength = Q_rsqrt(DotProduct(v, v));

			v[0] *= ilength;
			v[1] *= ilength;
			v[2] *= ilength;
		}

		// (float) normalize input vector and returns "float length"
		vec_t _VectorNormalize(vec3_t v)
		{
			float	length, ilength;

			length = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);

			if (length) 
			{
				ilength = 1 / length;
				v[0] *= ilength;
				v[1] *= ilength;
				v[2] *= ilength;
			}

			return length;
		}

		// glm 
		float _GLM_VectorNormalize(glm::vec3 &v)
		{
			float	length, ilength;

			length = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);

			if (length) 
			{
				ilength = 1 / length;
				v.x *= ilength;
				v.y *= ilength;
				v.z *= ilength;
			}

			return length;
		}

		// (float) normalize input vector, stores result in output if length != 0 and returns "float length"
		vec_t _VectorNormalize2(const vec3_t v, vec3_t out)
		{
			float	length, ilength;

			length = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
			length = sqrt(length);

			if (length)
			{
				ilength = 1 / length;
				out[0] = v[0] * ilength;
				out[1] = v[1] * ilength;
				out[2] = v[2] * ilength;
			}
			else 
			{
				VectorClear(out);
			}

			return length;

		}

		// out = v1 + (scale * v2)
		void _VectorMA(const vec3_t v1, float scale, const vec3_t v2, vec3_t out)
		{
			out[0] = v1[0] + scale * v2[0];
			out[1] = v1[1] + scale * v2[1];
			out[2] = v1[2] + scale * v2[2];
		}

		// (float) Dot Product of two 3D Vectors
		vec_t _DotProduct(const vec3_t v1, const vec3_t v2)
		{
			return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
		}

		void _VectorSubtract(const vec3_t veca, const vec3_t vecb, vec3_t out)
		{
			out[0] = veca[0] - vecb[0];
			out[1] = veca[1] - vecb[1];
			out[2] = veca[2] - vecb[2];
		}

		void _VectorAdd(const vec3_t veca, const vec3_t vecb, vec3_t out)
		{
			out[0] = veca[0] + vecb[0];
			out[1] = veca[1] + vecb[1];
			out[2] = veca[2] + vecb[2];
		}

		void _VectorCopy(const vec_t* in, vec_t* out, int size = 3)
		{
			for (auto i = 0; i < size; i++)
			{
				out[i] = in[i];
			}
		}

		void _VectorScale(const vec3_t in, float scale, vec3_t out)
		{
			out[0] = in[0] * scale;
			out[1] = in[1] * scale;
			out[2] = in[2] * scale;
		}

		void _Vector4Scale(const vec4_t in, vec_t scale, vec4_t out)
		{
			out[0] = in[0] * scale;
			out[1] = in[1] * scale;
			out[2] = in[2] * scale;
			out[3] = in[3] * scale;
		}

		void _Vector4toVector3(const vec4_t in, vec3_t out)
		{
			out[0] = in[0];
			out[1] = in[1];
			out[2] = in[2];
		}

		float _AngleNormalize180(const float angle)
		{
			float v1;

			v1 = floor((float)(angle * 0.0027777778f) + 0.5f);
			return ((float)(angle * 0.0027777778f) - v1) * 360.0f;
		}

		// returns angle normalized to the range [-PI <= angle < PI]
		float _AngleNormalizePI(float angle)
		{
			angle = fmodf(angle + M_PI, 2 * M_PI);
			return angle < 0 ? angle + M_PI : angle - M_PI;
		}

		void _AngleVectors(const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up)
		{
			float angle;
			static float sr, sp, sy, cr, cp, cy;
			// static to help MS compiler fp bugs

			angle = angles[YAW] * (M_PI * 2 / 360);
			sy = sin(angle);
			cy = cos(angle);
			angle = angles[PITCH] * (M_PI * 2 / 360);
			sp = sin(angle);
			cp = cos(angle);
			angle = angles[ROLL] * (M_PI * 2 / 360);
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
				right[0] = (-1 * sr*sp*cy + -1 * cr*-sy);
				right[1] = (-1 * sr*sp*sy + -1 * cr*cy);
				right[2] = -1 * sr*cp;
			}

			if (up)
			{
				up[0] = (cr*sp*cy + -sr * -sy);
				up[1] = (cr*sp*sy + -sr * cy);
				up[2] = cr * cp;
			}
		}

		void _AngleToForward(const vec3_t angles, vec3_t forward)
		{
			float angle;
			static float sp, sy, cr, cp, cy;

			angle = angles[YAW] * (M_PI * 2 / 360);
			sy = sin(angle);
			cy = cos(angle);
			angle = angles[PITCH] * (M_PI * 2 / 360);
			sp = sin(angle);
			cp = cos(angle);

			if (forward)
			{
				forward[0] = cp * cy;
				forward[1] = cp * sy;
				forward[2] = -sp;
			}
		}

		void _AngleToRight(const vec3_t angles, vec3_t right)
		{
			float angle;
			static float sr, sp, sy, cr, cp, cy;

			angle = angles[YAW] * (M_PI * 2 / 360);
			sy = sin(angle);
			cy = cos(angle);
			angle = angles[PITCH] * (M_PI * 2 / 360);
			sp = sin(angle);
			cp = cos(angle);
			angle = angles[ROLL] * (M_PI * 2 / 360);
			sr = sin(angle);
			cr = cos(angle);

			if (right)
			{
				right[0] = (-1 * sr*sp*cy + -1 * cr*-sy);
				right[1] = (-1 * sr*sp*sy + -1 * cr*cy);
				right[2] = -1 * sr*cp;
			}
		}

		void _VectorToAngles(const vec3_t value1, vec3_t angles) 
		{
			float	forward;
			float	yaw, pitch;

			if (value1[1] == 0 && value1[0] == 0)
			{
				yaw = 0;
				if (value1[2] > 0) 
				{
					pitch = 90;
				}
				else 
				{
					pitch = 270;
				}
			}
			else 
			{
				if (value1[0]) 
				{
					yaw = (atan2(value1[1], value1[0]) * 180 / M_PI);
				}
				else if (value1[1] > 0) 
				{
					yaw = 90;
				}
				else 
				{
					yaw = 270;
				}

				if (yaw < 0) 
				{
					yaw += 360;
				}

				forward = sqrt(value1[0] * value1[0] + value1[1] * value1[1]);
				pitch = (atan2(value1[2], forward) * 180 / M_PI);

				if (pitch < 0) 
				{
					pitch += 360;
				}
			}

			angles[PITCH] = -pitch;
			angles[YAW] = yaw;
			angles[ROLL] = 0;
		}

		// https://github.com/xonotic/darkplaces/blob/188aebc4a0ab517ad9f276a20d8e464d1d1ff9a6/mathlib.c#L649
		// LadyHavoc: calculates pitch/yaw/roll angles from forward and up vectors (world forward 1 0 0; world up = 0 0 1) or is forward our normal?
		void _VectorToAnglesWithRoll(vec3_t angles, const vec3_t forward, const vec3_t up, bool flippitch)
		{
			if (forward[0] == 0 && forward[1] == 0)
			{
				if (forward[2] > 0)
				{
					angles[PITCH] = -M_PI * 0.5;
					angles[YAW] = up ? atan2(-up[1], -up[0]) : 0;
				}
				else
				{
					angles[PITCH] = M_PI * 0.5;
					angles[YAW] = up ? atan2(up[1], up[0]) : 0;
				}

				angles[ROLL] = 0;
			}
			else
			{
				angles[YAW] = atan2(forward[1], forward[0]);
				angles[PITCH] = -atan2(forward[2], sqrt(forward[0] * forward[0] + forward[1] * forward[1]));
				
				// note: we know that angles[PITCH] is in ]-pi/2..pi/2[ due to atan2(anything, positive)
				if (up)
				{
					vec_t cp = cos(angles[PITCH]), sp = sin(angles[PITCH]);
					// note: we know cp > 0, due to the range angles[pitch] is in
					vec_t cy = cos(angles[YAW]), sy = sin(angles[YAW]);
					vec3_t tleft, tup;
					tleft[0] = -sy;
					tleft[1] = cy;
					tleft[2] = 0;
					tup[0] = sp * cy;
					tup[1] = sp * sy;
					tup[2] = cp;
					angles[ROLL] = -atan2(DotProduct(up, tleft), DotProduct(up, tup));

					// for up == '0 0 1', this is
					// angles[ROLL] = -atan2(0, cp);
					// which is 0
				}
				else
				{
					angles[ROLL] = 0;
				}

				// so no up vector is equivalent to '1 0 0'!
			}

			// now convert radians to degrees, and make all values positive
			VectorScale(angles, 180.0f / M_PI, angles);

			if (flippitch)
			{
				angles[PITCH] *= -1;
			}

			if (angles[PITCH] < 0) angles[PITCH] += 360;
			if (angles[YAW] < 0) angles[YAW] += 360;
			if (angles[ROLL] < 0) angles[ROLL] += 360;
		}


		void _AnglesToAxis(const vec3_t angles, vec3_t axis[3]) 
		{
			vec3_t	right;

			// angle vectors returns "right" instead of "y axis"
			_AngleVectors(angles, axis[0], right, axis[2]);
			VectorSubtract(vec3_origin, right, axis[1]);
		}

		// calculating trace->endpos // T5
		void _Vec3Lerp(const float *start, const float *end, const float fraction, float *endpos)
		{
			//endpos[0] = (float)((float)(end[0] - start[0]) * fraction) + start[0];
			//endpos[1] = (float)((float)(end[1] - start[1]) * fraction) + start[1];
			//endpos[2] = (float)((float)(end[2] - start[2]) * fraction) + start[2];

			if (fraction == 1.0f) 
			{
				VectorCopy(end, endpos);
			}
			else
			{
				for (int i = 0; i < 3; i++) 
				{
					endpos[i] = start[i] + fraction * (end[i] - start[i]);
				}
			}
		}

		void _Vec3Cross(const float *v0, const float *v1, float *cross)
		{
			cross[0] = (float)(v0[1] * v1[2]) - (float)(v0[2] * v1[1]);
			cross[1] = (float)(v0[2] * v1[0]) - (float)(v0[0] * v1[2]);
			cross[2] = (float)(v0[0] * v1[1]) - (float)(v0[1] * v1[0]);
		}

		void _Vec3UnpackUnitVec(const Game::PackedUnitVec in, float *out)
		{
			float decodeScale = (float)((float)(in.packed >> 24) + 192.0f) / 32385.0f;

			out[0] = (float)((float)(unsigned __int8)in.array[0] - 127.0) * decodeScale;
			out[1] = (float)((float)(unsigned __int8)in.array[1] - 127.0) * decodeScale;
			out[2] = (float)((float)(unsigned __int8)in.array[2] - 127.0) * decodeScale;
		}

		void _CreateRotationMatrix(const vec3_t angles, vec3_t matrix[3])
		{
			_AngleVectors(angles, matrix[0], matrix[1], matrix[2]);
			_VectorInverse(matrix[1]);
		}

		void _RotatePoint(vec3_t point, /*const*/ vec3_t matrix[3]) 
		{
			vec3_t tvec;

			VectorCopy(point, tvec);
			point[0] = DotProduct(matrix[0], tvec);
			point[1] = DotProduct(matrix[1], tvec);
			point[2] = DotProduct(matrix[2], tvec);
		}

		void UnitQuatToAxis(const float* quat, float(*axis)[3])
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
	}
}