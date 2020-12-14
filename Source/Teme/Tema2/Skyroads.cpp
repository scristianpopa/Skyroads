#include "Skyroads.h"

#include <vector>
#include <string>
#include <iostream>

#include <Core/Engine.h>

using namespace std;

Skyroads::Skyroads() :
	tileRows(3),
	ballX(0.f),
	ballY(1.0f),
	ballSpeedX(1.1f),
	ballSpeedY(0.5f),
	tileHeight(0.2f),
	tileLength(1.f),
	tileWidth(0.5f),
	newTilesTimer(0.f),
	minSpeed(1.f),
	maxSpeed(3.5f),
	tileSpeed(2.f),
	oldSpeed(tileSpeed),
	jumping(false),
	alive(true),
	third(true),
	ballColor(glm::vec3(1, 1, 1)),
	turbo(0.f),
	fuel(MAXFUEL),
	waitDead(0.8f),
	cameraY(0.5f),
	refuels(2),
	score(0)
{
}

Skyroads::~Skyroads()
{
}

void Skyroads::Init()
{
	{
		Mesh* mesh = new Mesh("box");
		mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "box.obj");
		meshes[mesh->GetMeshID()] = mesh;
	}

	{
		Mesh* mesh = new Mesh("sphere");
		mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "sphere.obj");
		meshes[mesh->GetMeshID()] = mesh;
	}

	{
		Mesh* mesh = new Mesh("wall");
		mesh->LoadMesh(RESOURCE_PATH::MODELS + "Props", "concrete_wall.obj");
		meshes[mesh->GetMeshID()] = mesh;
	}

	{
		Mesh* mesh = new Mesh("pump");
		mesh->LoadMesh(RESOURCE_PATH::MODELS + "Props", "pompabenzina.obj");
		meshes[mesh->GetMeshID()] = mesh;
	}

	{
		Shader *shader = new Shader("Skyroads");
		shader->AddShader("Source/Teme/Tema2/Shaders/VertexShader.glsl", GL_VERTEX_SHADER);
		shader->AddShader("Source/Teme/Tema2/Shaders/FragmentShader.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}

	{
		projectionMatrix = glm::perspective(RADIANS(60), window->props.aspectRatio, 0.01f, 200.0f);
		viewMatrix = glm::lookAt(glm::vec3(0, cameraY, -0.27f), glm::vec3(0, cameraY - 0.1, 1), glm::vec3(0, 1, 0));
		orthoViewMatrix = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));
		orthoProjectionMatrix = glm::ortho(-2.f, 2.f, -2.f, 2.f, 0.01f, 200.0f);
	}

	{
		tiles.resize(tileRows);
		for (int start = 0; start < 15; ++start) 
			for (int i = 0; i < tileRows; ++i) {
				tiles[i].emplace_back(start * tileLength + tileLength / 2, BLUE);
			}
	}
	
	{
		tileMatrix = glm::mat4(1);
		tileMatrix = glm::scale(tileMatrix, glm::vec3(tileWidth, tileHeight, tileLength));

		ballScale = glm::scale(glm::mat4(1), glm::vec3(0.2f));
	}

	srand((unsigned int)time(NULL));
}

void Skyroads::FrameStart()
{
	// clears the color buffer (using the previously set color) and depth buffer
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::ivec2 resolution = window->GetResolution();
	// sets the screen area where to draw
	glViewport(0, 0, resolution.x, resolution.y);	
}

void Skyroads::Update(float deltaTimeSeconds)
{
	if (!alive) {
		DeathComputations(deltaTimeSeconds);
	}
	
	RenderBackTexture();
	
	RenderSimpleMesh(meshes["sphere"], shaders["Skyroads"], glm::translate(glm::mat4(1), glm::vec3(ballX, ballY, 0.5f)) * ballScale, ballColor, turbo);
	
	RenderFuel();

	RenderTiles(deltaTimeSeconds);
	
	ScoreCheck();
	
	if (!third) {
		viewMatrix = glm::lookAt(glm::vec3(ballX, ballY + 0.f, 0.75f), glm::vec3(ballX, ballY + 0.2f, 1.5f), glm::vec3(0, 1, 0));
	}

	if (alive) {
		UpdateTicking(deltaTimeSeconds);
		
		GenerateTiles();

		Jump(deltaTimeSeconds);

		Collisions();
	}
}

