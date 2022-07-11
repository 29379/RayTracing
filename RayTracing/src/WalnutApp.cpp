#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Random.h"
#include "Walnut/Timer.h"

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

		// it recreates the image if it is null, or if it got resized
		if (!image || viewportHeight != image->GetWidth() || viewportHeight != image->GetHeight()){
			image = std::make_shared<Image>(viewportWidth, viewportHeight, ImageFormat::RGBA);
			delete[] imageData;
			imageData = new uint32_t[viewportWidth * viewportHeight];
		}

		for (uint32_t i = 0; i < viewportWidth * viewportHeight; i++) {
			imageData[i] = Random::UInt();
			// i dont want the alpha channel to be random, to
			// be sure to always 'see' stuff, so i set 
			// the most significant bytes to 265
			imageData[i] |= 0xff000000;
		}

		// uploading pixel data to the GPU
		image->SetData(imageData);

		lastRenderTime = timer.ElapsedMillis();
	}

private:
	std::shared_ptr<Image> image;
	uint32_t viewportWidth = 0;
	uint32_t viewportHeight = 0;
	uint32_t* imageData = nullptr; // buffer of pixel data

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