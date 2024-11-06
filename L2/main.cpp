// куб, пирамида, цилиндр (оно живое)

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <GL/glu.h>
#include <cmath>
#include <vector>

// Определение структуры 3D вектора
struct Vector3 {
	float x, y, z;
};

// Точки схода
Vector3 vanishingPointLeft = { -10.0f, 0.0f, -20.0f };
Vector3 vanishingPointRight = { 10.0f, 0.0f, -20.0f };

#define M_PI 3.14159265358979323846

// Параметры куба
Vector3 cubeOrigin = { 0.0f, 0.0f, 0.0f };
Vector3 pyramidOrigin = { 0.0f, 0.0f, 0.0f };  // Начальная позиция пирамиды
Vector3 cylinderOrigin = { 0.0f, 0.0f, 0.0f };  // Инициализация центра цилиндра


float cubeSize = 1.0f;
float moveSpeed = 0.05f;

// Переменная для отслеживания выбранного объекта
int selectedObject = 3;  // 1 - левая точка схода, 2 - правая точка схода, 3 - куб

// Функция нормализации вектора
Vector3 normalize(const Vector3& v) {
	float length = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	return { v.x / length, v.y / length, v.z / length };
}

// ------------------------------------------------------------------------

// Функция для вычисления вершин цилиндра с двумя точками схода
std::vector<Vector3> calculateCylinderVertices(Vector3 origin, float radius, float height, int segments) {
	std::vector<Vector3> vertices;

	// Векторы к точкам схода от начальной точки
	Vector3 toLeft = normalize({ vanishingPointLeft.x - origin.x, vanishingPointLeft.y - origin.y, vanishingPointLeft.z - origin.z });
	Vector3 toRight = normalize({ vanishingPointRight.x - origin.x, vanishingPointRight.y - origin.y, vanishingPointRight.z - origin.z });

	// Переменные для углов (по умолчанию 360° / segments)
	float angleStep = 2 * M_PI / segments;

	// Вычисляем вершины для нижнего основания цилиндра
	for (int i = 0; i < segments; ++i) {
		float angle = i * angleStep;
		// Координаты точки на окружности
		float x = radius * cos(angle);
		float z = radius * sin(angle);

		// Нижняя окружность
		float y_bottom = origin.y + (toLeft.y * x + toRight.y * z);
		Vector3 bottomPoint = { origin.x + toLeft.x * x + toRight.x * z, y_bottom, origin.z + toLeft.z * x + toRight.z * z };
		vertices.push_back(bottomPoint);

		// Верхняя окружность
		float y_top = origin.y + height + (toLeft.y * x + toRight.y * z);
		Vector3 topPoint = { origin.x + toLeft.x * x + toRight.x * z, y_top, origin.z + toLeft.z * x + toRight.z * z };
		vertices.push_back(topPoint);
	}

	return vertices;
}


// Функция для рисования цилиндра с разными цветами для каждой окружности и боковых граней
void drawCylinder(const std::vector<Vector3>& vertices, int segments) {
	// Рисуем нижнюю окружность (оранжевая)
	glColor3f(1.0f, 0.5f, 0.0f);  // Оранжевый
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(vertices[0].x, vertices[0].y, vertices[0].z); // Центр нижней окружности
	for (int i = 0; i < segments; ++i) {
		glVertex3f(vertices[2 * i].x, vertices[2 * i].y, vertices[2 * i].z);  // Нижняя окружность
	}
	glEnd();

	// Рисуем верхнюю окружность (жёлтая)
	glColor3f(1.0f, 1.0f, 0.0f);  // Жёлтый
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(vertices[1].x, vertices[1].y, vertices[1].z); // Центр верхней окружности
	for (int i = 0; i < segments; ++i) {
		glVertex3f(vertices[2 * i + 1].x, vertices[2 * i + 1].y, vertices[2 * i + 1].z);  // Верхняя окружность
	}
	glEnd();

	// Рисуем боковую поверхность цилиндра (синий)
	glColor3f(0.0f, 0.0f, 1.0f);  // Синий
	glBegin(GL_QUADS);
	for (int i = 0; i < segments; ++i) {
		int next = (i + 1) % segments;

		// Боковые квадраты
		glVertex3f(vertices[2 * i].x, vertices[2 * i].y, vertices[2 * i].z);  // Нижняя
		glVertex3f(vertices[2 * next].x, vertices[2 * next].y, vertices[2 * next].z);  // Нижняя (следующая)
		glVertex3f(vertices[2 * next + 1].x, vertices[2 * next + 1].y, vertices[2 * next + 1].z);  // Верхняя (следующая)
		glVertex3f(vertices[2 * i + 1].x, vertices[2 * i + 1].y, vertices[2 * i + 1].z);  // Верхняя
	}
	glEnd();
}

