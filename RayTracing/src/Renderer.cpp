#include "Renderer.h"
#include <execution>

namespace Utils {

	static uint32_t Vec4ToRGBA(const glm::vec4& color) {
		uint8_t r = (uint8_t)(color.r * 255.0f);
		uint8_t g = (uint8_t)(color.g * 255.0f);
		uint8_t b = (uint8_t)(color.b * 255.0f);
		uint8_t a = (uint8_t)(color.a * 255.0f);

		uint32_t output = (a << 24) | (b << 16) | (g << 8) | r;
		return output;
	}

	static uint32_t WangHash(uint32_t seed) {
		seed = (seed ^ 61) ^ (seed >> 16);
		seed *= 9;
		seed = seed ^ (seed >> 4);
		seed *= 0x27d4eb2d;
		seed = seed ^ (seed >> 15);
		return seed;
	}

	static uint32_t PcgHash(uint32_t input) {
		uint32_t state = input * 747796405u + 2891336453u;
		uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
		return (word >> 22u) ^ word;
	}

	static float RandomFloat(uint32_t& seed) {
		seed = PcgHash(seed);
		return (float)seed / (float)std::numeric_limits<uint32_t>::max();
	}

	static glm::vec3 InUnitSphere(uint32_t& seed) {
		return glm::normalize(glm::vec3(
			RandomFloat(seed) * 2.0f - 1.0f,
			RandomFloat(seed) * 2.0f - 1.0f,
			RandomFloat(seed) * 2.0f - 1.0f
		));
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

	delete[] imageData;
	imageData = new uint32_t[width * height];
	delete[] accumulationData;
	accumulationData = new glm::vec4[width * height];

	horizontalIter.resize(width);
	verticalIter.resize(height);

	for (uint32_t i = 0; i < width; i++) {
		horizontalIter[i] = i;
	}
	for (uint32_t i = 0; i < height; i++) {
		verticalIter[i] = i;
	}

}


void Renderer::Render(const Scene& scene, const Camera& camera) {
	// rendering the pixels

	activeScene = &scene;
	activeCamera = &camera;

	//	if it is frame 1 then we have no data, so the buffer has to get cleared all across the image
	if (frameIndex == 1) {
		memset(accumulationData, 0, finalImage->GetHeight() * finalImage->GetWidth() * sizeof(glm::vec4));
	}

/*
	MT 0	-	~80ms
	MT 1	-	~40ms
*/
#define MT 1
#if MT
	std::for_each(std::execution::par, verticalIter.begin(), verticalIter.end(),
		[this](uint32_t y)
			{
				std::for_each(std::execution::par, horizontalIter.begin(), horizontalIter.end(),
					[this, y](uint32_t x)
					{
						glm::vec4 color = PerPixel(x, y);
						accumulationData[x + y * finalImage->GetWidth()] += color;	//	doesn't have to be clamped, because accumulationData accepts floats

						//	without normalizing it, the image would become unnaturally bright
						glm::vec4 accumulatedColor = accumulationData[x + y * finalImage->GetWidth()];
						accumulatedColor /= (float)frameIndex;

						accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));
						imageData[x + y * finalImage->GetWidth()] = Utils::Vec4ToRGBA(accumulatedColor);
					});
			});
#else
	for (uint32_t y = 0; y < finalImage->GetHeight(); y++) {
		for (uint32_t x = 0; x < finalImage->GetWidth(); x++) {
			glm::vec4 color = PerPixel(x, y);
			accumulationData[x + y * finalImage->GetWidth()] += color;	//	doesn't have to be clamped, because accumulationData accepts floats

			//	without normalizing it, the image would become unnaturally bright
			glm::vec4 accumulatedColor = accumulationData[x + y * finalImage->GetWidth()];
			accumulatedColor /= (float)frameIndex;

			accumulatedColor = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
			imageData[x + y * finalImage->GetWidth()] = Utils::Vec4ToRGBA(accumulatedColor);
		}
	}
#endif

	// uploading pixel data to the GPU
	finalImage->SetData(imageData);

	if (settings.accumulate) {
		frameIndex++;
	}
	else {
		frameIndex = 1;
	}
}


glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y) {
	Ray ray;
	ray.origin = activeCamera->GetPosition();
	ray.direction = activeCamera->GetRayDirections()[x + y * finalImage->GetWidth()];

	glm::vec3 light(0.0f);
	//	as light bounces, some wavelength will be absorbed, and some
	//	redirected - this multiplier will help in representing 
	//	the particular ray after its been bouncing around for a while
	//	and the color contribution that it carries
	glm::vec3 lightColorContribution(1.0f);

	//	bounces are used to make the spheres reflect their image on themselves, kinda like mirrors
	int bounces = 5;

	uint32_t seed = x + y * finalImage->GetWidth();
	seed *= frameIndex;

	for (int i = 0; i < bounces; i++) {
		Renderer::HitPayload payload = TraceRay(ray);
		seed += i;

		if (payload.hitDistance < 0.0f) {
			//	ambient occlusion - important term for realistic rendering
			glm::vec3 skyColor = glm::vec3(0.6f, 0.7f, 0.9f);
			//light += skyColor * lightColorContribution; 
			break;
		}

		//glm::vec3 lightDir = glm::normalize(glm::vec3(-1, -1, -1));
		//float lightIntensity = glm::max(glm::dot(payload.worldNormal, -lightDir), 0.0f); // == cos(angle)

		const Sphere& sphere = activeScene->objects[payload.objectIndex];
		const Material& material = activeScene->materials[sphere.materialIndex];
		

		lightColorContribution *= material.albedo;
		light += material.GetEmission();	// *material.albedo;

		ray.origin = payload.worldPosition + payload.worldNormal * 0.0001f;
		//	adding random noise to the reflected rays, representing the roughness of a surface
		if (settings.slowRandom) {
			ray.direction = glm::normalize(payload.worldNormal + Walnut::Random::InUnitSphere());
		}
		else{
			ray.direction = glm::normalize(payload.worldNormal + Utils::InUnitSphere(seed));
		}

	}

	return glm::vec4(light, 1.0f);
}


Renderer::HitPayload Renderer::TraceRay(const Ray& ray) {
	int closestSphere = -1;
	float hitDistance = FLT_MAX;

	for (size_t i = 0; i < activeScene->objects.size(); i++) {
		const Sphere& sphere = activeScene->objects[i];
		glm::vec3 origin = ray.origin - sphere.position;

		float a = glm::dot(ray.direction, ray.direction);
		float b = 2.0f * glm::dot(origin, ray.direction);
		float c = glm::dot(origin, origin) - sphere.radius * sphere.radius;
		// Quadratic forumula discriminant:
		// b^2 - 4ac
		float delta = b * b - 4.0f * a * c;
		if (delta < 0.0f) {
			continue;
		}
		// Quadratic formula:
		// (-b +- sqrt(discriminant)) / 2a

		// float t0 = (-b + glm::sqrt(discriminant)) / (2.0f * a); // Second hit distance (currently unused)
		float closestT = (-b - glm::sqrt(delta)) / (2.0f * a);

		if (closestT > 0.0f && closestT < hitDistance) {
			hitDistance = closestT;
			closestSphere = (int)i;
		}
	}

	if (closestSphere < 0){
		return Miss(ray);
	}

	return ClosestHit(ray, hitDistance, closestSphere);
}


Renderer::HitPayload Renderer::ClosestHit(const Ray& ray, float hitDistance, int objectIndex){

	Renderer::HitPayload payload{};
	payload.hitDistance = hitDistance;
	payload.objectIndex = objectIndex;

	const Sphere& closestSphere = activeScene->objects[objectIndex];

	glm::vec3 origin = ray.origin - closestSphere.position;
	payload.worldPosition = origin + ray.direction * hitDistance;
	payload.worldNormal = glm::normalize(payload.worldPosition);

	payload.worldPosition += closestSphere.position;

	return payload;
}


Renderer::HitPayload Renderer::Miss(const Ray& ray){
	Renderer::HitPayload payload{};
	payload.hitDistance = -1.0f;
	return payload;
}