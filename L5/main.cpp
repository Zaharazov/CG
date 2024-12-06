// версия 1.0

#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <iostream>
#include <limits>

#define M_PI 3.14159265358979323846

int traceDepth = 5; // глубина трассировки лучей, максимальное количество отражений и преломлений для одного луча

// вектор для 3D операций
struct Vector3 
{
	float x, y, z; // координаты

	Vector3() : x(0), y(0), z(0) {} // конструкторы
	Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
	// перегрузка операторов
	Vector3 operator+(const Vector3& v) const { return Vector3(x + v.x, y + v.y, z + v.z); }
	Vector3 operator-(const Vector3& v) const { return Vector3(x - v.x, y - v.y, z - v.z); }
	Vector3 operator*(float s) const { return Vector3(x * s, y * s, z * s); }
	Vector3 operator*(const Vector3& v) const { return Vector3(x * v.x, y * v.y, z * v.z); } // Покомпонентное умножение
	Vector3 operator/(float s) const { return Vector3(x / s, y / s, z / s); }
	// скалярное произведенеие
	float dot(const Vector3& v) const { return x * v.x + y * v.y + z * v.z; }
	// нормализует вектор
	Vector3 normalize() const 
	{
		float len = std::sqrt(x * x + y * y + z * z);
		return *this / len;
	}
	// векторное произведение
	Vector3 cross(const Vector3& v) const 
	{
		return Vector3(
			y * v.z - z * v.y,
			z * v.x - x * v.z,
			x * v.y - y * v.x
		);
	}
};


// сфера
struct Sphere 
{
	Vector3 center;
	float radius;
	Vector3 color;
	float reflectivity; // отражение
	float transmissivity; // прозрачность
	float refractiveIndex; // показатель преломления
	// конструктор
	Sphere(const Vector3& c, float r, const Vector3& col, float refl, float trans, float refrIdx)
		: center(c), radius(r), color(col), reflectivity(refl), transmissivity(trans), refractiveIndex(refrIdx) {}
	// проверяем, пересекает ли луч сферу
	bool intersect(const Vector3& origin, const Vector3& direction, float& t) const 
	{ // начальная точка луча, направление луча и переменная с расстоянием до точки пересечения
		Vector3 oc = origin - center; // вектор от сферы к н.т. луча
		float b = oc.dot(direction); // насколько проекция вектора совпадает с направлением луча
		float c = oc.dot(oc) - radius * radius; // c для дискриминант
		float discriminant = b * b - c; // дискриминант
		if (discriminant > 0) 
		{
			float sqrtD = std::sqrt(discriminant);
			t = -b - sqrtD;
			if (t < 0) t = -b + sqrtD;
			return t >= 0;
		}
		return false; // нет пересечения
	}
};

// плоскость
struct Plane 
{
	Vector3 point; // исходная точка
	Vector3 normal; // ориентация
	Vector3 color; 
	float reflectivity; // отражжение
	// конструктор
	Plane(const Vector3& p, const Vector3& n, const Vector3& col, float refl)
		: point(p), normal(n.normalize()), color(col), reflectivity(refl) {}
	// аналогичная функция
	bool intersect(const Vector3& origin, const Vector3& direction, float& t) const 
	{
		float denom = normal.dot(direction); // угол между лучом и плоскостью
		if (std::abs(denom) > 1e-6) 
		{ // больше нуля (учитываем погрешность)
			t = (point - origin).dot(normal) / denom; // масштабируем расстояние от н.т. луча до точки пересечения
			return t >= 0;
		}
		return false; // нет пересечения
	}
};

