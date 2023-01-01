#pragma once

namespace Nork::Components
{
	struct Camera
	{
		enum struct Direction
		{
			None = 0,
			Up,
			Down,
			Left,
			Rigth,
			Forward,
			Backward,
		};

		Camera();
		Camera(float fieldOfViewDegrees, float aspectRatio, float nearClip, float farClip);
		void Move(Direction direction, float delta);
		void Zoom(float delta);
		void Rotate(float deltaPitch, float deltaYaw);
		void SetRotation(float pitch, float yaw);
		void SetPosition(glm::vec3 pos);
		void Update();
	public:
		float FOV, ratio, nearClip, farClip;
		float pitch = 0.0f, yaw = -90.0f, moveSpeed = 0.01f, zoomSpeed = 0.1f, rotationSpeed = 0.1f;
		glm::vec3 position, front, up, right;
		glm::mat4 view, projection, viewProjection;
	private:
		void UpdateFrontRight();
		void UpdateView();
		void UpdateProjection();
		void UpdateViewProjection();
	};
}