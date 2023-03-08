#include <qpl/qpl.hpp>
#include "console.hpp"

struct main_state : qsf::base_state {
	void init() override {
		this->console.set_font("consola");

		this->clear_color = qpl::rgb(12, 12, 12);

		this->call_on_resize();

		this->console.add_random();
		this->console.start_accepting_input();
	}
	void call_on_resize() override {
		this->console.set_dimension(this->dimension());
	}

	void updating() override {
		this->update(this->console);

		//if (this->console.enter_pressed) {
		//	this->console.stop_accepting_input();
		//}
		if (this->event().key_pressed(sf::Keyboard::Tab)) {
			this->console.start_accepting_input();
		}
	}
	void drawing() override {

		this->draw(this->console);
	}

	console console;
};

int main() try {

	qsf::framework framework;
	framework.set_title("QPL");
	framework.add_font("consola", "resources/consola.ttf");
	framework.set_dimension({ 1400u, 950u });

	framework.add_state<main_state>();
	framework.game_loop();
}
catch (std::exception& any) {
	qpl::println("caught exception:\n", any.what());
	qpl::system_pause();
}