#pragma once

#define	PITCH		0		// up / down
#define	YAW			1		// left / right
#define	ROLL		2		// fall over

#define M_RADPI		57.295779513082f
#define M_PI		3.14159265358979323846f

#define DotProduct(x,y)	 ((x)[0]*(y)[0]+(x)[1]*(y)[1]+(x)[2]*(y)[2])

#define DEG2RAD(x)	(static_cast<float>(x) * (M_PI / 180.0f))
#define RAD2DEG(x)	(static_cast<float>(x) * (180.0f / M_PI))

#define DEG2RADF(f)	(f * (M_PI / 180.0f))
#define RAD2DEGF(f)	(f * (180.0f / M_PI))
#include "remix/remix_c.h"

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

class Vector4D
{
public:
	Vector4D(void)
	{
		x = y = z = w = 0.0f;
	}

	Vector4D(float X, float Y, float Z, float W)
	{
		x = X; y = Y; z = Z; w = W;
	}

	Vector4D(float* v)
	{
		x = v[0]; y = v[1]; z = v[2]; w = v[3];
	}

	Vector4D operator+(const Vector4D& v) const
	{
		return Vector4D(x + v.x, y + v.y, z + v.z, w + v.w);
	}

	Vector4D operator-(const Vector4D& v) const
	{
		return Vector4D(x - v.x, y - v.y, z - v.z, w - v.w);
	}

	Vector4D operator*(const Vector4D& v) const
	{
		return Vector4D(x * v.x, y * v.y, z * v.z, w * v.w);
	}

	Vector4D operator/(const Vector4D& v) const
	{
		return Vector4D(x / v.x, y / v.y, z / v.z, w / v.w);
	}

	Vector4D operator+(float v) const
	{
		return Vector4D(x + v, y + v, z + v, w + v);
	}

	Vector4D operator-(float v) const
	{
		return Vector4D(x - v, y - v, z - v, w - v);
	}

	Vector4D operator*(float v) const
	{
		return Vector4D(x * v, y * v, z * v, w * v);
	}

	friend Vector4D operator*(float v, const Vector4D& vec)
	{
		return Vector4D(vec.x * v, vec.y * v, vec.z * v, vec.w * v);
	}

	Vector4D operator/(float v) const
	{
		return Vector4D(x / v, y / v, z / v, w / v);
	}

	Vector4D operator-() const
	{
		return Vector4D(-x, -y, -z, -w);
	}

	game::vec_t x, y, z, w;
};

class Vector2D
{
public:
	Vector2D(void)
	{
		x = y = 0.0f;
	}

	Vector2D(float X, float Y)
	{
		x = X; y = Y;
	}

	Vector2D(float* v)
	{
		x = v[0]; y = v[1];
	}

	Vector2D(const float* v)
	{
		x = v[0]; y = v[1];
	}

	Vector2D(const Vector2D& v)
	{
		x = v.x; y = v.y;
	}

	Vector2D& operator=(const Vector2D& v)
	{
		x = v.x; y = v.y; return *this;
	}

	float& operator[](int i)
	{
		return ((float*)this)[i];
	}

	float operator[](int i) const
	{
		return ((float*)this)[i];
	}

	Vector2D& operator+=(const Vector2D& v)
	{
		x += v.x; y += v.y; return *this;
	}

	Vector2D& operator-=(const Vector2D& v)
	{
		x -= v.x; y -= v.y; return *this;
	}

	Vector2D& operator*=(const Vector2D& v)
	{
		x *= v.x; y *= v.y; return *this;
	}

	Vector2D& operator/=(const Vector2D& v)
	{
		x /= v.x; y /= v.y; return *this;
	}

	Vector2D& operator+=(float v)
	{
		x += v; y += v; return *this;
	}

	Vector2D& operator-=(float v)
	{
		x -= v; y -= v; return *this;
	}

	Vector2D& operator*=(float v)
	{
		x *= v; y *= v; return *this;
	}

	Vector2D& operator/=(float v)
	{
		x /= v; y /= v; return *this;
	}

	Vector2D operator+(const Vector2D& v) const
	{
		return Vector2D(x + v.x, y + v.y);
	}

	Vector2D operator-(const Vector2D& v) const
	{
		return Vector2D(x - v.x, y - v.y);
	}

	Vector2D operator*(const Vector2D& v) const
	{
		return Vector2D(x * v.x, y * v.y);
	}

	Vector2D operator/(const Vector2D& v) const
	{
		return Vector2D(x / v.x, y / v.y);
	}

