#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Image.h"
#include "Walnut/Timer.h"

#include "Renderer.h"

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:
	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
		ImGui::Text("Last render time: %.3f", lastRenderTime);
		
		static float red = 1.0f;
		static float green = 1.0f;
		static float blue = 1.0f;
		static float alpha = 1.0f;

		ImGui::Separator();
		ImGui::Text("Sphere colors:");
		ImGui::SliderFloat("Red", &red, 0.0f, 1.0f);
		ImGui::SliderFloat("Green", &green, 0.0f, 1.0f);
		ImGui::SliderFloat("Blue", &blue, 0.0f, 1.0f);
		ImGui::SliderFloat("Alpha", &alpha, 0.0f, 1.0f);

		static float cx = 0.0f;
		static float cy = 0.0f;
		static float cz = 1.0f;

		ImGui::Separator();
		ImGui::Text("Camera location");
		ImGui::SliderFloat("Camera: x", &cx, -5.0f, 5.0f);
		ImGui::SliderFloat("Camera: y", &cy, -5.0f, 5.0f);
		ImGui::SliderFloat("Camera: z", &cz, -5.0f, 5.0f);

		static float lx = -1.0f;
		static float ly = -1.0f;
		static float lz = -1.0f;

		ImGui::Separator();
		ImGui::Text("Light source location");
		ImGui::SliderFloat("Light source: x", &lx, -5.0f, 5.0f);
		ImGui::SliderFloat("Light source: y", &ly, -5.0f, 5.0f);
		ImGui::SliderFloat("Light source: z", &lz, -5.0f, 5.0f);

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
		
		glm::vec4 sphereColor(red, green, blue, alpha);
		glm::vec3 cameraSlider(cx, cy, cz);
		glm::vec3 lightSlider(lx, ly, lz);

		// rendering in a loop, inside the app, not only after clicking the button
		Render(sphereColor, cameraSlider, lightSlider);
	}

	void Render(glm::vec4 colorSlider, glm::vec3 cameraSlider, glm::vec3 lightSlider) {
		Timer timer;

		myRenderer.OnResize(viewportWidth, viewportHeight);
		myRenderer.Render(colorSlider, cameraSlider, lightSlider);

		lastRenderTime = timer.ElapsedMillis();
	}

private:
	Renderer myRenderer;
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