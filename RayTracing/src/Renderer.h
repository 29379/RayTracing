#pragma once
#include "Walnut/Image.h"
#include "Walnut/Random.h" 
#include <memory>



// input is the scene description of the 3D world that the engine
// will be trying to render, the output is an image holding the produced pixels
class Renderer
{
public:
	Renderer() = default; // for now
	void OnResize(uint32_t width, uint32_t height);
	void Render();
	std::shared_ptr<Walnut::Image> GetFinalImage() const { return finalImage; }
private:
	/*	i may have more than one image at the same time in 
		the pipeline, so it will be clear that it is the final buffer*/
	std::shared_ptr<Walnut::Image> finalImage;
	uint32_t* imageData = nullptr;	// buffer of pixel data
};
