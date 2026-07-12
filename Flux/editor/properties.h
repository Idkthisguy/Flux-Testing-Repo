#pragma once
#include "imgui.h"

#include <variant>
#include <vector>
#include <cstring>
#include <string>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Flux {
	struct TransformComponent {
		glm::vec3 position;
		glm::vec3 rotation;
		glm::vec3 scale;
	};

	struct CameraComponent {
		float fov;
		bool  isMainCamera;
	};

	struct MeshComponent {
		std::string modelPath;
		float roughness;
		float metallic;
	};

	struct PhysicsComponent {
		glm::vec3 velocity;
		bool isAnchored;
	};

	struct Component {
		std::string name;
		std::variant<TransformComponent, CameraComponent, MeshComponent, PhysicsComponent> data;
	};

	class Heiarchy;

	class Properties {
	public:
		void renderProperties(Heiarchy* h = nullptr);
	};
}