	Vector2D operator+(float v) const
	{
		return Vector2D(x + v, y + v);
	}

	Vector2D operator-(float v) const
	{
		return Vector2D(x - v, y - v);
	}

	Vector2D operator*(float v) const
	{
		return Vector2D(x * v, y * v);
	}

	Vector2D operator/(float v) const
	{
		return Vector2D(x / v, y / v);
	}

	void Set(float X = 0.0f, float Y = 0.0f)
	{
		x = X; y = Y;
	}

	float Length(void) const
	{
		return ::sqrtf(x * x + y * y);
	}

	float LengthSqr(void) const
	{
		return (x * x + y * y);
	}

	float DistTo(const Vector2D& v) const
	{
		return (*this - v).Length();
	}

	float DistToSqr(const Vector2D& v) const
	{
		return (*this - v).LengthSqr();
	}

	float Dot(const Vector2D& v) const
	{
		return (x * v.x + y * v.y);
	}

	bool IsZero(void) const
	{
		return (x > -0.01f && x < 0.01f &&
			y > -0.01f && y < 0.01f);
	}

public:
	game::vec_t x, y;
};

class Vector
{
public:
	Vector(void)
	{
		x = y = z = 0.0f;
	}

	Vector(float X, float Y, float Z)
	{
		x = X; y = Y; z = Z;
	}

	Vector(float* v)
	{
		x = v[0]; y = v[1]; z = v[2];
	}

	Vector(const float* v)
	{
		x = v[0]; y = v[1]; z = v[2];
	}

	Vector(const Vector& v)
	{
		x = v.x; y = v.y; z = v.z;
	}

	Vector(const Vector4D& v)
	{
		x = v.x; y = v.y; z = v.z;
	}

	Vector(const Vector2D& v)
	{
		x = v.x; y = v.y; z = 0.0f;
	}

	Vector& operator=(const Vector& v)
	{
		x = v.x; y = v.y; z = v.z; return *this;
	}

	Vector& operator=(const Vector2D& v)
	{
		x = v.x; y = v.y; z = 0.0f; return *this;
	}

	float& operator[](int i)
	{
		return ((float*)this)[i];
	}

	float operator[](int i) const
	{
		return ((float*)this)[i];
	}

	Vector& operator+=(const Vector& v)
	{
		x += v.x; y += v.y; z += v.z; return *this;
	}

	Vector& operator-=(const Vector& v)
	{
		x -= v.x; y -= v.y; z -= v.z; return *this;
	}

	Vector& operator*=(const Vector& v)
	{
		x *= v.x; y *= v.y; z *= v.z; return *this;
	}

	Vector& operator/=(const Vector& v)
	{
		x /= v.x; y /= v.y; z /= v.z; return *this;
	}

	Vector& operator+=(float v)
	{
		x += v; y += v; z += v; return *this;
	}

	Vector& operator-=(float v)
	{
		x -= v; y -= v; z -= v; return *this;
	}

	Vector& operator*=(float v)
	{
		x *= v; y *= v; z *= v; return *this;
	}

	Vector& operator/=(float v)
	{
		x /= v; y /= v; z /= v; return *this;
	}

	Vector operator+(const Vector& v) const
	{
		return Vector(x + v.x, y + v.y, z + v.z);
	}

	Vector operator-(const Vector& v) const
	{
		return Vector(x - v.x, y - v.y, z - v.z);
	}

	Vector operator*(const Vector& v) const
	{
		return Vector(x * v.x, y * v.y, z * v.z);
	}

	Vector operator/(const Vector& v) const
	{
		return Vector(x / v.x, y / v.y, z / v.z);
	}

	Vector operator+(float v) const
	{
		return Vector(x + v, y + v, z + v);
	}

	Vector operator-(float v) const
	{
		return Vector(x - v, y - v, z - v);
	}

	Vector operator*(float v) const
	{
		return Vector(x * v, y * v, z * v);
	}

	friend Vector operator*(float v, const Vector& vec)
	{
		return Vector(vec.x * v, vec.y * v, vec.z * v);
	}

	Vector operator/(float v) const
	{
		return Vector(x / v, y / v, z / v);
	}

	Vector operator-() const
	{
		return Vector(-x, -y, -z);
	}

	bool operator==(const Vector& vec) const
	{
		if (std::fabs(x - vec.x) < 1.e-6f
			&& std::fabs(y - vec.y) < 1.e-6f
			&& std::fabs(z - vec.z) < 1.e-6f)
		{
			return true;
		}

		return false;
	}

