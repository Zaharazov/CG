// версия 0.1 (только сферы)

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

// Сфера как объект сцены
struct Sphere {
	Vector3 center;
	float radius;
	Vector3 color;
	float reflectivity;

	Sphere(const Vector3& c, float r, const Vector3& col, float refl)
		: center(c), radius(r), color(col), reflectivity(refl) {}

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
	Vector3 point; // Точка на плоскости
	Vector3 normal; // Нормаль к плоскости
	Vector3 color;
	float reflectivity;

	Plane(const Vector3& p, const Vector3& n, const Vector3& col, float refl)
		: point(p), normal(n.normalize()), color(col), reflectivity(refl) {}

	bool intersect(const Vector3& origin, const Vector3& direction, float& t) const {
		float denom = normal.dot(direction);
		if (std::abs(denom) > 1e-6) { // Проверка на параллельность
			t = (point - origin).dot(normal) / denom;
			return t >= 0;
		}
		return false;
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

// Функция для расчёта освещения
Vector3 computeLighting(const Vector3& point, const Vector3& normal, const Vector3& viewDir,
	const std::vector<Light>& lights, const Vector3& baseColor) {
	Vector3 color(0, 0, 0);
	for (const auto& light : lights) {
		Vector3 lightDir = (light.position - point).normalize();

		// Диффузное освещение
		float diff = std::max(0.0f, normal.dot(lightDir));

		// Зеркальное освещение
		Vector3 reflectDir = (lightDir - normal * 2.0f * lightDir.dot(normal)).normalize();
		float spec = std::pow(std::max(viewDir.dot(reflectDir), 0.0f), 32);

		color = color + baseColor * diff + light.intensity * spec;
	}
	return color;
}

// Трассировка луча
Vector3 traceRay(const Vector3& origin, const Vector3& direction,
	const std::vector<Sphere>& spheres, const std::vector<Plane>& planes,
	const std::vector<Light>& lights, int depth) {
	if (depth <= 0) return Vector3(0, 0, 0);

	float tMin = std::numeric_limits<float>::infinity();
	Vector3 hitPoint, normal, baseColor;
	float reflectivity = 0;

	// Поиск пересечения со сферами
	for (const auto& sphere : spheres) {
		float t;
		if (sphere.intersect(origin, direction, t) && t < tMin) {
			tMin = t;
			hitPoint = origin + direction * t;
			normal = (hitPoint - sphere.center).normalize();
			baseColor = sphere.color;
			reflectivity = sphere.reflectivity;
		}
	}

	// Поиск пересечения с плоскостями
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

	if (tMin == std::numeric_limits<float>::infinity()) return Vector3(0.1, 0.1, 0.1); // Цвет фона

	// Базовое освещение
	Vector3 viewDir = (origin - hitPoint).normalize();
	Vector3 localColor = computeLighting(hitPoint, normal, viewDir, lights, baseColor);

	// Рекурсивное отражение
	if (reflectivity > 0) {
		Vector3 reflectDir = direction - normal * 2 * direction.dot(normal);
		Vector3 reflectedColor = traceRay(hitPoint + normal * 0.001, reflectDir, spheres, planes, lights, depth - 1);
		localColor = localColor * (1 - reflectivity) + reflectedColor * reflectivity;
	}

	return localColor;
}

// Основная программа
int main() {
	const int width = 1200, height = 1000;
	sf::RenderWindow window(sf::VideoMode(width, height), "Ray Tracing");

	// Настройка сцены
	std::vector<Sphere> spheres = {
		Sphere(Vector3(0, 0, -5), 1, Vector3(1, 0, 0), 0.5),
		Sphere(Vector3(2, 1, -6), 1, Vector3(0, 1, 0), 0.3),
		Sphere(Vector3(-2, 1, -7), 1, Vector3(0, 0, 1), 0.7),
	};

	std::vector<Plane> planes = {
		Plane(Vector3(0, -1, 0), Vector3(0, 1, 0), Vector3(0.5, 0.5, 0.5), 0.2)
	};

	std::vector<Light> lights = {
		Light(Vector3(0, 10, 0), Vector3(1, 1, 1)), // Верхний источник света
		Light(Vector3(-5, 5, 5), Vector3(0.5, 0.5, 0.5)),
	};

	Camera camera(Vector3(0, 2, 0), Vector3(0, 0, -3), Vector3(0, 1, 0));

	sf::Image image;
	image.create(width, height);

	// Рендеринг
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			Vector3 rayDir = camera.getRayDirection(x, y, width, height);
			Vector3 color = traceRay(camera.position, rayDir, spheres, planes, lights, 5);

			sf::Color pixelColor(
				static_cast<sf::Uint8>(std::min(color.x * 255, 255.0f)),
				static_cast<sf::Uint8>(std::min(color.y * 255, 255.0f)),
				static_cast<sf::Uint8>(std::min(color.z * 255, 255.0f)));

			image.setPixel(x, y, pixelColor);
		}
	}

	sf::Texture texture;
	texture.loadFromImage(image);
	sf::Sprite sprite(texture);

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close();
		}

		window.clear();
		window.draw(sprite);
		window.display();
	}

	return 0;
}
