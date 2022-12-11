#pragma once
#include "Walnut/Image.h"
#include "Walnut/Random.h" 

#include <glm/glm.hpp>
#include <memory>
#include <list>
#include <iostream>

#include "Camera.h"
#include "Ray.h"
#include "Scene.h"



// input is the scene description of the 3D world that the engine
// will be trying to render, the output is an image holding the produced pixels
class Renderer
{
public:
	Renderer() = default; // for now
	void OnResize(uint32_t width, uint32_t height);
	void Render(const Scene& scene, const Camera& camera, glm::vec3 lightSlider);
	std::shared_ptr<Walnut::Image> GetFinalImage() const { return finalImage; }
private:	

	struct HitPayload {
		float hitDistance;
		glm::vec3 worldPosition;
		glm::vec3 worldNormal;
		int objectIndex;
		//	Sphere* recentlyHitObject;	//	reference to a recently hit sphere
	};

	glm::vec4 PerPixel(uint32_t x, uint32_t y);	//	the color gets determined here on the basis of the return value of hitDistance in HitPayload from TraceRay
	HitPayload TraceRay(const Ray& ray);	//	does not return color  now

	/*	if the ray in TraceRay hits something, ClosestHit shader is calledand determines
		the worldPosition and worldNormal parameters	*/
	HitPayload ClosestHit(const Ray& ray, float hitDistance, int objectIndex);	
	HitPayload Miss(const Ray& ray);	//	if the ray in TraceRay misses everythin, this gets called

private:
	/*	i may have more than one image at the same time in 
		the pipeline, so it will be clear that it is the final buffer*/
	std::shared_ptr<Walnut::Image> finalImage;
	uint32_t* imageData = nullptr;	// buffer of pixel data

	const Scene* activeScene = nullptr;
	const Camera* activeCamera = nullptr;
};
