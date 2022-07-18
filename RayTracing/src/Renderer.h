#pragma once
#include "Walnut/Image.h"
#include "Walnut/Random.h" 

#include <glm/glm.hpp>
#include <memory>
#include <list>
#include <iostream>



// input is the scene description of the 3D world that the engine
// will be trying to render, the output is an image holding the produced pixels
class Renderer
{
public:
	Renderer() = default; // for now
	void OnResize(uint32_t width, uint32_t height);
	void Render();
	std::shared_ptr<Walnut::Image> GetFinalImage() const { return finalImage; }

	std::list<float>* hit_sphere(float radius, const glm::vec3& rayOrigin, const glm::vec3& rayDirection);
	unsigned long RGBAtoHEX(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);
private:
	uint32_t PerPixel(glm::vec2 coord);
private:
	/*	i may have more than one image at the same time in 
		the pipeline, so it will be clear that it is the final buffer*/
	std::shared_ptr<Walnut::Image> finalImage;
	uint32_t* imageData = nullptr;	// buffer of pixel data
};
