#pragma once

#include "Camera.h"
#include "Ray.h"
#include "Scene.h"

#include <SDL.h>
#include <glm.hpp>

class RayTracer {

private:

	SDL_Window* m_Window;
	SDL_Renderer* m_Renderer;

	SDL_Event m_Event;

	uint32_t* m_Data;

	bool m_Running;
	int m_Width, m_Height;

	bool m_Another;
	bool m_RealTime;

	Camera m_Camera;
	Scene m_Scene;

	const glm::vec3 m_LightDir;

public:

	struct Settings {

		bool Accumulate = true;

	};

public:

	RayTracer(const char* title, unsigned width, unsigned height, bool fullScreen = false);

	void Run();
	void ResetFrameIndex() { m_FrameIndex = 1; }
	Settings& GetSettings() { return m_Settings; }

private:

	struct HitPayload {
		float HitDistance;
		glm::vec3 WorldPosition;
		glm::vec3 WorldNormal;

		int ObjectIndex;
	};

	void HandleEvents();
	void Update();

	glm::vec4 PerPixel(int x, int y);

	uint32_t ToRGBA(const glm::vec4& color);

	HitPayload TraceRay(const Ray& ray);
	HitPayload ClosestHit(const Ray& ray, float hitDistance, int objectIndex);
	HitPayload Miss(const Ray& ray);

	glm::vec4* m_AccumulationData = nullptr;
	uint32_t m_FrameIndex = 1;

	Settings m_Settings;

	std::vector<uint32_t> m_ImageHorizontalIter, m_ImageVerticalIter;
 

};