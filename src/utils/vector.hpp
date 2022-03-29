#pragma once

#define	PITCH				0		// up / down
#define	YAW					1		// left / right
#define	ROLL				2		// fall over

#define M_PI				3.14159265358979323846f

namespace glm
{
	// ---------------------------------------------------
	// glm addons

	glm::vec2 to_vec2(const float *value);
	glm::vec3 to_vec3(const float *value);
	glm::vec4 to_vec4(const float *value);

	void set_float2(float *dest, const glm::vec2 &src);
	void set_float3(float *dest, const glm::vec3 &src);
	void set_float4(float *dest, const glm::vec4 &src);
}

namespace utils
{
	namespace vector // https://github.com/id-Software/Quake-III-Arena/blob/master/code/game/q_shared.h
	{
		typedef float vec_t;
		typedef vec_t vec2_t[2];
		typedef vec_t vec3_t[3];
		typedef vec_t vec4_t[4];
		typedef vec_t vec5_t[5];

		#define m_dot_product(x,y)			((x)[0] * (y)[0] + (x)[1] * (y)[1] + (x)[2] * (y)[2])
		#define m_vector_length(x)			(sqrt((x)[0] * (x)[0] + (x)[1] * (x)[1] + (x)[2] * (x)[2]))
		#define m_vector_length2(x)			(sqrt((x)[0] * (x)[0] + (x)[1] * (x)[1]))
		#define m_vector_negate(a,b)		((b)[0] =- (a)[0], (b)[1] =- (a)[1], (b)[2] =- (a)[2])
		#define m_vector_add(a,b,c)			((c)[0] = (a)[0] + (b)[0], (c)[1] = (a)[1] + (b)[1], (c)[2] = (a)[2] + (b)[2])
		#define m_vector_subtract(a,b,c)	((c)[0] = (a)[0] - (b)[0], (c)[1] = (a)[1] - (b)[1], (c)[2] = (a)[2] - (b)[2])
		#define	m_vector_scale(v, s, o)		((o)[0] = (v)[0] * (s), (o)[1] = (v)[1] * (s), (o)[2] = (v)[2] * (s))
		#define	m_vector_ma(v, s, b, o)		((o)[0] = (v)[0] + (b)[0] * (s), (o)[1] = (v)[1] + (b)[1] * (s), (o)[2] = (v)[2] + (b)[2] * (s))
		#define m_vector_clear(a)			((a)[0] = (a)[1] = (a)[2]=0)
		#define m_vector_set3(v, x, y, z)	((v)[0] = (x), (v)[1] = (y), (v)[2] = (z))
		#define m_vector_copy3(a,b)			((b)[0] = (a)[0], (b)[1] = (a)[1], (b)[2] = (a)[2])
		#define m_vector_copy4(a,b)			((b)[0] = (a)[0], (b)[1] = (a)[1], (b)[2] = (a)[2],(b)[3] = (a)[3])

		float	rad_to_deg(float radians);
		float	deg_to_rad(float degrees);
		void	to_euler_angles(const vec4_t *matrix, vec3_t out);
		void	to_euler_angles_deg(const vec4_t *matrix, vec3_t out);

		int		compare3(const vec3_t v1, const vec3_t v2);
		int		compare_epsilon3(const vec3_t v1, const vec3_t v2, float epsilonV1);
		int		compare_int3(const int *v1, const int *v2);

		vec_t	length_squared2(const vec2_t v);
		vec_t	length_squared3(const vec3_t v);
		vec_t	length2(const vec2_t v);
		vec_t	length3(const vec3_t v);

		vec_t	distance_squared3(const vec3_t p1, const vec3_t p2);
		vec_t	distance3(const vec3_t p1, const vec3_t p2);

		void	zero3(vec3_t v1);
		void	zero4(vec4_t v1);

		void	inverse3(vec3_t v);
		void	inverse4(vec4_t v);

		void	cross3(const vec3_t v1, const vec3_t v2, vec3_t cross);
		vec_t	normalize3(vec3_t v);
		float	normalize3_glm(glm::vec3 &v);
		vec_t	normalize_to(const vec3_t v, vec3_t to);
		
