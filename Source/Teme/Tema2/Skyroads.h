#pragma once
#include <Component/SimpleScene.h>
#include <Component/Transform/Transform.h>
#include <Core/GPU/Mesh.h>

#define RED glm::vec3(1.f, 0.f, 0.f)
#define BLUE glm::vec3(0.f, 0.f, 1.f)
#define PURPLE glm::vec3(0.5f, 0.f, 0.5f)
#define YELLOW glm::vec3(1.f, 1.f, 0.f)
#define GREEN glm::vec3(0.f, 1.f, 0.f)
#define ORANGE glm::vec3(1.f, 0.5f, 0.f)
#define PINK glm::vec3(1.f, 0.09f, 0.55f)
#define CYAN glm::vec3(0.f, 1.f, 1.f)
#define MAXFUEL 10.f
#define SCORESET 15.f

class Skyroads : public SimpleScene
{
	public:
		Skyroads();
		~Skyroads();

		void Init() override;

	private:
		void FrameStart() override;
		void Update(float deltaTimeSeconds) override;
		void FrameEnd() override;

		void RenderSimpleMesh(Mesh *mesh, Shader *shader, const glm::mat4 &modelMatrix, const glm::vec3 &color = glm::vec3(1), float autistic = 0);
		glm::vec3 randomColor();
		void RenderBackTexture();
		void RenderOrtho(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, const glm::vec3& color = glm::vec3(1));
		void DeathComputations(float deltaTimeSeconds);
		void RenderFuel();
		void RenderTiles(float deltaTimeSeconds);
		void ScoreCheck();
		void UpdateTicking(float deltaTimeSeconds);
		void GenerateTiles();
		void Jump(float deltaTimeSeconds);
		void Collisions();

		void OnInputUpdate(float deltaTime, int mods) override;
		void OnKeyPress(int key, int mods) override;
		void OnKeyRelease(int key, int mods) override;
		void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
		void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
		void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
		void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
		void OnWindowResize(int width, int height) override;

		std::vector<std::vector<std::pair<float, glm::vec3>>> tiles;
		bool jumping, alive, third;
		int tileRows, refuels;
		glm::vec3 ballColor;
		float ballX, ballY, ballSpeedX, ballSpeedY;
		float tileWidth, tileHeight, tileLength, tileSpeed, minSpeed, maxSpeed, oldSpeed;
		float newTilesTimer;
		float turbo, fuel;
		float waitDead;
		float cameraY;
		int score;
		glm::mat4 tileMatrix, ballScale;
		glm::mat4 projectionMatrix, viewMatrix, orthoProjectionMatrix, orthoViewMatrix;
};
