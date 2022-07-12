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
		if (ImGui::Button("Render")) {
			Render();
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
			ImGui::Image(image->GetDescriptorSet(), { (float)image->GetWidth(), (float)image->GetHeight() });
		}

		ImGui::End();
		ImGui::PopStyleVar();
		
		// rendering in a loop, inside the app, not only after clicking the button
		Render();
	}

	void Render() {
		Timer timer;

		myRenderer.OnResize(viewportWidth, viewportHeight);
		myRenderer.Render();

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
	spec.Name = "Ray Tracing Example";

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