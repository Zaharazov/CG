// только куб (это кошмар)

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

// Параметры куба
Vector3 cubeOrigin = { 0.0f, 0.0f, 0.0f };
float cubeSize = 1.0f;
float moveSpeed = 0.05f;

// Функция нормализации вектора
Vector3 normalize(const Vector3& v) {
	float length = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	return { v.x / length, v.y / length, v.z / length };
}

// Вычисление вершин куба с равными сторонами в двухточечной перспективе
std::vector<Vector3> calculateCubeVertices(Vector3 origin, float size) {
	std::vector<Vector3> vertices;

	// Векторы к точкам схода от начальной точки
	Vector3 toLeft = normalize({ vanishingPointLeft.x - origin.x, 0.0f, vanishingPointLeft.z - origin.z });
	Vector3 toRight = normalize({ vanishingPointRight.x - origin.x, 0.0f, vanishingPointRight.z - origin.z });

	// Нижняя грань
	Vector3 P0 = origin;
	Vector3 P1 = { origin.x + toLeft.x * size, origin.y, origin.z + toLeft.z * size };
	Vector3 P2 = { origin.x + toRight.x * size, origin.y, origin.z + toRight.z * size };
	Vector3 P3 = { P1.x + toRight.x * size, origin.y, P1.z + toRight.z * size };

	// Высота куба
	float height = size;

	// Верхняя грань (на высоте height от нижней грани)
	Vector3 P4 = { P0.x, P0.y + height, P0.z };
	Vector3 P5 = { P1.x, P1.y + height, P1.z };
	Vector3 P6 = { P2.x, P2.y + height, P2.z };
	Vector3 P7 = { P3.x, P3.y + height, P3.z };

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

// Функция для рисования куба
// Функция для рисования куба
// Функция для рисования куба с разными цветами для каждой грани
void drawCube(const std::vector<Vector3>& vertices) {
	glBegin(GL_QUADS);

	// Нижняя грань - Красный
	glColor3f(1.0f, 0.0f, 0.0f);  // Красный
	glVertex3f(vertices[0].x, vertices[0].y, vertices[0].z);
	glVertex3f(vertices[1].x, vertices[1].y, vertices[1].z);
	glVertex3f(vertices[3].x, vertices[3].y, vertices[3].z);
	glVertex3f(vertices[2].x, vertices[2].y, vertices[2].z);

	// Верхняя грань - Зеленый
	glColor3f(0.0f, 1.0f, 0.0f);  // Зеленый
	glVertex3f(vertices[4].x, vertices[4].y, vertices[4].z);
	glVertex3f(vertices[5].x, vertices[5].y, vertices[5].z);
	glVertex3f(vertices[7].x, vertices[7].y, vertices[7].z);
	glVertex3f(vertices[6].x, vertices[6].y, vertices[6].z);

	// Передняя грань - Синий
	glColor3f(0.0f, 0.0f, 1.0f);  // Синий
	glVertex3f(vertices[0].x, vertices[0].y, vertices[0].z);
	glVertex3f(vertices[1].x, vertices[1].y, vertices[1].z);
	glVertex3f(vertices[5].x, vertices[5].y, vertices[5].z);
	glVertex3f(vertices[4].x, vertices[4].y, vertices[4].z);

	// Задняя грань - Желтый
	glColor3f(1.0f, 1.0f, 0.0f);  // Желтый
	glVertex3f(vertices[2].x, vertices[2].y, vertices[2].z);
	glVertex3f(vertices[3].x, vertices[3].y, vertices[3].z);
	glVertex3f(vertices[7].x, vertices[7].y, vertices[7].z);
	glVertex3f(vertices[6].x, vertices[6].y, vertices[6].z);

	// Левая грань - Оранжевый
	glColor3f(1.0f, 0.5f, 0.0f);  // Оранжевый
	glVertex3f(vertices[0].x, vertices[0].y, vertices[0].z);
	glVertex3f(vertices[4].x, vertices[4].y, vertices[4].z);
	glVertex3f(vertices[6].x, vertices[6].y, vertices[6].z);
	glVertex3f(vertices[2].x, vertices[2].y, vertices[2].z);

	// Правая грань - Фиолетовый
	glColor3f(0.5f, 0.0f, 1.0f);  // Фиолетовый
	glVertex3f(vertices[1].x, vertices[1].y, vertices[1].z);
	glVertex3f(vertices[5].x, vertices[5].y, vertices[5].z);
	glVertex3f(vertices[7].x, vertices[7].y, vertices[7].z);
	glVertex3f(vertices[3].x, vertices[3].y, vertices[3].z);

	glEnd();
}



// Функция для отображения линий схода
void drawVanishingLines(const std::vector<Vector3>& vertices) {
	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_LINES);

	// Линии схода от левой точки схода
	glVertex3f(vanishingPointLeft.x, vanishingPointLeft.y, vanishingPointLeft.z);
	glVertex3f(vertices[0].x, vertices[0].y, vertices[0].z); // Нижняя передняя
	glVertex3f(vanishingPointLeft.x, vanishingPointLeft.y, vanishingPointLeft.z);
	glVertex3f(vertices[4].x, vertices[4].y, vertices[4].z); // Верхняя передняя

	// Линии схода от правой точки схода
	glVertex3f(vanishingPointRight.x, vanishingPointRight.y, vanishingPointRight.z);
	glVertex3f(vertices[2].x, vertices[2].y, vertices[2].z); // Нижняя передняя
	glVertex3f(vanishingPointRight.x, vanishingPointRight.y, vanishingPointRight.z);
	glVertex3f(vertices[6].x, vertices[6].y, vertices[6].z); // Верхняя передняя

	glEnd();
}

// Функция обработки ввода для перемещения куба
void handleInput(Vector3 &origin) {
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
		origin.z -= moveSpeed;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
		origin.z += moveSpeed;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
		origin.x -= moveSpeed;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
		origin.x += moveSpeed;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
		origin.y += moveSpeed;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) {
		origin.y -= moveSpeed;
	}
}

int main() {
	sf::RenderWindow window(sf::VideoMode(1200, 1000), "Cube with Vanishing Lines in Two-Point Perspective", sf::Style::Default, sf::ContextSettings(24));
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
		handleInput(cubeOrigin);

		// Пересчитываем вершины
		std::vector<Vector3> cubeVertices = calculateCubeVertices(cubeOrigin, cubeSize);

		// Очищаем буфер
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();

		// Камера
		gluLookAt(0.0f, 1.5f, 5.0f, 0.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f);

		// Рисуем куб
		drawCube(cubeVertices);

		// Рисуем линии схода
		drawVanishingLines(cubeVertices);

		window.display();
	}

	return 0;
}
