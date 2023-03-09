#include <qpl/qpl.hpp>

struct main_state : qsf::base_state {
	void add_random() {
		auto dim = qpl::vec(80, 100);

		qpl::styled_string<qpl::u32_string> new_string;

		for (qpl::size i = 0u; i < dim.y; ++i) {
			if (qpl::random_b(0.1)) {
				new_string << qpl::get_random_color();
			}
			new_string << qpl::get_random_lowercase_uppercase_number_string(qpl::random(dim.x / 2.0, dim.x * 1.5)) << '\n';
		}
		this->console.add(new_string);
	}

	void init() override {
		this->console.set_font("consola");

		this->clear_color = qpl::rgb(12, 12, 12);

		this->call_on_resize();

		this->add_random();
		this->console.set_input_color(qpl::aqua);
		this->console.start_accepting_input();
	}
	void call_on_resize() override {
		this->console.set_dimension(this->dimension());
	}

	void updating() override {
		this->update(this->console);

		if (this->console.line_entered) {
			qpl::println("text = \"", this->console.get_last_input_line(), "\"");
			if (this->console.get_last_input_line() == L"func") {
				this->console.stop_accepting_input();
			}
		}

		if (this->event().key_pressed(sf::Keyboard::Tab)) {
			this->console.start_accepting_input();
		}
		if (this->event().key_pressed(sf::Keyboard::F)) {
			qpl::println("string length = ", qpl::big_number_string(this->console.string_and_input.string().length()));
		}


	}
	void drawing() override {

		this->draw(this->console);
	}

	qsf::console console;
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