#include <molten/molten.hpp>
#include <molten/ui.hpp>

void loop() {
	static std::string buffer;
	UI::Text("MoltenUI Demo", {20, 30}, {0.4f, 0.6f, 1.0f, 1.0f});

	if(UI::Button(0, {20, 60}, {100, 35}, "Clear")) {
		buffer = "";
	}

	UI::InputField(1, buffer, {20, 110}, {250, 40});
	UI::Text("Status: Active", {20, 170}, {0.5f, 0.5f, 0.5f, 1.0f});
}

class MoltenApp final {
  private:
	Molten *molt;

  public:
	MoltenApp(Molten *molt) : molt(molt) {};
	~MoltenApp() = default;

	int start() {
		molt->init();
		molt->run(loop);

		return 0;
	}
};

int main() {
	Molten molt("Demo", 1280, 720);
	MoltenApp app(&molt);
	app.start();
}