// Функция для вычисления вершин пирамиды в двухточечной перспективе
std::vector<Vector3> calculatePyramidVertices(Vector3 origin, float baseSize, float height) {
	std::vector<Vector3> vertices;

	// Векторы к точкам схода от начальной точки
	Vector3 toLeft = normalize({ vanishingPointLeft.x - origin.x, vanishingPointLeft.y - origin.y, vanishingPointLeft.z - origin.z });
	Vector3 toRight = normalize({ vanishingPointRight.x - origin.x, vanishingPointRight.y - origin.y, vanishingPointRight.z - origin.z });

	// Основание пирамиды - треугольник
	Vector3 P0 = origin;
	Vector3 P1 = { origin.x + toLeft.x * baseSize, origin.y + toLeft.y * baseSize, origin.z + toLeft.z * baseSize };
	Vector3 P2 = { origin.x + toRight.x * baseSize, origin.y + toRight.y * baseSize, origin.z + toRight.z * baseSize };

	// Апекс пирамиды (верхняя точка)
	Vector3 apex = { origin.x, origin.y + height, origin.z };

	// Добавляем вершины в список
	vertices.push_back(P0);
	vertices.push_back(P1);
	vertices.push_back(P2);
	vertices.push_back(apex);

	return vertices;
}


// Функция для рисования пирамиды с разными цветами для каждой грани
void drawPyramid(const std::vector<Vector3>& vertices) {
	glBegin(GL_TRIANGLES);

	// Передняя грань - Красный
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(vertices[0].x, vertices[0].y, vertices[0].z);
	glVertex3f(vertices[1].x, vertices[1].y, vertices[1].z);
	glVertex3f(vertices[3].x, vertices[3].y, vertices[3].z);

	// Левая грань - Зеленый
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(vertices[1].x, vertices[1].y, vertices[1].z);
	glVertex3f(vertices[2].x, vertices[2].y, vertices[2].z);
	glVertex3f(vertices[3].x, vertices[3].y, vertices[3].z);

	// Правая грань - Синий
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(vertices[2].x, vertices[2].y, vertices[2].z);
	glVertex3f(vertices[0].x, vertices[0].y, vertices[0].z);
	glVertex3f(vertices[3].x, vertices[3].y, vertices[3].z);

	// Основание пирамиды - Оранжевый (Треугольник)
	glColor3f(1.0f, 0.5f, 0.0f);
	glVertex3f(vertices[0].x, vertices[0].y, vertices[0].z);
	glVertex3f(vertices[1].x, vertices[1].y, vertices[1].z);
	glVertex3f(vertices[2].x, vertices[2].y, vertices[2].z);

	glEnd();
}



// Вычисление вершин куба с равными сторонами в двухточечной перспективе
std::vector<Vector3> calculateCubeVertices(Vector3 origin, float size) {
	std::vector<Vector3> vertices;

	// Векторы к точкам схода от начальной точки
	Vector3 toLeft = normalize({ vanishingPointLeft.x - origin.x, vanishingPointLeft.y - origin.y, vanishingPointLeft.z - origin.z });
	Vector3 toRight = normalize({ vanishingPointRight.x - origin.x, vanishingPointRight.y - origin.y, vanishingPointRight.z - origin.z });

	// Нижняя грань куба
	Vector3 P0 = origin;
	Vector3 P1 = { origin.x + toLeft.x * size, origin.y + toLeft.y * size, origin.z + toLeft.z * size };
	Vector3 P2 = { origin.x + toRight.x * size, origin.y + toRight.y * size, origin.z + toRight.z * size };
	Vector3 P3 = { P1.x + toRight.x * size, P1.y + toRight.y * size, P1.z + toRight.z * size };

	// Высота куба
	float height = size;

	// Верхняя грань (P4, P5, P6, P7) должна быть на расстоянии height выше от нижней грани
	Vector3 up = { 0.0f, 1.0f, 0.0f }; // Вектор вверх для верхней грани

	// Верхняя передняя левая вершина
	Vector3 P4 = { P0.x + up.x * height, P0.y + up.y * height, P0.z + up.z * height };

	// Верхняя задняя правая вершина
	Vector3 P5 = { P1.x + up.x * height, P1.y + (up.y - 0.035f) * height, P1.z + up.z * height };

	// Верхняя задняя левая вершина
	Vector3 P6 = { P2.x + up.x * height, P2.y + (up.y - 0.035f) * height, P2.z + up.z * height };

	// Верхняя задняя главная вершина
	Vector3 P7 = { P3.x + up.x * height, P3.y + (up.y - 0.065f) * height , P3.z + up.z * height };

	// Добавляем вершины в список
	vertices.push_back(P0);
	vertices.push_back(P1);
	vertices.push_back(P2);
	vertices.push_back(P3);
	vertices.push_back(P4);
	vertices.push_back(P5);
	vertices.push_back(P6);
	vertices.push_back(P7);

	return vertices;
}