void Skyroads::FrameEnd()
{
}

void Skyroads::Jump(float deltaTimeSeconds) {
	if (ballY > 0.1 && !jumping) {
		ballY = std::max(0.1f, ballY - ballSpeedY * deltaTimeSeconds);
	}
	if (jumping) {
		ballY += ballSpeedY * deltaTimeSeconds;
		if (ballY > 0.35f) {
			jumping = false;
		}
	}
}

void Skyroads::GenerateTiles() {
	if (newTilesTimer <= 0.f) {
		int random = rand() % (1 << tileRows);
		bool needFuel = (fuel <= MAXFUEL / 2);
		for (int i = 0; i < tileRows; ++i) {
			if (random & (1 << i)) {
				glm::vec3 color;
				if (tiles[i].empty()) {
					for (int j = 0; j < 3; ++j) {
						if (needFuel && random % 3 == j)
							color = GREEN;
						else
							color = randomColor();
						tiles[i].emplace_back(20 + tileLength * j, color);
					}
				}
				else {
					float start = tiles[i].back().first + 1 + tileLength * (rand() % 4);
					for (int j = 0; j < 3; ++j) {
						if (needFuel && random % 3 == j)
							color = GREEN;
						else
							color = randomColor();
						tiles[i].emplace_back(start + tileLength * j, color);
					}
				}
			}
		}
		newTilesTimer = 0.25f;
	}
}

void Skyroads::Collisions() {
	if (ballY == 0.1f) {
		if (ballX < (tileWidth + 0.15f) * (-tileRows / 2) - tileWidth / 2 ||
			ballX >(tileWidth + 0.15f) * (tileRows - 1 - tileRows / 2) + tileWidth / 2)
			alive = false;
		else {
			for (int i = 0; i < tileRows - 1; ++i) {
				if (ballX > (tileWidth + 0.15f) * (i - tileRows / 2) + tileWidth / 2  + 0.05f &&
					ballX < (tileWidth + 0.15f) * (i + 1 - tileRows / 2) - tileWidth / 2 - 0.05f) {
					alive = false;
					break;
				}
			}
			if (alive) {
				bool grounded = false;
				for (int i = 0; i < tileRows; ++i) {
					for (int j = 0; j <= std::min(1, (int)tiles[i].size() - 1); ++j) {
						if ((tiles[i][j].first > 0.45f - tileLength / 2 && tiles[i][j].first < 0.55f + tileLength / 2) &&
							ballX < (tileWidth + 0.15f) * (i - tileRows / 2) + tileWidth / 2 + 0.05 &&
							ballX >(tileWidth + 0.15f) * (i - tileRows / 2) - tileWidth / 2 - 0.05) {
							grounded = true;
							if (tiles[i][j].second == PURPLE)
								continue;
							if (tiles[i][j].second == RED) {
								alive = false;
							}
							if (tiles[i][j].second == ORANGE) {
								turbo = 5.f;
								oldSpeed = tileSpeed;
								tileSpeed = maxSpeed;
							}
							if (tiles[i][j].second == GREEN) {
								fuel = std::min(MAXFUEL, fuel + MAXFUEL * 0.4f);
							}
							if (tiles[i][j].second == YELLOW) {
								fuel -= MAXFUEL * 0.2f;
							}
							if (tiles[i][j].second == PINK) {
								refuels = max(0, refuels - 1);
							}
							if (tiles[i][j].second == CYAN) {
								refuels = min(2, refuels + 1);
							}
							tiles[i][j].second = PURPLE;
							++score;
						}
					}
				}
				if (!grounded)
					alive = false;
			}
		}
	}
}

