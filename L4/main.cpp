// версия v1.3

#include <SFML/Window.hpp>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Функция для компиляции шейдера
GLuint createShader(GLenum type, const char* source) {
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, nullptr);
	glCompileShader(shader);
	return shader;
}

// Функция для создания программы шейдеров, объединяет вершинный и фрагментный шейдеры в одну шейдерную программу
GLuint createShaderProgram(const char* vertexShaderSource, const char* fragmentShaderSource) {
	GLuint vertexShader = createShader(GL_VERTEX_SHADER, vertexShaderSource);
	GLuint fragmentShader = createShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	return shaderProgram;
}

// Функция для создания цилиндра, создает массив вершин и индексов цилиндра, а затем настраивает VAO, VBO и EBO для использования OpenGL
void setupCylinder(GLuint& VAO, GLuint& VBO, GLuint& EBO) {
	const int numSegments = 12;  // Количество сегментов
	const float radius = 1.0f;
	const float height = 2.0f;

	std::vector<GLfloat> vertices;
	std::vector<GLuint> indices;

	// Генерация вершин цилиндра
	for (int i = 0; i < numSegments; i++) {
		float angle = i * 2.0f * M_PI / numSegments;
		float nextAngle = (i + 1) * 2.0f * M_PI / numSegments;

		float x0 = radius * cos(angle);
		float z0 = radius * sin(angle);
		float x1 = radius * cos(nextAngle);
		float z1 = radius * sin(nextAngle);

		// Нижняя и верхняя части
		vertices.push_back(x0);
		vertices.push_back(-height / 2.0f);
		vertices.push_back(z0);

		vertices.push_back(x1);
		vertices.push_back(-height / 2.0f);
		vertices.push_back(z1);

		vertices.push_back(x0);
		vertices.push_back(height / 2.0f);
		vertices.push_back(z0);

		vertices.push_back(x1);
		vertices.push_back(height / 2.0f);
		vertices.push_back(z1);
	}

	// Индексы для треугольников
	for (int i = 0; i < numSegments; i++) {
		int startIdx = i * 4;
		indices.push_back(startIdx);
		indices.push_back(startIdx + 1);
		indices.push_back(startIdx + 2);

		indices.push_back(startIdx + 1);
		indices.push_back(startIdx + 3);
		indices.push_back(startIdx + 2);
	}

	// Создаем VAO, VBO и EBO
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

// Основной цикл
int main() {
	sf::Window window(sf::VideoMode(1200, 1000), "Projector with cylinder", sf::Style::Default, sf::ContextSettings{ 24 });
	glewInit();

	GLuint VAO, VBO, EBO;
	setupCylinder(VAO, VBO, EBO);

	// Шейдеры
	const char* vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        out vec3 fragPos;  // Передача позиции фрагмента
        void main() {
            fragPos = vec3(model * vec4(aPos, 1.0f));  // Вычисление позиции фрагмента
            gl_Position = projection * view * vec4(fragPos, 1.0f);  // Преобразование в экранные координаты
        })";

	const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        uniform vec3 lightPos;   // Позиция источника света
        uniform vec3 lightDir;   // Направление света (для прожектора)
        uniform vec3 viewPos;    // Позиция камеры
        uniform vec3 lightColor; // Цвет света
        uniform float cutoff;    // Угол конуса прожектора (косинус угла)
        uniform float outerCutoff; // Внешний угол прожектора (косинус угла)
        uniform float constant;  // Коэффициент затухания: постоянный
        uniform float linear;    // Коэффициент затухания: линейный
        uniform float quadratic; // Коэффициент затухания: квадратичный
        in vec3 fragPos;         // Позиция фрагмента, полученная из вершинного шейдера

        void main() {
            vec3 lightDirToFrag = normalize(lightPos - fragPos);
            float theta = dot(lightDirToFrag, normalize(lightDir));
            float epsilon = outerCutoff - cutoff;
            float intensity = clamp((theta - outerCutoff) / epsilon, 0.0, 1.0);

            float distance = length(lightPos - fragPos);
            float attenuation = 1.0f / (constant + linear * distance + quadratic * distance * distance);

            vec3 ambient = 0.1f * lightColor;
            vec3 diffuse = max(dot(normalize(lightPos - fragPos), normalize(vec3(0.0f, 0.0f, 1.0f))), 0.0f) * lightColor;
            vec3 result = (ambient + diffuse) * attenuation * intensity;
            FragColor = vec4(result, 1.0f);
        })";

	GLuint shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);

	// Параметры освещения и камеры
	glm::vec3 lightPos(0.0f, 2.0f, 2.0f);
	glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
	glm::vec3 viewPos(0.0f, 2.0f, 10.0f);
	glm::vec3 lightDir(-1.0f, -1.0f, -1.0f);

	float cutoff = cos(glm::radians(12.5f));
	float outerCutoff = cos(glm::radians(17.5f));

	// Коэффициенты затухания
	float constant = 1.0f;
	float linear = 0.09f;
	float quadratic = 0.032f;

	// Передача параметров в шейдер
	glUseProgram(shaderProgram);
	glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos"), 1, glm::value_ptr(lightPos));
	glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(viewPos));
	glUniform3fv(glGetUniformLocation(shaderProgram, "lightColor"), 1, glm::value_ptr(lightColor));
	glUniform3fv(glGetUniformLocation(shaderProgram, "lightDir"), 1, glm::value_ptr(lightDir));
	glUniform1f(glGetUniformLocation(shaderProgram, "cutoff"), cutoff);
	glUniform1f(glGetUniformLocation(shaderProgram, "outerCutoff"), outerCutoff);
	glUniform1f(glGetUniformLocation(shaderProgram, "constant"), constant);
	glUniform1f(glGetUniformLocation(shaderProgram, "linear"), linear);
	glUniform1f(glGetUniformLocation(shaderProgram, "quadratic"), quadratic);

	// Хранение текущей позиции цилиндра
	glm::vec3 cylinderPosition(0.0f, 0.0f, 0.0f);

	glEnable(GL_DEPTH_TEST);

	// Главный цикл
	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close();

			if (event.type == sf::Event::KeyPressed) {
				// Обработка нажатий клавиш для изменения параметров затухания
				if (event.key.code == sf::Keyboard::Up) {
					quadratic += 0.01f;
					if (quadratic < 0.0f) quadratic = 0.0f;  // Ограничиваем значение
					std::cout << "Quadratic attenuation increased: " << quadratic << std::endl;
				}
				if (event.key.code == sf::Keyboard::Down) {
					quadratic -= 0.01f;
					if (quadratic < 0.0f) quadratic = 0.0f;  // Ограничиваем значение
					std::cout << "Quadratic attenuation decreased: " << quadratic << std::endl;
				}
				if (event.key.code == sf::Keyboard::Right) {
					linear += 0.01f;
					if (linear < 0.0f) linear = 0.0f;  // Ограничиваем значение
					std::cout << "Linear attenuation increased: " << linear << std::endl;
				}
				if (event.key.code == sf::Keyboard::Left) {
					linear -= 0.01f;
					if (linear < 0.0f) linear = 0.0f;  // Ограничиваем значение
					std::cout << "Linear attenuation decreased: " << linear << std::endl;
				}
				if (event.key.code == sf::Keyboard::Equal) {
					constant += 0.01f;
					if (constant < 0.0f) constant = 0.0f;  // Ограничиваем значение
					std::cout << "Constant attenuation increased: " << constant << std::endl;
				}
				if (event.key.code == sf::Keyboard::Dash) {
					constant -= 0.1f;
					if (constant < 0.0f) constant = 0.0f;  // Ограничиваем значение
					std::cout << "Constant attenuation decreased: " << constant << std::endl;
				}

				glUniform1f(glGetUniformLocation(shaderProgram, "constant"), constant);
				glUniform1f(glGetUniformLocation(shaderProgram, "linear"), linear);
				glUniform1f(glGetUniformLocation(shaderProgram, "quadratic"), quadratic);
			}
		}

		// Управление движением цилиндра
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
			cylinderPosition.y += 0.01f;
			std::cout << "Cylinder moved up: " << cylinderPosition.y << std::endl;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
			cylinderPosition.y -= 0.01f;
			std::cout << "Cylinder moved down: " << cylinderPosition.y << std::endl;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
			cylinderPosition.x -= 0.01f;
			std::cout << "Cylinder moved left: " << cylinderPosition.x << std::endl;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
			cylinderPosition.x += 0.01f;
			std::cout << "Cylinder moved right: " << cylinderPosition.x << std::endl;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
			cylinderPosition.z -= 0.01f;
			std::cout << "Cylinder moved backward: " << cylinderPosition.z << std::endl;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) {
			cylinderPosition.z += 0.01f;
			std::cout << "Cylinder moved forward: " << cylinderPosition.z << std::endl;
		}

		glm::mat4 model = glm::translate(glm::mat4(1.0f), cylinderPosition);

		glm::mat4 view = glm::lookAt(viewPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		// Очистка экрана
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Рендеринг цилиндра
		glUseProgram(shaderProgram);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 72, GL_UNSIGNED_INT, 0);

		window.display();
	}

	// Очистка ресурсов
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteProgram(shaderProgram);

	return 0;
}
