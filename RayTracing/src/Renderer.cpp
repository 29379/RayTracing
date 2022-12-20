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
	delete[] accumulationData;
	accumulationData = new glm::vec4[width * height];
}


void Renderer::Render(const Scene& scene, const Camera& camera, glm::vec3 lightSlider) {
	// rendering the pixels

	activeScene = &scene;
	activeCamera = &camera;

	/*
	for (uint32_t i = 0; i < finalImage->GetWidth() * finalImage->GetHeight(); i++) {
		imageData[i] = Walnut::Random::UInt();
		i dont want the alpha channel to be random, to
		be sure to always 'see' stuff, so i set
		the most significant bytes to 255	
		imageData[i] |= 0xff000000;
	}
	*/
	//	float aspectRatio = finalImage->GetWidth() / (float)finalImage->GetHeight();

	//	if it is frame 1 then we have no data, so the buffer has to get cleared all across the image
	if (frameIndex == 1) {
		memset(accumulationData, 0, finalImage->GetHeight() * finalImage->GetWidth() * sizeof(glm::vec4));
	}

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

	// uploading pixel data to the GPU
	finalImage->SetData(imageData);

	if (settings.accumulate) {
		frameIndex++;
	}
	else {
		frameIndex = 1;
	}
}

//glm::vec4 Renderer::TraceRay(const Ray& ray) {
//	/*	
//	glm::dot(x, y) - 'iloczyn skalarny' x * y
//		sphere equation:
//		-	t^2*(bx^2+by^2 + bz^2) + t(2(axbx + ayby + azbz)) + (ax^2 + ay^2 + az^2 - r^2) = 0
//	a	-	ray origin
//	b	-	ray direction
//	r	-	radius of the sphere
//	t	-	intersection distances
//	quadratic equation:
//							-	dx^2 + ex + f = 0
//	t^2 d coeff				-	tSqCoeff
//	t e coeff				-	tCoeff
//	f constant				-	volatilePart
//	delta (discriminant)	-	e^2 - 4*d*f
//
//	intersectios equation:
//		-	(-e +- sqrt(delta)) / (2.0f * d)
//	*/
//	//	float radius = 0.5f;
//	//	float sphereOrigin = 0.0f;
//
//	//	rayDirection = glm::normalize(rayDirection);	//	creates a unit vector
//
//	//	float a = glm::dot(ray.direction, ray.direction);
//	//	float b = 2.0f * glm::dot(ray.origin, ray.direction);
//	//	float c = glm::dot(ray.origin, ray.origin) - radius * radius;
//	//	float delta = b * b - 4.0f * a * c;
//
//	if (scene.objects.size() == 0) {
//		auto t = 0.5f * (ray.direction.y + 1);
//		return (1.0f - t) * glm::vec4(1, 1, 1, 1) + t * glm::vec4(0.5, 0.7, 1, 1);
//		//return glm::vec4(0, 0, 0, 1);
//	}
//
//	//	float entryPointDistance = (-b - glm::sqrt(delta)) / (2.0f * a);
//	//	float exitPointDistance = (-b + glm::sqrt(delta)) / (2.0f * a);
//
//	//	return glm::vec4(1, 0.3, 0.7, 1);
//
//	const Sphere* closestSphere = nullptr;
//	float closestHit = FLT_MAX;
//
//	for (const Sphere& sphere : scene.objects) {
//		glm::vec3 origin = ray.origin - sphere.position;
//		float a = glm::dot(ray.direction, ray.direction);
//		float b = 2.0f * glm::dot(origin, ray.direction);
//		float c = glm::dot(origin, origin) - sphere.radius * sphere.radius;
//		float delta = b * b - 4.0f * a * c;
//
//		//	nothing is returned here, because even if i dont hit e.g. the first
//		//	sphere, some other might be, so i gotta go along with 
//		//	checking all of them from the scene
//		if (delta < 0.0f) {
//			continue;
//		}
//
//		float entryPointDistance = (-b - glm::sqrt(delta)) / (2.0f * a);
//		//	float exitPointDistance = (-b + glm::sqrt(delta)) / (2.0f * a);
//
//		if (entryPointDistance < closestHit)
//		{
//			closestHit = entryPointDistance;
//			closestSphere = &sphere;
//		}
//	}
//
//	//	if it still is nullptr at this point, there are no intersections
//	if (closestSphere == nullptr) {
//		auto t = 0.5f * (ray.direction.y + 1);
//		return (1.0f - t) * glm::vec4(1, 1, 1, 1) + t * glm::vec4(0.5, 0.7, 1, 1);
//	}
//	
//	glm::vec3 origin = ray.origin - closestSphere->position;
//	glm::vec3 hitPoint = origin + ray.direction * closestHit;
//	glm::vec3 normal = glm::normalize(hitPoint);
//
//	glm::vec3 lightDirection = glm::normalize(glm::vec3(-1, -1, -1));
//	float d = glm::max(glm::dot(normal, -lightDirection), 0.0f);
//	glm::vec4 sphereColor = {closestSphere->albedo, 1.0f};
//
//
//	//	glm::vec3 entryPoint = ray.origin + entryPointDistance * ray.direction;
//	//	glm::vec3 exitPoint = ray.origin + exitPointDistance * ray.direction;
//
//	//	glm::vec3 normal = glm::normalize(entryPoint - sphereOrigin);
//
//	//	glm::vec3 lightDirection = glm::normalize(lightSlider);
//	//	float d = glm::max(glm::dot(normal, -lightDirection), 0.0f);		//	d == -cos(angle), cos(> 90) < 0
//	//	float d = glm::dot(normal, -lightDirection);
//
//	//	values of the normal were in <-1; 1>, but the operations with 0.5f pushed it to <0; 1> bounds
//	//	glm::vec3 sphereColor2 = normal * 0.5f + 0.5f;
//	//	glm::vec4 sphereColor(sphereColor2, 1.0f);
//	//	glm::vec4 sphereColor = colorsSlider;
//	sphereColor *= d;
//
//	return glm::vec4(sphereColor);
//}

glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y) {
	Ray ray;
	ray.origin = activeCamera->GetPosition();
	ray.direction = activeCamera->GetRayDirections()[x + y * finalImage->GetWidth()];

	glm::vec3 color(0.0f);
	float multiplier = 1.0f;

	//	bounces are used to make the spheres reflect their image on themselves, kinda like mirrors
	int bounces = 5;
	for (int i = 0; i < bounces; i++) {
		Renderer::HitPayload payload = TraceRay(ray);

		if (payload.hitDistance < 0.0f) {
			//	ambient occlusion? important term for realistic rendering
			glm::vec3 skyColor = glm::vec3(0.6f, 0.7f, 0.9f);
			color += skyColor * multiplier; 
			break;
		}

		glm::vec3 lightDir = glm::normalize(glm::vec3(-1, -1, -1));
		float lightIntensity = glm::max(glm::dot(payload.worldNormal, -lightDir), 0.0f); // == cos(angle)

		const Sphere& sphere = activeScene->objects[payload.objectIndex];
		const Material& material = activeScene->materials[sphere.materialIndex];

		glm::vec3 sphereColor = material.albedo;
		sphereColor *= lightIntensity;
		color += sphereColor * multiplier;

		multiplier *= 0.5f;

		ray.origin = payload.worldPosition + payload.worldNormal * 0.0001f;
		//	adding random noise to the reflected rays, representing the roughness of a surface
		ray.direction = glm::reflect(ray.direction, 
			payload.worldNormal + material.roughness * Walnut::Random::Vec3(-0.5f, 0.5f));
	}

	return glm::vec4(color, 1.0f);
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

	Renderer::HitPayload payload;
	payload.hitDistance = hitDistance;
	payload.objectIndex = objectIndex;

	//glm::vec3 sphereColor = closestSphere.albedo;
	//sphereColor *= lightIntensity;
	//return glm::vec4(sphereColor, 1.0f);

	const Sphere& closestSphere = activeScene->objects[objectIndex];

	glm::vec3 origin = ray.origin - closestSphere.position;
	payload.worldPosition = origin + ray.direction * hitDistance;
	payload.worldNormal = glm::normalize(payload.worldPosition);

	payload.worldPosition += closestSphere.position;

	return payload;
}


Renderer::HitPayload Renderer::Miss(const Ray& ray){
	Renderer::HitPayload payload;
	payload.hitDistance = -1.0f;
	return payload;
}