void Skyroads::UpdateTicking(float deltaTimeSeconds) {
	if (fuel <= 0.f) {
		--refuels;
		alive = (refuels >= 0);
		fuel = MAXFUEL * 2 / 3;
	}
	turbo = std::max(0.f, turbo - deltaTimeSeconds);
	if (turbo == 0.f) {
		tileSpeed = oldSpeed;
	}
	fuel -= deltaTimeSeconds * tileSpeed / maxSpeed;
	newTilesTimer -= deltaTimeSeconds;
}

void Skyroads::ScoreCheck() {
	if (score > SCORESET) {
		minSpeed += 0.5f;
		maxSpeed += 0.5f;
		oldSpeed += 0.5f;
		score = 0;
	}
}

void Skyroads::RenderTiles(float deltaTimeSeconds) {
	for (int i = 0; i < tileRows; ++i) {
		if (!tiles[i].empty()) {
			auto it = tiles[i].begin();
			while (it != tiles[i].end()) {
				RenderSimpleMesh(meshes["box"], shaders["Skyroads"],
					glm::translate(glm::mat4(1), glm::vec3((tileWidth + 0.15f) * (i - tileRows / 2), -tileHeight / 2, it->first)) * tileMatrix, it->second);
				if (alive)
					it->first -= deltaTimeSeconds * tileSpeed;
				++it;
			}
			if (tiles[i].begin()->first < -tileLength / 2)
				tiles[i].erase(tiles[i].begin());
		}
	}
}

void Skyroads::RenderFuel() {
	float perc = fuel / MAXFUEL;
	glm::mat4 fuelMatrix = glm::scale(glm::translate(glm::mat4(1), glm::vec3(-1.2f + 0.5f * (perc - 1), 1.7, 0)), glm::vec3(perc, 0.2f, 1));
	glm::mat4 fuelMax = glm::scale(glm::translate(glm::mat4(1), glm::vec3(-1.2f, 1.7, 0)), glm::vec3(1, 0.2f, 1));
	RenderOrtho(meshes["box"], shaders["Skyroads"], fuelMatrix, glm::vec3(1.f, 0.5f, 0.3f));
	RenderOrtho(meshes["box"], shaders["Skyroads"], fuelMax, glm::vec3(0.1f, 0.12f, 0.1f));
	for (int i = 0; i < refuels; ++i) {
		RenderOrtho(meshes["box"], shaders["Skyroads"], glm::translate(fuelMax, glm::vec3(0.f, -(i + 1) * 1.5f, 0.f)), glm::vec3(1.f, 0.5f, 0.3f));
	}
}

void Skyroads::DeathComputations(float deltaTimeSeconds) {
	waitDead -= deltaTimeSeconds;
	if (waitDead < 0.f)
		if (ballY > -0.9f) {
			ballY -= deltaTimeSeconds;
			cameraY -= 1.f * deltaTimeSeconds;
			if (third)
				viewMatrix = glm::lookAt(glm::vec3(0, cameraY, -0.27f), glm::vec3(0, cameraY - 0.1, 1), glm::vec3(0, 1, 0));
		}
		else {
			ballColor.y = ballColor.y - deltaTimeSeconds / 1.5f;
			ballColor.z = ballColor.z - deltaTimeSeconds / 1.5f;
		}
	if (ballColor.y < -0.5f)
		window->Close();
}

glm::vec3 Skyroads::randomColor() {
	int rnd = rand() % 1300;
	if (rnd < 50)
		return GREEN;
	if (rnd < 200)
		return RED;
	if (rnd < 250)
		return YELLOW;
	if (rnd < 325)
		return ORANGE;
	if (rnd < 400)
		return PINK;
	if (rnd < 475)
		return CYAN;
	return BLUE;
}

void Skyroads::RenderBackTexture() {
	Shader* shader = shaders["Simple"];
	glm::mat4 modelMatrix = glm::translate(glm::mat4(1), glm::vec3(0, -1, 14)) * glm::scale(glm::mat4(1), glm::vec3(2.5, 0.5f, 1.5));

	shader->Use();
	glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

	meshes["wall"]->Render();
}

