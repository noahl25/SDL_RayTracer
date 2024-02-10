#pragma once

#include <glm.hpp>
#include <vector>
#include <SDL.h>
#include <iostream>

class Camera
{
public:
	Camera() = default;
	Camera(SDL_Window* window, float verticalFOV, float nearClip, float farClip);

	bool OnUpdate(float ts);
	void OnResize(uint32_t width, uint32_t height);

	const glm::mat4& GetProjection() const { return m_Projection; }
	const glm::mat4& GetInverseProjection() const { return m_InverseProjection; }
	const glm::mat4& GetView() const { return m_View; }
	const glm::mat4& GetInverseView() const { return m_InverseView; }

	const glm::vec3& GetPosition() const { return m_Position; }
	const glm::vec3& GetDirection() const { return m_ForwardDirection; }

	const std::vector<glm::vec3>& GetRayDirections() const { return m_RayDirections; }

	float GetRotationSpeed();

	enum Keys {
		KEY_W = 0,
		KEY_S = 1,
		KEY_A = 2,
		KEY_D = 3,
		KEY_Q = 4,
		KEY_E = 5
	};

	inline void KeyChanged(Keys key, bool state) { m_Keys[key] = state;  }

private:
	void RecalculateProjection();
	void RecalculateView();
	void RecalculateRayDirections();
	inline bool RightMousePressed() {

		return SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(3);

	}
private:
	glm::mat4 m_Projection{ 1.0f };
	glm::mat4 m_View{ 1.0f };
	glm::mat4 m_InverseProjection{ 1.0f };
	glm::mat4 m_InverseView{ 1.0f };

	float m_VerticalFOV = 45.0f;
	float m_NearClip = 0.1f;
	float m_FarClip = 100.0f;

	glm::vec3 m_Position{ 0.0f, 0.0f, 0.0f };
	glm::vec3 m_ForwardDirection{ 0.0f, 0.0f, 0.0f };

	// Cached ray directions
	std::vector<glm::vec3> m_RayDirections;

	glm::vec2 m_LastMousePosition{ 0.0f, 0.0f };

	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

	SDL_Window* m_Window;

	bool m_Keys[6] = { false };

	const Uint8* keys;
 

};