#include "RayTracer.h"
#include "Ray.h"
#include "Random.h"

#include <stdexcept>
#include <iostream>
#include <execution>
 
RayTracer::RayTracer(const char* title, unsigned width, unsigned height, bool fullScreen) 
	: m_Running(false), m_Another(false), m_RealTime(true), m_Height(height), m_Width(width), m_Data(new uint32_t[height * width]), m_Window(nullptr), m_Renderer(nullptr), m_LightDir( glm::vec3(-1, -1, -1)), m_AccumulationData(new glm::vec4[height * width])
{
	if (SDL_Init(SDL_INIT_EVERYTHING) == 0) {

		std::cout << "SDL initialized." << "\n";

		if (m_Window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, (fullScreen ? SDL_WINDOW_FULLSCREEN : 0)))
			std::cout << "Window initialized." << "\n";
		if (m_Renderer = SDL_CreateRenderer(m_Window, -1, SDL_RENDERER_ACCELERATED))
			std::cout << "Renderer initialized." << "\n";

		m_Running = true;
		m_Camera = Camera{ m_Window, 45.0f, 0.1f, 100.0f };

		Material& sphere1 = m_Scene.Materials.emplace_back();
		sphere1.Albedo = { 1.0f, 0.0f, 1.0f };
		sphere1.Roughness = 1.0f;

		Material& sphere2 = m_Scene.Materials.emplace_back();
		sphere2.Albedo = { 0.2f, 0.3f, 1.0f };
		sphere2.Roughness = 0.1f;

		{
			Sphere sphere;
			sphere.Position = { 0.0f, 0.0f, 0.0f };
			sphere.Radius = 1.0f;
			sphere.MaterialIndex = 0;
			m_Scene.Spheres.push_back(sphere);
		}
		{
			Sphere sphere;
			sphere.Position = { 0.0f, 101.0f, 0.0f };
			sphere.Radius = 100.0f;
			sphere.MaterialIndex = 1;
			m_Scene.Spheres.push_back(sphere);
		}

		m_ImageHorizontalIter.resize(width);
		m_ImageVerticalIter.resize(height);

		for (uint32_t i = 0; i < width; i++) m_ImageHorizontalIter[i] = i;
		for (uint32_t i = 0; i < height; i++) m_ImageVerticalIter[i] = i;
	}

}

void RayTracer::Run()
{

	const int FPSTarget = 60;
	const int deltaTimeTarget = 1000 / FPSTarget;

	unsigned frameStart;
	unsigned frameTime;

	unsigned prevTime = SDL_GetTicks();

	while (m_Running) {

		frameStart = SDL_GetTicks();
		this->HandleEvents(); 
		this->Update();

		frameTime = SDL_GetTicks() - frameStart;

		if (deltaTimeTarget > frameTime)
			SDL_Delay(deltaTimeTarget - frameTime);

	}


}
 
void RayTracer::HandleEvents()
{
 
	SDL_PollEvent(&m_Event);

	switch (m_Event.type) {

	case SDL_QUIT:

		m_Running = false;
		break;

	case SDL_KEYDOWN:

		switch (m_Event.key.keysym.sym) {

		case SDLK_w:
			m_Camera.KeyChanged(Camera::KEY_W, true);
			break;

		case SDLK_d:

			m_Camera.KeyChanged(Camera::KEY_D, true);
			break;

		case SDLK_s:

			m_Camera.KeyChanged(Camera::KEY_S, true);
			break;

		case SDLK_a:

			m_Camera.KeyChanged(Camera::KEY_A, true);
			break;

		case SDLK_q:

			m_Camera.KeyChanged(Camera::KEY_Q, true);
			break;

		case SDLK_e:

			m_Camera.KeyChanged(Camera::KEY_E, true);
			break;

		case SDLK_r:

			ResetFrameIndex();

		}

		break;

	case SDL_KEYUP:


		switch (m_Event.key.keysym.sym) {

		case SDLK_w:
			m_Camera.KeyChanged(Camera::KEY_W, false);
			break;

		case SDLK_d:

			m_Camera.KeyChanged(Camera::KEY_D, false);
			break;

		case SDLK_s:

			m_Camera.KeyChanged(Camera::KEY_S, false);
			break;

		case SDLK_a:

			m_Camera.KeyChanged(Camera::KEY_A, false);
			break;

		case SDLK_q:

			m_Camera.KeyChanged(Camera::KEY_Q, false);
			break;

		case SDLK_e:

			m_Camera.KeyChanged(Camera::KEY_E, false);
			break;

		}

		break;
 


	}


}

inline uint32_t RayTracer::ToRGBA(const glm::vec4& color)
{
	return (((uint8_t)(color.r * 255.0f)) << 24) | (((uint8_t)(color.g * 255.0f)) << 16) | (((uint8_t)(color.b * 255.0f)) << 8) | (uint8_t)(color.a * 255.0f);
}

static constexpr bool timeTrace = false;