// куб
struct Cube 
{
	Vector3 min; // две противоположные вершины куба
	Vector3 max;
	Vector3 color;
	float reflectivity; // отражение
	float transmissivity; // прозрачность
	float refractiveIndex; // показатель преломления
	// конструктор
	Cube(const Vector3& min, const Vector3& max, const Vector3& col, float refl, float trans, float refrIdx)
		: min(min), max(max), color(col), reflectivity(refl), transmissivity(trans), refractiveIndex(refrIdx) {}
	// аналогичная функция пересечения луча с кубом
	bool intersect(const Vector3& origin, const Vector3& direction, float& t) const 
	{
		float tMin = (min.x - origin.x) / direction.x; // расстояния до передней и задней граней куба
		float tMax = (max.x - origin.x) / direction.x;
		if (tMin > tMax) std::swap(tMin, tMax);

		float tyMin = (min.y - origin.y) / direction.y; // аналогично для y
		float tyMax = (max.y - origin.y) / direction.y;
		if (tyMin > tyMax) std::swap(tyMin, tyMax);

		if ((tMin > tyMax) || (tyMin > tMax)) return false;
		if (tyMin > tMin) tMin = tyMin; // учитываем новую ось
		if (tyMax < tMax) tMax = tyMax;

		float tzMin = (min.z - origin.z) / direction.z; // аналогично для z
		float tzMax = (max.z - origin.z) / direction.z;
		if (tzMin > tzMax) std::swap(tzMin, tzMax);

		if ((tMin > tzMax) || (tzMin > tMax)) return false;
		if (tzMin > tMin) tMin = tzMin; // учитываем новую ось
		if (tzMax < tMax) tMax = tzMax;

		t = tMin >= 0 ? tMin : tMax;
		return t >= 0;
	}
};

// Источник света
struct Light 
{
	Vector3 position;
	Vector3 intensity;

	Light(const Vector3& pos, const Vector3& inten) : position(pos), intensity(inten) {}
};

// камера
struct Camera 
{
	Vector3 position;
	// ориентация
	Vector3 forward;
	Vector3 up;
	Vector3 right;

	// конструктор
	Camera(const Vector3& pos, const Vector3& lookAt, const Vector3& upVec)
		: position(pos) 
	{
		forward = (lookAt - position).normalize(); 
		right = forward.cross(upVec).normalize();
		up = right.cross(forward);
	}
	// считаем направление луча из камеры для пикселя на изображении
	Vector3 getRayDirection(float x, float y, float imageWidth, float imageHeight) const 
	{
		float fovScale = tan(M_PI / 4); // масштаб поля зрения
		float aspectRatio = imageWidth / imageHeight; // соотношение сторон
		// преобразуем пиксельные координаты в пространственные
		float px = (2 * (x + 0.5) / imageWidth - 1) * aspectRatio * fovScale;
		float py = (1 - 2 * (y + 0.5) / imageHeight) * fovScale;
		return (forward + right * px + up * py).normalize(); // вектор направления
	}
};

// преломление
Vector3 refract(const Vector3& I, const Vector3& N, float eta) 
{ // вектор падения, вектор нормали к поверхности, отношение преломлений сред -> вектор направления преломленного луча
	float cosI = -I.dot(N); // косинус угла падения
	float sinT2 = eta * eta * (1 - cosI * cosI);
	if (sinT2 > 1) return Vector3(0, 0, 0); // проверка на полное внутренне отражение
	float cosT = std::sqrt(1 - sinT2); // косинус угла преломления
	return I * eta + N * (eta * cosI - cosT); // корректный вектор преломленного луча
}

