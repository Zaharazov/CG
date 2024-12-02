// версия 0.5 (сферы и куб)

#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <iostream>
#include <limits>

#define M_PI 3.14159265358979323846

// Вектор для 3D операций
struct Vector3 {
	float x, y, z;

	Vector3() : x(0), y(0), z(0) {}
	Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

	Vector3 operator+(const Vector3& v) const { return Vector3(x + v.x, y + v.y, z + v.z); }
	Vector3 operator-(const Vector3& v) const { return Vector3(x - v.x, y - v.y, z - v.z); }
	Vector3 operator*(float s) const { return Vector3(x * s, y * s, z * s); }
	Vector3 operator*(const Vector3& v) const { return Vector3(x * v.x, y * v.y, z * v.z); } // Покомпонентное умножение
	Vector3 operator/(float s) const { return Vector3(x / s, y / s, z / s); }

	float dot(const Vector3& v) const { return x * v.x + y * v.y + z * v.z; }
	Vector3 normalize() const {
		float len = std::sqrt(x * x + y * y + z * z);
		return *this / len;
	}

	Vector3 cross(const Vector3& v) const {
		return Vector3(
			y * v.z - z * v.y,
			z * v.x - x * v.z,
			x * v.y - y * v.x
		);
	}
};


// Сфера
struct Sphere {
	Vector3 center;
	float radius;
	Vector3 color;
	float reflectivity;
	float transmissivity; // Прозрачность
	float refractiveIndex; // Показатель преломления

	Sphere(const Vector3& c, float r, const Vector3& col, float refl, float trans, float refrIdx)
		: center(c), radius(r), color(col), reflectivity(refl), transmissivity(trans), refractiveIndex(refrIdx) {}

	bool intersect(const Vector3& origin, const Vector3& direction, float& t) const {
		Vector3 oc = origin - center;
		float b = oc.dot(direction);
		float c = oc.dot(oc) - radius * radius;
		float discriminant = b * b - c;
		if (discriminant > 0) {
			float sqrtD = std::sqrt(discriminant);
			t = -b - sqrtD;
			if (t < 0) t = -b + sqrtD;
			return t >= 0;
		}
		return false;
	}
};

// Плоскость
struct Plane {
	Vector3 point;
	Vector3 normal;
	Vector3 color;
	float reflectivity;

	Plane(const Vector3& p, const Vector3& n, const Vector3& col, float refl)
		: point(p), normal(n.normalize()), color(col), reflectivity(refl) {}

	bool intersect(const Vector3& origin, const Vector3& direction, float& t) const {
		float denom = normal.dot(direction);
		if (std::abs(denom) > 1e-6) {
			t = (point - origin).dot(normal) / denom;
			return t >= 0;
		}
		return false;
	}
};

// Куб
struct Cube {
	Vector3 min;
	Vector3 max;
	Vector3 color;
	float reflectivity;
	float transmissivity; // Прозрачность
	float refractiveIndex; // Показатель преломления

	Cube(const Vector3& min, const Vector3& max, const Vector3& col, float refl, float trans, float refrIdx)
		: min(min), max(max), color(col), reflectivity(refl), transmissivity(trans), refractiveIndex(refrIdx) {}

	bool intersect(const Vector3& origin, const Vector3& direction, float& t) const {
		float tMin = (min.x - origin.x) / direction.x;
		float tMax = (max.x - origin.x) / direction.x;
		if (tMin > tMax) std::swap(tMin, tMax);

		float tyMin = (min.y - origin.y) / direction.y;
		float tyMax = (max.y - origin.y) / direction.y;
		if (tyMin > tyMax) std::swap(tyMin, tyMax);

		if ((tMin > tyMax) || (tyMin > tMax)) return false;
		if (tyMin > tMin) tMin = tyMin;
		if (tyMax < tMax) tMax = tyMax;

		float tzMin = (min.z - origin.z) / direction.z;
		float tzMax = (max.z - origin.z) / direction.z;
		if (tzMin > tzMax) std::swap(tzMin, tzMax);

		if ((tMin > tzMax) || (tzMin > tMax)) return false;
		if (tzMin > tMin) tMin = tzMin;
		if (tzMax < tMax) tMax = tzMax;

		t = tMin >= 0 ? tMin : tMax;
		return t >= 0;
	}
};

// Источник света
struct Light {
	Vector3 position;
	Vector3 intensity;

	Light(const Vector3& pos, const Vector3& inten) : position(pos), intensity(inten) {}
};

// Камера
struct Camera {
	Vector3 position;
	Vector3 forward;
	Vector3 up;
	Vector3 right;

	Camera(const Vector3& pos, const Vector3& lookAt, const Vector3& upVec)
		: position(pos) {
		forward = (lookAt - position).normalize();
		right = forward.cross(upVec).normalize();
		up = right.cross(forward);
	}

	Vector3 getRayDirection(float x, float y, float imageWidth, float imageHeight) const {
		float fovScale = tan(M_PI / 4); // 90 градусов
		float aspectRatio = imageWidth / imageHeight;
		float px = (2 * (x + 0.5) / imageWidth - 1) * aspectRatio * fovScale;
		float py = (1 - 2 * (y + 0.5) / imageHeight) * fovScale;
		return (forward + right * px + up * py).normalize();
	}
};

// Преломление
Vector3 refract(const Vector3& I, const Vector3& N, float eta) {
	float cosI = -I.dot(N);
	float sinT2 = eta * eta * (1 - cosI * cosI);
	if (sinT2 > 1) return Vector3(0, 0, 0);
	float cosT = std::sqrt(1 - sinT2);
	return I * eta + N * (eta * cosI - cosT);
}

