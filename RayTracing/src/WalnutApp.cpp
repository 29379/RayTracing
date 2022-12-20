#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Image.h"
#include "Walnut/Timer.h"

#include "Renderer.h"
#include "Camera.h"
#include "Scene.h"

#include <glm/gtc/type_ptr.hpp>

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:
	ExampleLayer()
		: myCamera(45.0f, 0.1f, 100.0f) 
	{

		Material& firstSphere = myScene.materials.emplace_back();
		firstSphere.albedo = { 1.0f, 0.0f, 0.0f };
		firstSphere.roughness = 0.0f;

		Material& secondSphere = myScene.materials.emplace_back();
		secondSphere.albedo = { 0.2f, 0.3f, 1.0f };
		secondSphere.roughness = 0.08f;

		{
			Sphere sphere;
			sphere.position = { 0.0f, 0.0f, 0.0f };
			sphere.radius = 1.0f;
			sphere.materialIndex = 0;
			myScene.objects.push_back(sphere);
		}
		{
			Sphere sphere;
			sphere.position = { 0.0f, -101.0f, -0.0f };
			sphere.radius = 100.0f;
			sphere.materialIndex = 1;
			myScene.objects.push_back(sphere);
		}
	}

	virtual void OnUpdate(float ts) override {
		if (myCamera.OnUpdate(ts)) {
			myRenderer.ResetFrameIndex();
		}
	}
	
	virtual void OnUIRender() override {
		static float lx = -1.0f;
		static float ly = -1.0f;
		static float lz = -1.0f;
		glm::vec3 lightSlider(lx, ly, lz);

		ImGui::Begin("Settings");
		ImGui::Text("Last render time: %.3f", lastRenderTime);

		ImGui::Separator();

		if (ImGui::Button("Render")) {
			Render(lightSlider);
		}

		ImGui::Checkbox("Accumulate", &myRenderer.GetSettings().accumulate);

		if (ImGui::Button("Reset")) {
			myRenderer.ResetFrameIndex();
		}

		ImGui::Separator();
		ImGui::Text("Light source location");
		ImGui::SliderFloat("Light source: x", &lx, -5.0f, 5.0f);
		ImGui::SliderFloat("Light source: y", &ly, -5.0f, 5.0f);
		ImGui::SliderFloat("Light source: z", &lz, -5.0f, 5.0f);

		ImGui::Separator();
		for (size_t i = 0; i < myScene.objects.size(); i++) {
			ImGui::PushID(i);

			Sphere& sphere = myScene.objects[i];
			ImGui::DragFloat3("Sphere position", glm::value_ptr(sphere.position), 0.1f);
			ImGui::DragFloat("Sphere radius", &sphere.radius, 0.1f);
			ImGui::DragInt("Material", &sphere.materialIndex, 1.0f, 0, (int)myScene.materials.size()-1);

			ImGui::Separator();
			ImGui::PopID();
		}

		for (size_t i = 0; i < myScene.materials.size(); i++) {
			ImGui::PushID(i);

			Material& material = myScene.materials[i];
			ImGui::ColorEdit3("Sphere albedo", glm::value_ptr(material.albedo));
			ImGui::DragFloat("Roughness", &material.roughness, 0.01f, 0, 1);
			ImGui::DragFloat("Metallic value", &material.metallic, 0.01f, 0, 1);

			ImGui::Separator();
			ImGui::PopID();
		}

		ImGui::End();

		// getting rid of an annoying blank bar at the top here and right after ImGui::End
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		ImGui::Begin("Viewport");
		viewportWidth = ImGui::GetContentRegionAvail().x;
		viewportHeight = ImGui::GetContentRegionAvail().y;

		// render an image if there is one
		auto image = myRenderer.GetFinalImage();
		if (image) {
			ImGui::Image(image->GetDescriptorSet(),
				{ (float)image->GetWidth(), (float)image->GetHeight() },
					ImVec2(0, 1), ImVec2(1, 0));
		}

		ImGui::End();
		ImGui::PopStyleVar();

		// rendering in a loop inside the app
		Render(lightSlider);
	}

	void Render(glm::vec3 lightSlider) {
		Timer timer;

		myRenderer.OnResize(viewportWidth, viewportHeight);
		myCamera.OnResize(viewportWidth, viewportHeight);
		myRenderer.Render(myScene, myCamera, lightSlider);

		lastRenderTime = timer.ElapsedMillis();
	}


private:
	Renderer myRenderer;
	Camera myCamera;
	Scene myScene;
	uint32_t viewportWidth = 0;
	uint32_t viewportHeight = 0;

	float lastRenderTime = 0.0f;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Ray Tracing";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});
	return app;
}