// Функция для рисования куба с разными цветами для каждой грани
void drawCube(const std::vector<Vector3>& vertices) {
	glBegin(GL_QUADS);

	// Нижняя грань - Красный
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(vertices[0].x, vertices[0].y, vertices[0].z);
	glVertex3f(vertices[1].x, vertices[1].y, vertices[1].z);
	glVertex3f(vertices[3].x, vertices[3].y, vertices[3].z);
	glVertex3f(vertices[2].x, vertices[2].y, vertices[2].z);

	// Верхняя грань - Зеленый
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(vertices[4].x, vertices[4].y, vertices[4].z);
	glVertex3f(vertices[5].x, vertices[5].y, vertices[5].z);
	glVertex3f(vertices[7].x, vertices[7].y, vertices[7].z);
	glVertex3f(vertices[6].x, vertices[6].y, vertices[6].z);

	// Передняя грань - Синий
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(vertices[0].x, vertices[0].y, vertices[0].z);
	glVertex3f(vertices[1].x, vertices[1].y, vertices[1].z);
	glVertex3f(vertices[5].x, vertices[5].y, vertices[5].z);
	glVertex3f(vertices[4].x, vertices[4].y, vertices[4].z);

	// Задняя грань - Желтый
	glColor3f(1.0f, 1.0f, 0.0f);
	glVertex3f(vertices[2].x, vertices[2].y, vertices[2].z);
	glVertex3f(vertices[3].x, vertices[3].y, vertices[3].z);
	glVertex3f(vertices[7].x, vertices[7].y, vertices[7].z);
	glVertex3f(vertices[6].x, vertices[6].y, vertices[6].z);

	// Левая грань - Оранжевый
	glColor3f(1.0f, 0.5f, 0.0f);
	glVertex3f(vertices[0].x, vertices[0].y, vertices[0].z);
	glVertex3f(vertices[4].x, vertices[4].y, vertices[4].z);
	glVertex3f(vertices[6].x, vertices[6].y, vertices[6].z);
	glVertex3f(vertices[2].x, vertices[2].y, vertices[2].z);

	// Правая грань - Фиолетовый
	glColor3f(0.5f, 0.0f, 1.0f);
	glVertex3f(vertices[1].x, vertices[1].y, vertices[1].z);
	glVertex3f(vertices[5].x, vertices[5].y, vertices[5].z);
	glVertex3f(vertices[7].x, vertices[7].y, vertices[7].z);
	glVertex3f(vertices[3].x, vertices[3].y, vertices[3].z);

	glEnd();
}

// ------------------------------------------------------------------------

// Функция для отображения линий схода
void drawVanishingLines(const std::vector<Vector3>& vertices) {
	glColor3f(0.5f, 0.5f, 0.5f);  // Серый цвет для линий схода
	glBegin(GL_LINES);

	// Линии схода от левой точки схода
	glVertex3f(vanishingPointLeft.x, vanishingPointLeft.y, vanishingPointLeft.z);
	glVertex3f(vertices[0].x, vertices[0].y, vertices[0].z); // Нижняя передняя
	glVertex3f(vanishingPointLeft.x, vanishingPointLeft.y, vanishingPointLeft.z);
	glVertex3f(vertices[4].x, vertices[4].y, vertices[4].z); // Верхняя передняя
	glVertex3f(vanishingPointLeft.x, vanishingPointLeft.y, vanishingPointLeft.z);
	glVertex3f(vertices[6].x, vertices[6].y, vertices[6].z); // Верхняя справа

	// Линии схода от правой точки схода
	glVertex3f(vanishingPointRight.x, vanishingPointRight.y, vanishingPointRight.z);
	glVertex3f(vertices[0].x, vertices[0].y, vertices[0].z); // Нижняя передняя
	glVertex3f(vanishingPointRight.x, vanishingPointRight.y, vanishingPointRight.z);
	glVertex3f(vertices[4].x, vertices[4].y, vertices[4].z); // Верхняя передняя
	glVertex3f(vanishingPointRight.x, vanishingPointRight.y, vanishingPointRight.z);
	glVertex3f(vertices[5].x, vertices[5].y, vertices[5].z); // Верхняя слева

	glEnd();
}

