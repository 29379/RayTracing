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
		{
			Sphere sphere;
			sphere.position = { 0.0f, 0.0f, 0.0f };
			sphere.albedo = { 1.0f, 1.0f, 1.0f };
			sphere.radius = 1.0f;
			myScene.objects.push_back(sphere);
		}
		{
			Sphere sphere;
			sphere.position = { 1.0f, 0.0f, -5.0f };
			sphere.albedo = { 0.2f, 0.3f, 1.0f };
			sphere.radius = 1.5f;
			myScene.objects.push_back(sphere);
		}
	}

	virtual void OnUpdate(float ts) override {
		myCamera.OnUpdate(ts);
	}
	
	virtual void OnUIRender() override {
		ImGui::Begin("Settings");
		ImGui::Text("Last render time: %.3f", lastRenderTime);

		ImGui::Separator();

		static float lx = -1.0f;
		static float ly = -1.0f;
		static float lz = -1.0f;

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
			ImGui::ColorEdit3("Sphere albedo", glm::value_ptr(sphere.albedo));

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
		
		glm::vec3 lightSlider(lx, ly, lz);

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