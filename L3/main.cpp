// доделать!!!

#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <GL/glu.h>
#include <cmath>
#include <iostream>

const float PI = 3.14159265358979323846;
float angle = 0.0f;      // Угол для вращения куба
float radiusX = 5.0f;   // Радиус по оси X
float radiusZ = 5.0f;   // Радиус по оси Z
float speed = 0.01f;     // Скорость движения по окружности

// Функция для рисования однотонного красного куба
void drawCube() {
	glBegin(GL_QUADS);

	// Все грани куба будут красного цвета
	glColor3f(1.0f, 0.0f, 0.0f); // Красный цвет

	// Передняя грань
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);

	// Задняя грань
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);

	// Верхняя грань
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);

	// Нижняя грань
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);

	// Левая грань
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);

	// Правая грань
	glVertex3f(1.0f, -1.0f, -1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);

	glEnd();
}

// Функция для рисования круговой траектории
void drawTrajectory(float radiusX, float radiusZ) {
	glColor3f(0.8f, 0.0f, 0.0f); // Красный цвет для траектории
	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < 360; i++) {
		float theta = i * PI / 180.0f;
		float x = radiusX * cos(theta); // Радиус по оси X
		float z = radiusZ * sin(theta); // Радиус по оси Z
		glVertex3f(x, 0.0f, z);
	}
	glEnd();
}

// Обновление позиции куба
void update(float deltaTime) {
	angle += speed * deltaTime; // Изменение угла
	if (angle >= 2 * PI) {
		angle -= 2 * PI; // Удерживаем угол в пределах 0 - 2PI
	}
}

int main() {
	// Создаем окно
	sf::Window window(sf::VideoMode(800, 600), "Cube Animation", sf::Style::Close | sf::Style::Titlebar);
	window.setActive(true);

	// Настройки OpenGL
	glEnable(GL_DEPTH_TEST); // Включаем тест глубины
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // Цвет фона (белый)

	// Установка перспективы
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, 800.0 / 600.0, 1.0, 100.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	sf::Clock clock; // Часы для отслеживания времени

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close();
			if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::Up) {
					speed += 0.01f; // Увеличиваем скорость
					std::cout << "Speed increased: " << speed << std::endl;
				}
				else if (event.key.code == sf::Keyboard::Down) {
					speed -= 0.01f; // Уменьшаем скорость
					std::cout << "Speed decreased: " << speed << std::endl;
				}
				else if (event.key.code == sf::Keyboard::W) {
					radiusX += 0.1f; // Увеличиваем радиус по X
					std::cout << "Radius X increased: " << radiusX << std::endl;
				}
				else if (event.key.code == sf::Keyboard::S) {
					if (radiusX > 0.1f) { // Уменьшаем радиус по X, не позволяя ему быть отрицательным
						radiusX -= 0.1f;
						std::cout << "Radius X decreased: " << radiusX << std::endl;
					}
				}
			}
		}

		float deltaTime = clock.restart().asSeconds(); // Получаем время с последнего кадра
		update(deltaTime); // Обновляем угол

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Очищаем экран

		glLoadIdentity(); // Сбрасываем матрицу

		// Рассчитываем текущее положение куба
		float cubeX = radiusX * cos(angle); // Используем радиус по оси X
		float cubeZ = radiusZ * sin(angle); // Используем радиус по оси Z
		float cubeY = 0.0f;

		// Установка позиции и ориентации камеры, направляем её на куб
		float cameraX = 0.0f;
		float cameraY = 8.0f;   // Камера находится выше
		float cameraZ = 10.0f;  // Камера дальше по оси Z
		gluLookAt(cameraX, cameraY, cameraZ, cubeX, cubeY, cubeZ, 0.0f, 1.0f, 0.0f);

		drawTrajectory(radiusX, radiusZ); // Рисуем эллиптическую траекторию

		// Перемещаем куб по эллиптической траектории
		glPushMatrix(); // Сохраняем текущую матрицу
		glTranslatef(cubeX, cubeY, cubeZ); // Позиционируем куб по эллипсу
		drawCube(); // Рисуем куб
		glPopMatrix(); // Восстанавливаем матрицу

		window.display(); // Отображаем содержимое окна
	}

	return 0;
}