	bool operator!=(const Vector& vec) const
	{
		if (std::fabs(x - vec.x) >= 1.e-6f
			|| std::fabs(y - vec.y) >= 1.e-6f
			|| std::fabs(z - vec.z) >= 1.e-6f)
		{
			return true;
		}

		return false;
	}

	bool operator>(const Vector& vec) const
	{
		if (x > vec.x && y > vec.y && z > vec.z) {
			return true;
		}

		return false;
	}

	bool operator<(const Vector& vec) const
	{
		if (x < vec.x && y < vec.y && z < vec.z) {
			return true;
		}

		return false;
	}

	float Length(void) const
	{
		return sqrtf(x * x + y * y + z * z);
	}

	float LengthSqr(void) const
	{
		return (x * x + y * y + z * z);
	}

	float Normalize()
	{
		float fl_lenght = Length();
		float fl_lenght_normal = 1.f / ((1.19209290E-07F) + fl_lenght);

		x = x * fl_lenght_normal;
		y = y * fl_lenght_normal;
		z = z * fl_lenght_normal;

		return fl_lenght;
	}

	float NormalizeChecked()
	{
		const float fl_lenght = Length();
		if (fl_lenght != 0.0f)
		{
			const float ilength = 1.0f / fl_lenght;
			x *= ilength;
			y *= ilength;
			z *= ilength;
		}

		return fl_lenght;
	}

	void Rotate(const float flYaw)
	{
		const float r = DEG2RAD(flYaw);
		const float s = sinf(r), c = cosf(r);
		const float flX = x, flY = y;

		x = (flX * c) - (flY * s);
		y = (flX * s) + (flY * c);
	}

	float NormalizeInPlace()
	{
		return Normalize();
	}

	float Length2D(void) const
	{
		return sqrtf(x * x + y * y);
	}

	float Lenght2DSqr(void) const
	{
		return (x * x + y * y);
	}

	float DistTo(const Vector& v) const
	{
		return (*this - v).Length();
	}

	float DistToSqr(const Vector& v) const
	{
		return (*this - v).LengthSqr();
	}

	float Dot(const Vector& v) const
	{
		return (x * v.x + y * v.y + z * v.z);
	}

	Vector Cross(const Vector& v) const
	{
		return Vector(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}

	bool IsZero(float epsilon = 1.e-6f) const
	{
		return (
			x >= -epsilon && x <= epsilon &&
			y >= -epsilon && y <= epsilon &&
			z >= -epsilon && z <= epsilon);
	}

	Vector Scale(float fl) {
		return Vector(x * fl, y * fl, z * fl);
	}

	void Init(float ix = 0.0f, float iy = 0.0f, float iz = 0.0f)
	{
		x = ix; y = iy; z = iz;
	}

	void Add(const Vector& a, const Vector& b)
	{
		x = (a.x + b.x);
		y = (a.y + b.y);
		z = (a.z + b.z);
	}

	bool is_position_within_aabb(const Vector& min_bounds, const Vector& max_bounds, const Vector& position)
	{
		return	position.x >= min_bounds.x && position.x <= max_bounds.x &&
			position.y >= min_bounds.y && position.y <= max_bounds.y &&
			position.z >= min_bounds.z && position.z <= max_bounds.z;
	}

	remixapi_Float3D ToRemixFloat3D() const
	{
		return remixapi_Float3D{ x, y, z };
	}

public:
	game::vec_t x, y, z;
};


class __declspec(align(16))VectorAligned : public Vector
{
public:
	inline VectorAligned(void) {};

	inline VectorAligned(float x, float y, float z) {
		Init(x, y, z);
	}

	explicit VectorAligned(const Vector& othr) {
		Init(othr.x, othr.y, othr.z);
	}

	VectorAligned& operator=(const Vector& othr) {
		Init(othr.x, othr.y, othr.z);
		return *this;
	}

	game::vec_t w = 0.0f;
};

struct Vertex_t
{
	Vertex_t() {}
	Vertex_t(const Vector2D& pos, const Vector2D& coord = Vector2D(0, 0))
	{
		m_Position = pos;
		m_TexCoord = coord;
	}
	void Init(const Vector2D& pos, const Vector2D& coord = Vector2D(0, 0))
	{
		m_Position = pos;
		m_TexCoord = coord;
	}

	Vector2D m_Position;
	Vector2D m_TexCoord;
};

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
		vec_t	length_squared4(const vec4_t v);
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
		void	axis_to_quat(const float(*mat)[3], float* out);

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