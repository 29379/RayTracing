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
	/*	if its on - we're accumulating samples, if it's off -
		it will keep bouncing rays every frame without the path tracing part,
		constructing the scene from scratch every time	*/
	struct Settings {
		bool accumulate = true;
		bool slowRandom = true;
	};
public:
	Renderer() = default; // for now
	void OnResize(uint32_t width, uint32_t height);
	void Render(const Scene& scene, const Camera& camera);
	std::shared_ptr<Walnut::Image> GetFinalImage() const { return finalImage; }
	void ResetFrameIndex() { frameIndex = 1; }
	Settings& GetSettings() { return settings; }
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

	/*	if the ray in TraceRay hits something, ClosestHit shader is called and determines
		the worldPosition and worldNormal parameters	*/
	HitPayload ClosestHit(const Ray& ray, float hitDistance, int objectIndex);	
	HitPayload Miss(const Ray& ray);	//	if the ray in TraceRay misses everythin, this gets called

private:
	/*	i may have more than one image at the same time in 
		the pipeline, so it will be clear that it is the final buffer*/
	std::shared_ptr<Walnut::Image> finalImage;
	uint32_t* imageData = nullptr;	// buffer of pixel data
	
	/*	buffer of accumulated data, related to path tracing, which will allow to store one,
		definitive image, instead of rendering new random ray bounces every frame, because it
		causes a lot of noise when you zoom in - vec4 is 4 floats, so 32 bits per channel, 
		so i'll basically be able to have HDR, since it is a lot of data.*/
	glm::vec4* accumulationData = nullptr;	

	/*	moving the camera resets everything - when camera stands still the image will keep
		tracing paths, but this require the knowledge what frame are we on since we
		started accumulating data - 1 by default instead of 0, because it will be used
		to average the paths out with it as the number of evaluated paths, so it's a divider	*/
	uint32_t frameIndex = 1;

	Settings settings;

	const Scene* activeScene = nullptr;
	const Camera* activeCamera = nullptr;

	/*	i'll use these to allow running a parallel multi-threaded std::for_each loop
		while rendering the image, because now i will have an iterator from a vector	*/
	std::vector<uint32_t> horizontalIter;
	std::vector<uint32_t> verticalIter;
};