// Функция обработки ввода для перемещения выбранного объекта
void handleInput() {
	Vector3* selectedObjectPosition = nullptr;

	// Логика выбора объекта для управления
	if (selectedObject == 1) {
		selectedObjectPosition = &vanishingPointLeft;  // Левая точка схода
	}
	else if (selectedObject == 2) {
		selectedObjectPosition = &vanishingPointRight;  // Правая точка схода
	}
	else if (selectedObject == 3) {
		selectedObjectPosition = &cubeOrigin;  // Куб
	}
	else if (selectedObject == 4) {
		selectedObjectPosition = &pyramidOrigin;  // Пирамида
	}
	else if (selectedObject == 5) {
		selectedObjectPosition = &cylinderOrigin;  // Цилиндр
	}

	// Если объект выбран, обрабатываем его движение
	if (selectedObjectPosition != nullptr) {
		if (selectedObject == 5) {
			// Обработка ввода для цилиндра
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
				selectedObjectPosition->z -= moveSpeed;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
				selectedObjectPosition->z += moveSpeed;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
				selectedObjectPosition->x -= moveSpeed;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
				selectedObjectPosition->x += moveSpeed;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
				selectedObjectPosition->y += moveSpeed;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) {
				selectedObjectPosition->y -= moveSpeed;
			}
		}
		else {
			// Обработка ввода для остальных объектов
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
				selectedObjectPosition->z -= moveSpeed;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
				selectedObjectPosition->z += moveSpeed;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
				selectedObjectPosition->x -= moveSpeed;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
				selectedObjectPosition->x += moveSpeed;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
				selectedObjectPosition->y += moveSpeed;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) {
				selectedObjectPosition->y -= moveSpeed;
			}
		}
	}

	// Переключение между объектами
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) {
		selectedObject = 1;  // Выбираем левую точку схода
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) {
		selectedObject = 2;  // Выбираем правую точку схода
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3)) {
		selectedObject = 3;  // Выбираем куб
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num4)) {
		selectedObject = 4;  // Выбираем пирамиду
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num5)) {
		selectedObject = 5;  // Выбираем цилиндр
	}
}




int main() {
	sf::RenderWindow window(sf::VideoMode(1200, 1000), "Cube, Pyramid and Cylinder with Vanishing Lines", sf::Style::Default, sf::ContextSettings(24));
	window.setFramerateLimit(60);

	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, window.getSize().x / (float)window.getSize().y, 1.0f, 100.0f);
	glMatrixMode(GL_MODELVIEW);

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
			}
		}

		// Обработка ввода
		handleInput();

		// Пересчитываем вершины куба
		std::vector<Vector3> cubeVertices = calculateCubeVertices(cubeOrigin, cubeSize);
		// Пересчитываем вершины пирамиды
		std::vector<Vector3> pyramidVertices = calculatePyramidVertices(pyramidOrigin, cubeSize, 1.0f);
		// Пересчитываем вершины цилиндра
		std::vector<Vector3> cylinderVertices = calculateCylinderVertices(cylinderOrigin, 0.5f, cubeSize * 0.9f, 20); // 20 сегментов

		// Очищаем буфер
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();

		// Камера
		gluLookAt(0.0f, 1.5f, 5.0f, 0.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f);

		// Рисуем куб
		drawCube(cubeVertices);
		// Рисуем пирамиду
		drawPyramid(pyramidVertices);
		// Рисуем цилиндр
		drawCylinder(cylinderVertices, 20);  // 20 сегментов цилиндра

		// Рисуем линии схода
		drawVanishingLines(cubeVertices);

		window.display();
	}

	return 0;
}