void RayTracer::Update() {

	//SDL_RenderClear(m_Renderer);
	//SDL_SetRenderDrawColor(m_Renderer, 255, 0, 0, 255);
	//SDL_RenderDrawPoint(m_Renderer, 0, m_Height - 1);
	//SDL_SetRenderDrawColor(m_Renderer, 0, 0, 0, 255);
	//SDL_RenderPresent(m_Renderer);

	int start;

	static SDL_Surface* surface = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
	static uint32_t* pixels = static_cast<uint32_t*>(surface->pixels);

	if (m_Camera.OnUpdate(0.016))
		ResetFrameIndex();
	
	m_Camera.OnResize(m_Width, m_Height);

	if (m_Another || m_RealTime) {
		if (timeTrace)
			start = SDL_GetTicks();

	 
 

		if (m_FrameIndex == 1)
			memset(m_AccumulationData, 0, m_Width * m_Height * sizeof(glm::vec4));

#define MT 1

#if MT

		std::for_each(std::execution::par, m_ImageVerticalIter.begin(), m_ImageVerticalIter.end(), [this](uint32_t y) {

			std::for_each(std::execution::par, m_ImageHorizontalIter.begin(), m_ImageHorizontalIter.end(), [this, y](uint32_t x) {

				glm::vec4 color = PerPixel(x, y);

				m_AccumulationData[y * m_Width + x] += color;

				glm::vec4 accumulatedColor = m_AccumulationData[y * m_Width + x];
				accumulatedColor /= (float)m_FrameIndex;

				accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));
				pixels[y * m_Width + x] = ToRGBA(accumulatedColor);

			});

		});
#else
		
		for (int y = 0; y < m_Height; y++) {

			for (int x = 0; x < m_Width; x++) {

				glm::vec4 color = PerPixel(x, y);

				m_AccumulationData[y * m_Width + x] += color;

				glm::vec4 accumulatedColor = m_AccumulationData[y * m_Width + x];
				accumulatedColor /= (float)m_FrameIndex;

				accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));
				pixels[y * m_Width + x] = ToRGBA(accumulatedColor);

			}

		}
#endif
 
 

		SDL_Texture* texture = SDL_CreateTextureFromSurface(m_Renderer, surface);
		SDL_RenderCopy(m_Renderer, texture, NULL, NULL);

		SDL_RenderPresent(m_Renderer);

		SDL_DestroyTexture(texture);
 
		m_Another = false;

		if (timeTrace)
			std::cout << (SDL_GetTicks() - start) << "\n";

		if (m_Settings.Accumulate) {
			m_FrameIndex++;
		}
		else {
			m_FrameIndex = 1;
		}
	}
}

 
glm::vec4 RayTracer::PerPixel(int x, int y)
{
	Ray ray;
	ray.Origin = m_Camera.GetPosition();
	ray.Direction = m_Camera.GetRayDirections()[x + y * m_Width];

	glm::vec3 color(0.0f);
	float multiplier = 1.0f;

	int bounces = 5;
	for (int i = 0; i < bounces; i++)
	{
		RayTracer::HitPayload payload = TraceRay(ray);
		if (payload.HitDistance < 0.0f)
		{
			glm::vec3 skyColor = glm::vec3(0.6f, 0.7f, 0.9f);
			color += skyColor * multiplier;
			break;
		}

		glm::vec3 lightDir = glm::normalize(glm::vec3(-1, 1, -1));
		float lightIntensity = glm::max(glm::dot(payload.WorldNormal, -lightDir), 0.0f); // == cos(angle)

		const Sphere& sphere = m_Scene.Spheres[payload.ObjectIndex];
		const Material& material = m_Scene.Materials[sphere.MaterialIndex];

		glm::vec3 sphereColor = material.Albedo;
		sphereColor *= lightIntensity;
		color += sphereColor * multiplier;

		multiplier *= 0.5f;

		ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.0001f;
		ray.Direction = glm::reflect(ray.Direction,
			payload.WorldNormal + material.Roughness * Random::Vec3(-0.5f, 0.5f));
	}

	return glm::vec4(color, 1.0f);
}


RayTracer::HitPayload RayTracer::ClosestHit(const Ray& ray, float hitDistance, int objectIndex)
{
	RayTracer::HitPayload payload;
	payload.HitDistance = hitDistance;
	payload.ObjectIndex = objectIndex;

	const Sphere& closestSphere = m_Scene.Spheres[objectIndex];

	glm::vec3 origin = ray.Origin - closestSphere.Position;
	payload.WorldPosition = origin + ray.Direction * hitDistance;
	payload.WorldNormal = glm::normalize(payload.WorldPosition);

	payload.WorldPosition += closestSphere.Position;

	return payload;

	
}

RayTracer::HitPayload RayTracer::Miss(const Ray& ray)
{
	RayTracer::HitPayload payload;
	payload.HitDistance = -1.0f;
	return payload;
}


RayTracer::HitPayload RayTracer::TraceRay(const Ray& ray)
{
 

	int closestSphere = -1;
	float hitDistance = std::numeric_limits<float>::max();
	for (size_t i = 0; i < m_Scene.Spheres.size(); i++)
	{
		const Sphere& sphere = m_Scene.Spheres[i];
		glm::vec3 origin = ray.Origin - sphere.Position;

		float a = glm::dot(ray.Direction, ray.Direction);
		float b = 2.0f * glm::dot(origin, ray.Direction);
		float c = glm::dot(origin, origin) - sphere.Radius * sphere.Radius;
 
		float discriminant = b * b - 4.0f * a * c;
		if (discriminant < 0.0f)
			continue;

 

		// float farT = (-b + glm::sqrt(discriminant)) / (2.0f * a);  
		float closestT = (-b - glm::sqrt(discriminant)) / (2.0f * a);
		if (closestT > 0.0f && closestT < hitDistance)
		{
			hitDistance = closestT;
			closestSphere = (int)i;
		}
	}

	if (closestSphere < 0)
		return Miss(ray);

	return ClosestHit(ray, hitDistance, closestSphere);
}

