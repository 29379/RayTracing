#include "Renderer.h"


void Renderer::OnResize(uint32_t width, uint32_t height) {
	if (finalImage) {
		//	no resize necessary
		if (finalImage->GetWidth() == width && finalImage->GetHeight() == height)
			return;

		/*	Resize checks if the image needs resizing
		if not - returns with no effect
		if yes - changes the width and height values
		and then releases and reallocates the memory
		(instead of deleting the image and creating a new one,
		because the pointer and the object dont change,
		the spot in memory stays, i change only the inside)	*/
		finalImage->Resize(width, height);
	}
	else {
		finalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}
	//	delete nullptr is valid, it will do it internally for me
	delete[] imageData;
	imageData = new uint32_t[width * height];
}


void Renderer::Render() {
	// rendering the pixels
	for (uint32_t i = 0; i < finalImage->GetWidth() * finalImage->GetHeight(); i++) {
		imageData[i] = Walnut::Random::UInt();
		/*	i dont want the alpha channel to be random, to
		be sure to always 'see' stuff, so i set
		the most significant bytes to 265	*/
		imageData[i] |= 0xff000000;
	}

	// uploading pixel data to the GPU
	finalImage->SetData(imageData);
}



