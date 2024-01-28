#pragma once

#include <glm/glm.hpp>
#include <vector>

struct Material {
	glm::vec3 albedo{ 1.0f };
	float roughness = 1.0f; //	not smooth, not reflective
	float metallic = 0.0f;

	//	for emissive materials (light sources):
	glm::vec3 emissionColor{ 0.0f };
	float emissionPower = 0.0f;	

	glm::vec3 GetEmission() const { return emissionColor * emissionPower; }
};

struct Sphere {
	glm::vec3 position{ 0.0f };
	float radius = 0.5f;
	int materialIndex;
};

struct Scene {
	std::vector<Sphere> objects;
	std::vector<Material> materials;
};

/*	physically based rendering - a way to standardize parameters to
	represent different material behaviors, currently a standard approach
*/

// microfacet - term used to help to define roughness