		void	multiply_add(const vec3_t v1, float scalar, const vec3_t scaled_vec, vec3_t out);
		vec_t	dot3(const vec3_t v1, const vec3_t v2);
		vec_t	dot4(const vec4_t v1, const vec4_t v2);
		void	add3(const vec3_t v1, const vec3_t v2, vec3_t out);
		void	subtract3(const vec3_t v1, const vec3_t v2, vec3_t out);
		void	scale3(const vec3_t v1, float scalar, vec3_t out);
		void	scale4(const vec4_t v1, vec_t scalar, vec4_t out);
		void	set3(vec3_t set, const float x, const float y, const float z);
		void	lerp3(const float *start, const float *end, const float fraction, float *endpos);
		void	unpack_unit_vec3(game::PackedUnitVec v, float *out);

		void	copy(const vec_t* in, vec_t* out, int size);
		void	vec4_to_vec3(const vec4_t v1, vec3_t out);

		float	angle_normalize180(float angle);
		float   angle_normalize_pi(float angle);
		void	angle_vectors(const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);
		void	vector_to_angles(const vec3_t v1, vec3_t angles);
		void	vector_to_angles_with_roll(vec3_t angles, const vec3_t forward, const vec3_t up, bool flip_pitch);
		void	angle_to_forward(const vec3_t angles, vec3_t forward);
		void	angle_to_right(const vec3_t angles, vec3_t right);
		void	angles_to_axis(const vec3_t angles, vec3_t axis[3]);

		void	create_rotation_matrix(const vec3_t angles, vec3_t matrix[3]);
		void	rotate_point(vec3_t point, vec3_t matrix[3]);
		void	unit_quat_to_axis(const float* quat, float(*axis)[3]);

		class vec4
		{
		public:
			float x, y, z, w;

			vec4 () : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
			vec4 (const vec4& v) = default;
			vec4 (const float x, const float y, const float z, const float w) : x(x), y(y), z(z), w(w) {}
			vec4 (const float x) : x(x), y(x), z(x), w(x) {}
			vec4 (const float xyzw[3]) : x(xyzw[0]), y(xyzw[1]), z(xyzw[2]), w(xyzw[3]) {}
			~vec4() = default;

			vec4 operator + (const vec4& v) const { return { x + v.x,   y + v.y,   z + v.z,   w + v.w }; }
			vec4 operator - (const vec4& v) const { return { x - v.x,   y - v.y,   z - v.z,   w - v.w }; }
			vec4 operator * (const vec4& v) const { return { x * v.x,   y * v.y,   z * v.z,   w * v.w }; }

			vec4& operator  = (const vec4& v) { x  = v.x;   y  = v.y;   z  = v.z;   w  = v.w; return *this; }
			vec4& operator += (const vec4& v) { x += v.x;   y += v.y;   z += v.z;   w += v.w; return *this; }
			vec4& operator -= (const vec4& v) { x -= v.x;   y -= v.y;   z -= v.z;   w -= v.w; return *this; }

			vec4 operator + (const float s) const { return { x + s,   y + s,   z + s,   w + s }; }
			vec4 operator - (const float s) const { return { x - s,   y - s,   z - s,   w - s }; }
			vec4 operator * (const float s) const { return { x * s,   y * s,   z * s,   w * s }; }
			vec4 operator / (const float s) const { return { x / s,   y / s,   z / s,   w / s }; }

			vec4 operator - () const { return { -x, -y, -z, -w }; }

			friend vec4 operator + (const float lhs, vec4 v) { v += lhs; return v; }
			friend vec4 operator - (const float lhs, vec4 v) { v -= lhs; return v; }
			friend vec4 operator * (const float lhs, vec4 v) { v *= lhs; return v; }
			friend vec4 operator / (const float lhs, vec4 v) { v /= lhs; return v; }

			vec4& operator += (const float s) { x += s;   y += s;   z += s;   w += s; return *this; }
			vec4& operator -= (const float s) { x -= s;   y -= s;   z -= s;   w -= s; return *this; }
			vec4& operator *= (const float s) { x *= s;   y *= s;   z *= s;   w *= s; return *this; }
			vec4& operator /= (const float s) { x /= s;   y /= s;   z /= s;   w /= s; return *this; }