// трассировка луча
Vector3 traceRay(const Vector3& origin, const Vector3& direction,
	const std::vector<Sphere>& spheres, const std::vector<Plane>& planes,
	const std::vector<Cube>& cubes, const std::vector<Light>& lights, int depth) 
{
	if (depth <= 0) return Vector3(0, 0, 0); // черный цвет при нулевой глубине

	float tMin = std::numeric_limits<float>::infinity(); // минимальная дистанция до пересечения
	Vector3 hitPoint, normal, baseColor; // точка пересечения луча с объектом, нормаль и цвет в ней
	float reflectivity = 0; // отражение
	float transmissivity = 0; // прозрачность
	float refractiveIndex = 1; // преломление

	// сферы
	for (const auto& sphere : spheres) 
	{
		float t; // проверяем пересечение текущего луча со сферами
		if (sphere.intersect(origin, direction, t) && t < tMin) 
		{
			tMin = t;
			hitPoint = origin + direction * t;
			normal = (hitPoint - sphere.center).normalize();
			baseColor = sphere.color;
			reflectivity = sphere.reflectivity;
			transmissivity = sphere.transmissivity;
			refractiveIndex = sphere.refractiveIndex;
		}
	}

	// плоскости
	for (const auto& plane : planes) 
	{
		float t; // проверяем пересечение текущего луча с плоскостью
		if (plane.intersect(origin, direction, t) && t < tMin) 
		{
			tMin = t;
			hitPoint = origin + direction * t;
			normal = plane.normal;
			baseColor = plane.color;
			reflectivity = plane.reflectivity;
		}
	}

	// кубы
	for (const auto& cube : cubes) 
	{
		float t; // проверяем пересечение текущего луча с кубами
		if (cube.intersect(origin, direction, t) && t < tMin) 
		{
			tMin = t;
			hitPoint = origin + direction * t;

			// определение нормали
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

	if (tMin == std::numeric_limits<float>::infinity()) return Vector3(0, 0, 0); // ничего не пересечено

	// освещение
	Vector3 color(0, 0, 0);
	for (const auto& light : lights) 
	{
		// направление света к точке пересечения
		Vector3 lightDir = (light.position - hitPoint).normalize();
		Vector3 lightColor = baseColor * std::max(0.f, normal.dot(lightDir));
		color = color + lightColor * light.intensity; // итоговый свет
	}

	// рефлексия
	if (reflectivity > 0) 
	{
		// направление отраженного луча
		Vector3 reflectDir = direction - normal * 2 * direction.dot(normal);
		color = color + traceRay(hitPoint + normal * 1e-4, reflectDir, spheres, planes, cubes, lights, depth - 1) * reflectivity;
	}

	// преломление
	if (transmissivity > 0) 
	{
		float eta = direction.dot(normal) < 0 ? 1 / refractiveIndex : refractiveIndex;
		Vector3 refractDir = refract(direction, normal, eta);
		color = color + traceRay(hitPoint - normal * 1e-4, refractDir, spheres, planes, cubes, lights, depth - 1) * transmissivity;
	}

	return color;
}

// основной рендеринг
int main() 
{
	sf::RenderWindow window(sf::VideoMode(1200, 1000), "Traicing luchey");
	sf::Image image; // храним пиксели отрендеренного изображения
	image.create(1200, 1000);

	std::vector<Sphere> spheres = 
	{
		Sphere(Vector3(0, -1, 5), 1, Vector3(1, 0, 0), 0.5, 0.5, 1.5),
		Sphere(Vector3(2, 0, 4), 1, Vector3(0, 1, 0), 0.5, 0.5, 1.5),
	};

	std::vector<Plane> planes = 
	{
		Plane(Vector3(0, -2, 0), Vector3(0, 1, 0), Vector3(1, 1, 1), 0.3),
	};

	std::vector<Cube> cubes = 
	{
		Cube(Vector3(-2.5, 0, 3), Vector3(-1.5, 1, 4), Vector3(0, 0, 1), 0.4, 0.6, 1.33),
		Cube(Vector3(0.5, 0.5, 6), Vector3(1.5, 1.5, 7), Vector3(1, 0, 1), 0.4, 0.6, 1.33),
	};

	std::vector<Light> lights = 
	{
		Light(Vector3(0, 5, 0), Vector3(1, 1, 1)),
		Light(Vector3(5, 7, 5), Vector3(0.1, 0.1, 0.1)),
	};

	Camera camera(Vector3(0, 2, -0.5), Vector3(-1, 0, 3), Vector3(0, 1, 0));

	for (int y = 0; y < 1000; ++y) 
	{
		for (int x = 0; x < 1200; ++x) 
		{
			Vector3 direction = camera.getRayDirection(x, y, 1200, 1000); // направление луча из камеры
			Vector3 color = traceRay(camera.position, direction, spheres, planes, cubes, lights, traceDepth);
			sf::Color pixelColor(
				std::min(255, static_cast<int>(color.x * 255)),
				std::min(255, static_cast<int>(color.y * 255)),
				std::min(255, static_cast<int>(color.z * 255))
			);
			image.setPixel(x, y, pixelColor); // считаем и устанавливаем цвет пикселя
		}
	}

	sf::Texture texture; // создаем текстуру из картинки
	texture.loadFromImage(image);
	sf::Sprite sprite(texture); // для вывода на экран

	while (window.isOpen()) 
	{
		sf::Event event;
		while (window.pollEvent(event)) 
		{
			if (event.type == sf::Event::Closed) 
			{
				window.close();
			}
		}
		window.clear();
		window.draw(sprite); // рисуем спрайт
		window.display();
	}

	return 0;
}