void Skyroads::RenderOrtho(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, const glm::vec3& color) {
	if (!mesh || !shader || !shader->GetProgramID())
		return;

	// render an object using the specified shader and the specified position
	glUseProgram(shader->program);

	int location = glGetUniformLocation(shader->program, "object_color");
	glUniform3fv(location, 1, glm::value_ptr(color));

	location = glGetUniformLocation(shader->program, "autistic");
	glUniform1f(location, 0.f);

	// Bind model matrix
	GLint loc_model_matrix = glGetUniformLocation(shader->program, "Model");
	glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

	// Bind view matrix
	//glm::mat4 viewMatrix = GetSceneCamera()->GetViewMatrix();
	int loc_view_matrix = glGetUniformLocation(shader->program, "View");
	glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(orthoViewMatrix));

	// Bind projection matrix
	//glm::mat4 projectionMatrix = GetSceneCamera()->GetProjectionMatrix();
	int loc_projection_matrix = glGetUniformLocation(shader->program, "Projection");
	glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(orthoProjectionMatrix));

	// Draw the object
	glBindVertexArray(mesh->GetBuffers()->VAO);
	glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_SHORT, 0);
}


void Skyroads::RenderSimpleMesh(Mesh *mesh, Shader *shader, const glm::mat4 & modelMatrix, const glm::vec3 &color, float autistic)
{
	if (!mesh || !shader || !shader->GetProgramID())
		return;

	// render an object using the specified shader and the specified position
	glUseProgram(shader->program);

	int location = glGetUniformLocation(shader->program, "object_color");
	glUniform3fv(location, 1, glm::value_ptr(color));

	location = glGetUniformLocation(shader->program, "autistic");
	glUniform1f(location, autistic);

	// Bind model matrix
	GLint loc_model_matrix = glGetUniformLocation(shader->program, "Model");
	glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
	
	// Bind view matrix
	//glm::mat4 viewMatrix = GetSceneCamera()->GetViewMatrix();
	int loc_view_matrix = glGetUniformLocation(shader->program, "View");
	glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));

	// Bind projection matrix
	//glm::mat4 projectionMatrix = GetSceneCamera()->GetProjectionMatrix();
	int loc_projection_matrix = glGetUniformLocation(shader->program, "Projection");
	glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

	// Draw the object
	glBindVertexArray(mesh->GetBuffers()->VAO);
	glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_SHORT, 0);
}


void Skyroads::OnInputUpdate(float deltaTime, int mods)
{
	if (alive) {
		if (window->KeyHold(GLFW_KEY_W) && turbo == 0.f) {
			oldSpeed = std::min(maxSpeed, tileSpeed + deltaTime * 6);
			//cout << tileSpeed << '\n';
		}

		if (window->KeyHold(GLFW_KEY_S) && turbo == 0.f) {
			oldSpeed = std::max(minSpeed, tileSpeed - deltaTime * 7);
		}

		if (window->KeyHold(GLFW_KEY_A)) {
			ballX += ballSpeedX * deltaTime;
		}

		if (window->KeyHold(GLFW_KEY_D)) {
			ballX -= ballSpeedX * deltaTime;
		}
	}
}

void Skyroads::OnKeyPress(int key, int mods)
{
	// add key press event
	if (key == GLFW_KEY_SPACE && ballY == 0.1f) {
		jumping = true;
	}

	if (key == GLFW_KEY_C) {
		if (third) {
			third = false;
		}
		else {
			third = true;
			viewMatrix = glm::lookAt(glm::vec3(0, cameraY, -0.27f), glm::vec3(0, cameraY - 0.1, 1), glm::vec3(0, 1, 0));
		}
	}
}

void Skyroads::OnKeyRelease(int key, int mods)
{
	// add key release event
	if (key == GLFW_KEY_SPACE) {
		jumping = false;
	}
}

void Skyroads::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
	// add mouse move event
}

void Skyroads::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
	// add mouse button press event
}

void Skyroads::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
	// add mouse button release event
}

void Skyroads::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}

void Skyroads::OnWindowResize(int width, int height)
{
}
