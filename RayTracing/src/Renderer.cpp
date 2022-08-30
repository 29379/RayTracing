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
		the spot in memory stays, i change only the inside content)	*/
		finalImage->Resize(width, height);
	}
	else {
		finalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}
	//	delete nullptr is valid, it will do it internally for me
	delete[] imageData;
	imageData = new uint32_t[width * height];
}


void Renderer::Render(const Camera& camera, glm::vec4 colorSlider, glm::vec3 lightSlider) {
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
	Ray ray;
	ray.origin = camera.GetPosition();

	for (uint32_t y = 0; y < finalImage->GetHeight(); y++) {
		for (uint32_t x = 0; x < finalImage->GetWidth(); x++) {

			/*
			//	unnecessary at this point - these calculations are already done in camera.GetRayDirections
			glm::vec2 coord = { x / (float)finalImage->GetWidth(), y / (float)finalImage->GetHeight() };
			coord = coord * 2.0f - 1.0f;	//	remapping the coordinates:	0 - 1 range	--->	-1 - 1 range
			coord.x *= aspectRatio;	//	makes sure the image wont get distorted when it gets resized
			*/
			/*	2D x and y coords are flattened as if the y coord is 
			multiplied by how big each row is, with adding the x column offset
			which gives me the equivalent of 'i' from a single for loop*/
			ray.direction = camera.GetRayDirections()[x + y * finalImage->GetWidth()];

			glm::vec4 color = TraceRay(ray, colorSlider, lightSlider);
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
glm::vec4 Renderer::TraceRay(const Ray& ray, glm::vec4 colorsSlider, glm::vec3 lightSlider) {
	/*	
	glm::dot(x, y) - 'iloczyn skalarny' x * y
		sphere equation:
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
	float radius = 0.5f;
	float sphereOrigin = 0.0f;

	//	rayDirection = glm::normalize(rayDirection);	//	creates a unit vector

	float a = glm::dot(ray.direction, ray.direction);
	float b = 2.0f * glm::dot(ray.origin, ray.direction);
	float c = glm::dot(ray.origin, ray.origin) - radius * radius;
	float delta = b * b - 4.0f * a * c;

	if (delta < 0) {
		auto t = 0.5f * (ray.direction.y + 1);
		return (1.0f - t) * glm::vec4(1, 1, 1, 1) + t * glm::vec4(0.5, 0.7, 1, 1);
		//return glm::vec4(0, 0, 0, 1);
	}

	float entryPointDistance = (-b - glm::sqrt(delta)) / (2.0f * a);
	float exitPointDistance = (-b + glm::sqrt(delta)) / (2.0f * a);

	//	return glm::vec4(1, 0.3, 0.7, 1);
	
	glm::vec3 entryPoint = ray.origin + entryPointDistance * ray.direction;
	glm::vec3 exitPoint = ray.origin + exitPointDistance * ray.direction;

	glm::vec3 normal = glm::normalize(entryPoint - sphereOrigin);

	glm::vec3 lightDirection = glm::normalize(lightSlider);
	float d = glm::max(glm::dot(normal, -lightDirection), 0.0f);		//	d == -cos(angle), cos(> 90) < 0
	//float d = glm::dot(normal, -lightDirection);

	//	values of the normal were in <-1; 1>, but the operations with 0.5f pushed it to <0; 1> bounds
	//	glm::vec3 sphereColor2 = normal * 0.5f + 0.5f;
	//	glm::vec4 sphereColor(sphereColor2, 1.0f);
	glm::vec4 sphereColor = colorsSlider;
	sphereColor *= d;

	return glm::vec4(sphereColor);
}