// Трассировка луча
Vector3 traceRay(const Vector3& origin, const Vector3& direction,
	const std::vector<Sphere>& spheres, const std::vector<Plane>& planes,
	const std::vector<Cube>& cubes, const std::vector<Light>& lights, int depth) {
	if (depth <= 0) return Vector3(0, 0, 0);

	float tMin = std::numeric_limits<float>::infinity();
	Vector3 hitPoint, normal, baseColor;
	float reflectivity = 0;
	float transmissivity = 0;
	float refractiveIndex = 1;

	// Сферы
	for (const auto& sphere : spheres) {
		float t;
		if (sphere.intersect(origin, direction, t) && t < tMin) {
			tMin = t;
			hitPoint = origin + direction * t;
			normal = (hitPoint - sphere.center).normalize();
			baseColor = sphere.color;
			reflectivity = sphere.reflectivity;
			transmissivity = sphere.transmissivity;
			refractiveIndex = sphere.refractiveIndex;
		}
	}

	// Плоскости
	for (const auto& plane : planes) {
		float t;
		if (plane.intersect(origin, direction, t) && t < tMin) {
			tMin = t;
			hitPoint = origin + direction * t;
			normal = plane.normal;
			baseColor = plane.color;
			reflectivity = plane.reflectivity;
		}
	}

	// Кубы
	for (const auto& cube : cubes) {
		float t;
		if (cube.intersect(origin, direction, t) && t < tMin) {
			tMin = t;
			hitPoint = origin + direction * t;

			// Определение нормали
			if (std::abs(hitPoint.x - cube.min.x) < 1e-3) normal = Vector3(-1, 0, 0);
			else if (std::abs(hitPoint.x - cube.max.x) < 1e-3) normal = Vector3(1, 0, 0);
			else if (std::abs(hitPoint.y - cube.min.y) < 1e-3) normal = Vector3(0, -1, 0);
			else if (std::abs(hitPoint.y - cube.max.y) < 1e-3) normal = Vector3(0, 1, 0);
			else if (std::abs(hitPoint.z - cube.min.z) < 1e-3) normal = Vector3(0, 0, -1);
			else if (std::abs(hitPoint.z - cube.max.z) < 1e-3) normal = Vector3(0, 0, 1);

			baseColor = cube.color;
			reflectivity = cube.reflectivity;
			transmissivity = cube.transmissivity;
			refractiveIndex = cube.refractiveIndex;
		}
	}

	if (tMin == std::numeric_limits<float>::infinity()) return Vector3(0, 0, 0); // Ничего не пересечено

	// Освещение
	Vector3 color(0, 0, 0);
	for (const auto& light : lights) {
		Vector3 lightDir = (light.position - hitPoint).normalize();
		Vector3 lightColor = baseColor * std::max(0.f, normal.dot(lightDir));
		color = color + lightColor * light.intensity;
	}

	// Рефлексия
	if (reflectivity > 0) {
		Vector3 reflectDir = direction - normal * 2 * direction.dot(normal);
		color = color + traceRay(hitPoint + normal * 1e-4, reflectDir, spheres, planes, cubes, lights, depth - 1) * reflectivity;
	}

	// Преломление
	if (transmissivity > 0) {
		float eta = direction.dot(normal) < 0 ? 1 / refractiveIndex : refractiveIndex;
		Vector3 refractDir = refract(direction, normal, eta);
		color = color + traceRay(hitPoint - normal * 1e-4, refractDir, spheres, planes, cubes, lights, depth - 1) * transmissivity;
	}

	return color;
}

// Основной рендеринг
int main() {
	const int width = 1200, height = 1000;
	sf::RenderWindow window(sf::VideoMode(width, height), "Ray Tracer");
	sf::Image image;
	image.create(width, height);

	std::vector<Sphere> spheres = {
		Sphere(Vector3(0, -1, 5), 1, Vector3(1, 0, 0), 0.5, 0.5, 1.5),
		Sphere(Vector3(2, 0, 4), 1, Vector3(0, 1, 0), 0.5, 0.5, 1.5),
	};

	std::vector<Plane> planes = {
		Plane(Vector3(0, -2, 0), Vector3(0, 1, 0), Vector3(1, 1, 1), 0.3),
	};

	std::vector<Cube> cubes = {
		Cube(Vector3(-2, 0, 3), Vector3(-1, 1, 4), Vector3(0, 0, 1), 0.4, 0.6, 1.33),
	};

	std::vector<Light> lights = {
		Light(Vector3(0, 5, 0), Vector3(1, 1, 1)),
	};

	Camera camera(Vector3(0, 2, 0.5), Vector3(0, 0, 3), Vector3(0, 1, 0));

	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			Vector3 direction = camera.getRayDirection(x, y, width, height);
			Vector3 color = traceRay(camera.position, direction, spheres, planes, cubes, lights, 5);
			sf::Color pixelColor(
				std::min(255, static_cast<int>(color.x * 255)),
				std::min(255, static_cast<int>(color.y * 255)),
				std::min(255, static_cast<int>(color.z * 255))
			);
			image.setPixel(x, y, pixelColor);
		}
	}

	sf::Texture texture;
	texture.loadFromImage(image);
	sf::Sprite sprite(texture);

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
			}
		}
		window.clear();
		window.draw(sprite);
		window.display();
	}

	return 0;
}