			vec4& normalize()
			{
				const float len = length();
				if (len == 0.0f)
				{
					return *this;
				}

				*this *= (1.0f / len);
				return *this;
			}

			vec4& normalize(float& len_out)
			{
				len_out = length();
				if (len_out == 0.0f)
				{
					return *this;
				}

				*this *= (1.0f / len_out);
				return *this;
			}

			vec4& lerp(const vec4& start, const vec4& end, const float fract)
			{
				if (fract == 1.0f)
				{
					return *this;
				}

				*this = start + fract * (end - start);
				return *this;
			}

			[[nodiscard]] float distance(const vec4 v) const
			{
				const vec4 d(v.x - x, v.y - y, v.z - z, v.w - w);
				return d.length();
			}

			[[nodiscard]] float length_squared() const
			{
				return x * x + y * y + z * z + w * w;
			}

			[[nodiscard]] float length() const
			{
				return std::sqrtf(length_squared());
			}

			static float dot(const vec4& v1, const vec4& v2)
			{
				return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
			}

			void set_float4(float* dest) const
			{
				dest[0] = x;
				dest[1] = y;
				dest[2] = z;
				dest[3] = w;
			}
		};

		class vec3
		{
		public:
			float x, y, z;

			vec3 () : x(0.0f), y(0.0f), z(0.0f) {}
			vec3 (const vec3& v) = default;
			vec3 (const float x, const float y, const float z) : x(x), y(y), z(z) {}
			vec3 (const float x) : x(x), y(x), z(x) {}
			vec3 (const float xyz[3]) : x(xyz[0]), y(xyz[1]), z(xyz[2]) {}
			~vec3() = default;

			vec3 operator + (const vec3& v) const { return { x + v.x,   y + v.y,   z + v.z }; }
			vec3 operator - (const vec3& v) const { return { x - v.x,   y - v.y,   z - v.z }; }
			vec3 operator * (const vec3& v) const { return { x * v.x,   y * v.y,   z * v.z }; }

			vec3& operator  = (const vec3& v) { x = v.x;   y = v.y;   z = v.z; return *this; }
			vec3& operator += (const vec3& v) { x += v.x;   y += v.y;   z += v.z; return *this; }
			vec3& operator -= (const vec3& v) { x -= v.x;   y -= v.y;   z -= v.z; return *this; }

			vec3 operator + (const float s) const { return { x + s,   y + s,   z + s }; }
			vec3 operator - (const float s) const { return { x - s,   y - s,   z - s }; }
			vec3 operator * (const float s) const { return { x * s,   y * s,   z * s }; }
			vec3 operator / (const float s) const { return { x / s,   y / s,   z / s }; }

			vec3 operator - () const { return { -x, -y, -z }; }

			friend vec3 operator + (const float lhs, vec3 v) { v += lhs; return v; }
			friend vec3 operator - (const float lhs, vec3 v) { v -= lhs; return v; }
			friend vec3 operator * (const float lhs, vec3 v) { v *= lhs; return v; }
			friend vec3 operator / (const float lhs, vec3 v) { v /= lhs; return v; }

			vec3& operator += (const float s) { x += s;   y += s;   z += s; return *this; }
			vec3& operator -= (const float s) { x -= s;   y -= s;   z -= s; return *this; }
			vec3& operator *= (const float s) { x *= s;   y *= s;   z *= s; return *this; }
			vec3& operator /= (const float s) { x /= s;   y /= s;   z /= s; return *this; }

			vec3& normalize()
			{
				const float len = length();
				if (len == 0.0f)
				{
					return *this;
				}

				*this *= (1.0f / len);
				return *this;
			}

			vec3& normalize(float& len_out)
			{
				len_out = length();
				if (len_out == 0.0f)
				{
					return *this;
				}

				*this *= (1.0f / len_out);
				return *this;
			}

			vec3& lerp(const vec3& start, const vec3& end, const float fract)
			{
				if (fract == 1.0f)
				{
					return *this;
				}

				*this = start + fract * (end - start);
				return *this;
			}

			vec3& cross(const vec3& v)
			{
				const vec3 og = *this;

				x = og.y * v.z - og.z * v.y;
				y = og.z * v.x - og.x * v.z;
				z = og.x * v.y - og.y * v.x;

				return *this;
			}

