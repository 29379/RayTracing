#include "Renderer.h"


namespace Utils {

	static uint32_t Vec4ToRGBA(const glm::vec4& color) {
		uint8_t r = (color.r * 255.0f);
		uint8_t g = (color.g * 255.0f);
		uint8_t b = (color.b * 255.0f);
		uint8_t a = (color.a * 255.0f);

		uint32_t output = (a << 24) | (b << 16) | (g << 8) | r;
		return output;
	}

}


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


void Renderer::Render(glm::vec4 colorsSlider, glm::vec3 cameraSlider, glm::vec3 lightSlider) {
	// rendering the pixels
	/*
	for (uint32_t i = 0; i < finalImage->GetWidth() * finalImage->GetHeight(); i++) {
		imageData[i] = Walnut::Random::UInt();
		i dont want the alpha channel to be random, to
		be sure to always 'see' stuff, so i set
		the most significant bytes to 255	
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
			glm::vec4 color = PerPixel(coord, colorsSlider, cameraSlider, lightSlider);
			color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
			imageData[x + y * finalImage->GetWidth()] = Utils::Vec4ToRGBA(color);

		}
	}

	// uploading pixel data to the GPU
	finalImage->SetData(imageData);
}


/*	this method will form the basis of how i decide where
	to shoot my rays from the camera to see if they intersect
	with the objects in the scene	*/
glm::vec4 Renderer::PerPixel(glm::vec2 coord, glm::vec4 colorsSlider, glm::vec3 cameraSlider, glm::vec3 lightSlider) {
	glm::vec3 rayDirection(coord.x, coord.y, -1.0f);	//	-1 is just a convention for now
	glm::vec3 rayOrigin = cameraSlider;	//	camera position
	float radius = 0.5f;

	rayDirection = glm::normalize(rayDirection);	//	creates a unit vector

	float a = glm::dot(rayDirection, rayDirection);
	float b = 2.0f * glm::dot(rayOrigin, rayDirection);
	float c = glm::dot(rayOrigin, rayOrigin) - radius * radius;
	float delta = b * b - 4.0f * a * c;

	if (delta < 0) {
		return glm::vec4(0, 0, 0, 1);
	}

	float entryPointDistance = (-b - glm::sqrt(delta)) / (2.0f * a);
	float exitPointDistance = (-b + glm::sqrt(delta)) / (2.0f * a);

	//	return glm::vec4(1, 0.3, 0.7, 1);
	
	glm::vec3 entryPoint = rayOrigin + entryPointDistance * rayDirection;
	glm::vec3 exitPoint = rayOrigin + exitPointDistance * rayDirection;

	/*	normal vector; typically it is the effect of normalizing(hitPoint - sphereOrigin),
	but the sphere origin here is a 0, so it is redundant	*/
	glm::vec3 normal = glm::normalize(entryPoint);

	glm::vec3 lightDirection = glm::normalize(lightSlider);
	float d = glm::max(glm::dot(normal, -lightDirection), 0.0f);		//	d == -cos(angle), cos(> 90) < 0

	//	values of the normal were in <-1; 1>, but the operations with 0.5f pushed it to <0; 1> bounds
	//	glm::vec3 sphereColor = normal * 0.5f + 0.5f;
	//glm::vec3 sphereColor(1, 1, 1);
	glm::vec4 sphereColor = colorsSlider;
	sphereColor *= d;

	return glm::vec4(sphereColor);
	/*
	//glm::vec3 ref(0, 0, -1);
	//glm::vec3 tmp(entryPoint - ref);
	//auto N = glm::normalize(tmp);

	
	//uint8_t alpha = (0xff);
	//uint8_t red = (uint8_t)(N.x + 1);
	//uint8_t green = (uint8_t)(N.y + 1);
	//uint8_t blue = (uint8_t)(N.z + 1);
	
	//return 0xff000000 | (uint8_t)(0.0f * red) | (uint8_t)(255.0f * green) | (uint8_t)(0.0f * blue);
	//return RGBAtoHEX(red, green, blue, alpha);
	//return 0xffffffff;
	//float t = 0.5f * (rayDirection.y + 1);

	//return 0xffffffff;

	//unsigned long left = (1 - t) * RGBAtoHEX(0xfa, 0x09, 0xca, 0xff);
	//unsigned long right = t * RGBAtoHEX(0x05, 0x07, 0x0a, 0xff);
	//return 0xff000000 | (left + right);

	//return (uint32_t)((1 - t) * RGBAtoHEX(1, 1, 1, 1) + t * RGBAtoHEX(5, 7, 10, 1));
	//return 255.0f * (RGBAtoHEX(red, green, blue, alpha));
	//return RGBAtoHEX(255, 99, 71, 0);
	//unsigned long left = (1 - t) * RGBAtoHEX(red, green, blue, alpha);
	//unsigned long right = t * RGBAtoHEX(red, green, blue, alpha);
	//return (1 - t) * RGBAtoHEX(1, 1, 1, 255) + t * RGBAtoHEX(0.5, 0.7, 1.0, 255);
	//return 250.f * RGBAtoHEX(red, green, blue, alpha);
	*/
}


std::list<float>* Renderer::hit_sphere(float radius, const glm::vec3& rayOrigin, const glm::vec3& rayDirection) {
	std::list<float>* intersections = new std::list<float>;
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
	/*glm::dot(x, y) - 'iloczyn skalarny' x * y
	float tSqCoeff = rayDirection.x*rayDirection.x + rayDirection.y*rayDirection.y + rayDirection.z*rayDirection.z;	//	does the same thing as the line below	*/
	float tSqCoeff = glm::dot(rayDirection, rayDirection);
	float tCoeff = 2.0f * glm::dot(rayOrigin, rayDirection);
	float volatilePart = glm::dot(rayOrigin, rayOrigin) - radius * radius;

	float delta = tCoeff * tCoeff - 4.0f * tSqCoeff * volatilePart;
	if (delta >= 0.0f) {
		float entryPoint = (-tCoeff - sqrt(delta)) / (2.0f * tSqCoeff);
		float exitPoint = (-tCoeff + sqrt(delta)) / (2.0f * tSqCoeff);

		intersections->push_back(entryPoint);
		intersections->push_back(exitPoint);
		/*
		//auto a = (rayOrigin + rayDirection * intersection1) - glm::vec3(0, 0, -1);
		//auto b = (rayOrigin + rayDirection * intersection2) - glm::vec3(0, 0, -1);

		//auto t = 0.5f * (rayDirection.y + 1.0f);
		//auto g = (1.0f - t) * glm::vec3(1.0, 1.0, 1.0);
		//auto r = t * glm::vec3(0.5, 0.7, 1.0);
		//auto combined = g + r;

		//return 0xff000000 | (uint8_t)combined.x | (uint8_t)combined.y | (uint8_t)combined.z;
		//return 0xff000000 | (uint8_t)(255.0f * a.y) | (uint8_t)(255.0f * a.x) | (uint8_t)(255.0f * a.z);
		//return 0xff000000 | (greenChannel << 8) | redChannel;*/
	}
	return intersections;
}
