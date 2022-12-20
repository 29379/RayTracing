#pragma once

#include <glm/glm.hpp>
#include <vector>

class Camera
{
public:
	/*	vertical field of view is in degrees, nearand far clips specify
		what the camera can actually see (the shape is something along the lines
		of a pyramid with its top cut-off - SQUARE FRUSTUM) and it doesn't
		show stuff that happen to be out of bounds	*/
	Camera(float verticalFOV, float nearClip, float farClip);

	/*	called on every frame, enables the camera to move around at 
		a constant speed, independent of the frame rate	*/
	bool OnUpdate(float ts);
	void OnResize(uint32_t width, uint32_t height);

	const glm::mat4& GetProjection() const { return m_Projection; }
	const glm::mat4& GetInverseProjection() const { return m_InverseProjection; }
	const glm::mat4& GetView() const { return m_View; }
	const glm::mat4& GetInverseView() const { return m_InverseView; }
	
	const glm::vec3& GetPosition() const { return m_Position; }	//	position of the camera
	const glm::vec3& GetDirection() const { return m_ForwardDirection; }	//	direction of where the camera is pointing

	/*	the ray directions are cached in this design, because even though they
		have to be recalculated if the camera is moving, it will not be required
		if the camera is standing still which speeds up the rendering process	*/
	const std::vector<glm::vec3>& GetRayDirections() const { return m_RayDirections; }

	float GetRotationSpeed();
private:
	void RecalculateProjection();
	void RecalculateView();
	void RecalculateRayDirections();
private:
	glm::mat4 m_Projection{ 1.0f };
	glm::mat4 m_View{ 1.0f };
	glm::mat4 m_InverseProjection{ 1.0f };
	glm::mat4 m_InverseView{ 1.0f };

	float m_VerticalFOV = 45.0f;
	float m_NearClip = 0.1f;
	float m_FarClip = 100.0f;

	glm::vec3 m_Position{0.0f, 0.0f, 0.0f};
	glm::vec3 m_ForwardDirection{0.0f, 0.0f, 0.0f};

	// Cached ray directions
	std::vector<glm::vec3> m_RayDirections;

	glm::vec2 m_LastMousePosition{ 0.0f, 0.0f };

	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
};