			[[nodiscard]] static vec3 cross(const vec3& v1, const vec3& v2)
			{
				return
				{
					(v1.y * v2.z - v1.z * v2.y),
					(v1.z * v2.x - v1.x * v2.z),
					(v1.x * v2.y - v1.y * v2.x)
				};
			}

			[[nodiscard]] float distance(const vec3 v) const
			{
				const vec3 d(v.x - x, v.y - y, v.z - z);
				return d.length();
			}

			[[nodiscard]] float length_squared() const
			{
				return x * x + y * y + z * z;
			}

			[[nodiscard]] float length() const
			{
				return std::sqrtf(length_squared());
			}

			static float dot(const vec3& v1, const vec3& v2)
			{
				return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
			}

			void set_float3(float* dest) const
			{
				dest[0] = x;
				dest[1] = y;
				dest[2] = z;
			}
		};

		class vec2
		{
		public:
			float x, y;

			vec2 () : x(0.0f), y(0.0f) {}
			vec2 (const vec2& v) = default;
			vec2 (const float x, const float y) : x(x), y(y) {}
			vec2 (const float x) : x(x), y(x) {}
			vec2 (const float xy[2]) : x(xy[0]), y(xy[1]) {}

			vec2 (const vec3& v) : x(v.x), y(v.y) {}
			//vec2 (const vec4& v) : x(v.x), y(v.y) {}

			~vec2() = default;

			vec2 operator + (const vec2& v) const { return { x + v.x, y + v.y }; }
			vec2 operator - (const vec2& v) const { return { x - v.x, y - v.y }; }
			vec2 operator * (const vec2& v) const { return { x * v.x, y * v.y }; }

			vec2& operator  = (const vec2& v) { x = v.x; y = v.y; return *this; }
			vec2& operator += (const vec2& v) { x += v.x; y += v.y; return *this; }
			vec2& operator -= (const vec2& v) { x -= v.x; y -= v.y; return *this; }

			vec2 operator + (const float s) const { return { x + s, y + s }; }
			vec2 operator - (const float s) const { return { x - s, y - s }; }
			vec2 operator * (const float s) const { return { x * s, y * s }; }
			vec2 operator / (const float s) const { return { x / s, y / s }; }

			vec2 operator - () const { return { -x, -y }; }

			friend vec2 operator + (const float lhs, vec2 v) { v += lhs; return v; }
			friend vec2 operator - (const float lhs, vec2 v) { v -= lhs; return v; }
			friend vec2 operator * (const float lhs, vec2 v) { v *= lhs; return v; }
			friend vec2 operator / (const float lhs, vec2 v) { v /= lhs; return v; }

			vec2& operator += (const float s) { x += s; y += s; return *this; }
			vec2& operator -= (const float s) { x -= s; y -= s; return *this; }
			vec2& operator *= (const float s) { x *= s; y *= s; return *this; }
			vec2& operator /= (const float s) { x /= s; y /= s; return *this; }

			vec2& normalize()
			{
				const float len = length();
				if (len == 0.0f)
				{
					return *this;
				}

				*this *= (1.0f / len);
				return *this;
			}

			vec2& normalize(float& len_out)
			{
				len_out = length();
				if (len_out == 0.0f)
				{
					return *this;
				}

				*this *= (1.0f / len_out);
				return *this;
			}

			vec2& lerp(const vec2& start, const vec2& end, const float fract)
			{
				if (fract == 1.0f)
				{
					return *this;
				}

				*this = start + fract * (end - start);
				return *this;
			}

			[[nodiscard]] float distance(const vec2 v) const
			{
				const vec2 d(v.x - x, v.y - y);
				return d.length();
			}

			[[nodiscard]] float length_squared() const
			{
				return x * x + y * y;
			}

			[[nodiscard]] float length() const
			{
				return std::sqrtf(length_squared());
			}

			static float dot(const vec2& v1, const vec2& v2)
			{
				return v1.x * v2.x + v1.y * v2.y;
			}

			void set_float2(float* dest) const
			{
				dest[0] = x;
				dest[1] = y;
			}
		};
	}
}