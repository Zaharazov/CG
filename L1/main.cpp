#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <vector>
#include <cmath>
 
const float POINT_RADIUS = 10.0f; // Радиус отображаемых контрольных точек
const float MOVEMENT_RADIUS = 20.0f; // Радиус движения точек
const float MOVEMENT_SPEED = 1.0f; // Скорость движения точек
 
// Структура для представления контрольной точки
struct ControlPoint 
{
	sf::Vector2f startPosition; // Начальная позиция точки
	sf::Vector2f position;      // Текущая позиция точки
	bool isDragging = false;
	float angleOffset = 0.0f;   // Угол для анимации движения
};
 
// Проверка, наведена ли мышь на точку
bool isPointHovered(const sf::Vector2f& mousePos, const sf::Vector2f& pointPos) 
{
	float dx = mousePos.x - pointPos.x;
	float dy = mousePos.y - pointPos.y;
	return std::sqrt(dx * dx + dy * dy) <= POINT_RADIUS;
}
 
// Обновление позиции точки для анимации
void updatePointPosition(ControlPoint& point, float time) 
{
	point.position.x = point.startPosition.x + MOVEMENT_RADIUS * std::cos(time + point.angleOffset);
	point.position.y = point.startPosition.y + MOVEMENT_RADIUS * std::sin(time + point.angleOffset);
}
 
int main() {
	// Создаем рабочее окно
	sf::RenderWindow window(sf::VideoMode(800, 800), "Tochki");
	window.setFramerateLimit(60);
 
	// Вектор исходных контрольных точек
	std::vector<ControlPoint> controlPoints = {
		{{100.0f, 100.0f}, {100.0f, 100.0f}, false, 0.0f},
		{{200.0f, 200.0f}, {200.0f, 200.0f}, false, 1.0f},
		{{300.0f, 300.0f}, {300.0f, 300.0f}, false, 2.0f},
		{{400.0f, 400.0f}, {400.0f, 400.0f}, false, 3.0f},
		{{500.0f, 500.0f}, {500.0f, 500.0f}, false, 4.0f}
	};
 
	sf::Clock clock; // Часы для отслеживания времени
 
	// Основной цикл программы
	while (window.isOpen())
	{
		sf::Event event;
 
		// Есть ли новые события на окне
		while (window.pollEvent(event)) 
		{
			if (event.type == sf::Event::Closed)
				window.close();
 
			// Обработка событий мыши
			if (event.type == sf::Event::MouseButtonPressed) 
			{
				sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);
 
				// Добавление новой точки при правом клике
				if (event.mouseButton.button == sf::Mouse::Right) 
				{
					controlPoints.push_back({ mousePos, mousePos, false, static_cast<float>(controlPoints.size()) });
				}
 
				// Начало перетаскивания точки при левом клике
				else if (event.mouseButton.button == sf::Mouse::Left) 
				{
					for (auto& point : controlPoints) 
					{
						if (isPointHovered(mousePos, point.position)) 
						{
							point.isDragging = true;
							break;
						}
					}
				}
			}
 
			if (event.type == sf::Event::MouseMoved) 
			{
				sf::Vector2f mousePos(event.mouseMove.x, event.mouseMove.y);
				for (auto& point : controlPoints) 
				{
					if (point.isDragging) 
					{
						// Обновляем позицию точки по движению мыши
						point.position = mousePos;
						point.startPosition = mousePos; // Также обновляем начальную позицию
						break;
					}
				}
			}
 
			if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) 
			{
				for (auto& point : controlPoints) 
				{
					point.isDragging = false;
				}
			}
 
			// Добавление новой точки при нажатии клавиши C
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::C) 
			{
				// Получаем позицию мыши, где создадим новую точку
				sf::Vector2i mousePos = sf::Mouse::getPosition(window);  // Позиция мыши в пикселях
				sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)); // Позиция в вещественных значениях
 
				// Добавляем новую точку в вектор controlPoints
				controlPoints.push_back({ mousePosF, mousePosF, false, static_cast<float>(controlPoints.size()) });
			}
 
			// Удаление точки при нажатии клавиши D, если курсор находится над ней
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::D) 
			{
				sf::Vector2i mousePos = sf::Mouse::getPosition(window);  // Получаем позицию мыши
				sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
 
				// Проверяем, находится ли курсор над какой-либо точкой, и удаляем эту точку
				for (auto it = controlPoints.begin(); it != controlPoints.end(); ++it) 
				{
					if (isPointHovered(mousePosF, it->position)) 
					{
						controlPoints.erase(it);
						break;
					}
				}
			}
		}
 
		// Обновляем позиции точек для анимации
		float time = clock.getElapsedTime().asSeconds() * MOVEMENT_SPEED;
		for (auto& point : controlPoints) 
		{
			if (!point.isDragging) 
			{ // Только если точка не перетаскивается пользователем
				updatePointPosition(point, time); // Комментируем, если хотим отключить анимацию
			}
		}
 
		// Очищаем окно
		window.clear(sf::Color::White);
 
		// Настройка 2D ортогональной проекции
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, window.getSize().x, window.getSize().y, 0, -1, 1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
 
		// Включаем OpenGL для рисования ломаной кривой
		glBegin(GL_LINE_STRIP);
		glColor3f(0.0f, 0.0f, 1.0f); // Синий цвет линии
		for (const auto& point : controlPoints) 
		{
			glVertex2f(point.position.x, point.position.y);
		}
		glEnd();
 
		// Рисуем контрольные точки с помощью SFML
		for (const auto& point : controlPoints) 
		{
			sf::CircleShape shape(POINT_RADIUS);
			shape.setPosition(point.position.x - POINT_RADIUS, point.position.y - POINT_RADIUS);
			shape.setFillColor(sf::Color::Red);
			window.draw(shape);
		}
 
		// Отображаем содержимое окна
		window.display();
	}
 
	return 0;
}
 
 
//std::random_device rd;  // Источник случайности
//std::mt19937 gen(rd()); // Генератор псевдослучайных чисел (Mersenne Twister)
 
// 2. Создаем распределение для чисел с плавающей точкой в диапазоне от min до max
//std::uniform_real_distribution<float> dist(0.0f, 1.0f); // Диапазон [0.0, 1.0]
 
// 3. Генерируем случайное число
//float randomValue = dist(gen)
 
 
//sf::ConvexShape convex;
//convex.setPointCount(5); // Многоугольник с 5 вершинами
//convex.setPoint(0, sf::Vector2f(0.0f, 0.0f));
//convex.setPoint(1, sf::Vector2f(150.0f, 10.0f));
//convex.setPoint(2, sf::Vector2f(120.0f, 90.0f));
//convex.setPoint(3, sf::Vector2f(30.0f, 100.0f));
//convex.setPoint(4, sf::Vector2f(10.0f, 50.0f));
//convex.setPosition(point.position.x - POINT_RADIUS, point.position.y - POINT_RADIUS);
//convex.setFillColor(sf::Color::Yellow);
//window.draw(convex);
 
// glBegin(GL_LINE_LOOP); на 159
