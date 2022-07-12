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
	/*
	for (uint32_t i = 0; i < finalImage->GetWidth() * finalImage->GetHeight(); i++) {
		imageData[i] = Walnut::Random::UInt();
		i dont want the alpha channel to be random, to
		be sure to always 'see' stuff, so i set
		the most significant bytes to 265	
		imageData[i] |= 0xff000000;
	}
	*/

	float aspectRatio = finalImage->GetWidth() / (float)finalImage->GetHeight();

	for (uint32_t y = 0; y < finalImage->GetHeight(); y++) {
		for (uint32_t x = 0; x < finalImage->GetWidth(); x++) {

			glm::vec2 coord = { x / (float)finalImage->GetWidth(), y / (float)finalImage->GetHeight() };
			coord = coord * 2.0f - 1.0f;	//	remapping the coordinates:	0 - 1 range	--->	-1 - 1 range
			coord.x *= aspectRatio;	//	makes sure the image wont get distorted when it gets resized
			/*	2D x and y coords are flattened as if the y coord is 
			multiplied by how big each row is, with adding the x column offset
			which gives me the equivalent of 'i' from a single for loop*/
			imageData[x + y * finalImage->GetWidth()] = PerPixel(coord);

		}
	}

	// uploading pixel data to the GPU
	finalImage->SetData(imageData);
}

/*	this method will form the basis of how i decide where
	to shoot my rays from the camera to see if they intersect
	with the objects in the scene	*/
uint32_t Renderer::PerPixel(glm::vec2 coord) {
	uint8_t redChannel = (uint8_t)abs((coord.x * 255.0f));
	uint8_t greenChannel = (uint8_t)abs((coord.y * 255.0f));
	
	glm::vec3 rayDirection(coord.x, coord.y, -1.0f);	//	-1 is just a convention for now
	glm::vec3 rayOrigin(0.0f, 0.0f, 2.0f);	//	camera position
	float radius = 0.5f;

	rayDirection = glm::normalize(rayDirection);;

	/*	sphere equation:
			-	t^2*(bx^2+by^2 + bz^2) + t(2(axbx + ayby + azbz)) + (ax^2 + ay^2 + az^2 - r^2) = 0	
		a	-	ray origin
		b	-	ray direction
		r	-	radius of the sphere
		t	-	intersection distances
		quadratic equation:
								-	dx^2 + ex + f = 0
		t^2 d coeff				-	tSqCoeff
		t e coeff				-	tCoeff
		f constant				-	volatilePart
		delta (discriminant)	-	e^2 - 4*d*f

		intersectios equation:
			-	(-e +- sqrt(delta)) / (2.0f * d)
	*/

	//	glm::dot(x, y)	-	'iloczyn skalarny' x * y
	//	float tSqCoeff = rayDirection.x*rayDirection.x + rayDirection.y*rayDirection.y + rayDirection.z*rayDirection.z;	//	does the same thing as the line below
	float tSqCoeff = glm::dot(rayDirection, rayDirection);	
	float tCoeff = 2.0f * glm::dot(rayOrigin, rayDirection);
	float volatilePart = glm::dot(rayOrigin, rayOrigin) - radius * radius;

	float delta = tCoeff * tCoeff - 4.0f * tSqCoeff * volatilePart;
	if (delta >= 0.0f) {
		float intersection1 = (-tCoeff - sqrt(delta)) / (2.0f * tSqCoeff);
		float intersection2 = (-tCoeff + sqrt(delta)) / (2.0f * tSqCoeff);

		auto a = (rayOrigin + rayDirection * intersection1) - glm::vec3(0, 0, -1);
		auto b = (rayOrigin + rayDirection * intersection2) - glm::vec3(0, 0, -1);
		
		return 0xff000000 | (uint8_t)(255.0f * a.y) | (uint8_t)(255.0f * a.x) | (uint8_t)(255.0f * a.z);
		//return 0xff000000 | (greenChannel << 8) | redChannel;
	}
	return 0xff000000;
}

