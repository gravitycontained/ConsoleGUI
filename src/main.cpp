#include <qpl/qpl.hpp>

struct console_state : qsf::base_state {

	void add_random() {
		auto dim = qpl::vec(50, 1);

		qpl::styled_string<qpl::u32_string> new_string;

		for (qpl::size i = 0u; i < dim.y; ++i) {
			if (qpl::random_b(0.1)) {
				new_string << qpl::get_random_color();
			}
			new_string << qpl::get_random_lowercase_uppercase_number_string(qpl::random(dim.x / 2.0, dim.x * 1.5));
		}
		this->console.add(new_string);
	}

	void init() override {
		this->console.set_font("consola");
		this->console.set_unicode_font("unifont");
		this->console.set_border_texture(qsf::get_texture("border"));

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

		if (this->console.text_entered) {
			qpl::println("cursor_pos = ", this->console.cursor_position);
		}
		if (this->console.line_entered) {

			auto input = this->console.get_last_input_line();
			qpl::println("text = \"", input, "\"");
			if (input == L"func") {

				this->console.stop_accepting_input();
			}
			this->console.process_text();
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

	qsf::console2 console;
};

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

	void add_count_up() {

		std::vector<qpl::size> double_wides;

		auto default_advance = this->console.colored_text.get_unicode_glyph(U'x', this->console.colored_text.character_size, false);

		qpl::styled_string<qpl::u32_string> new_string;
		for (qpl::size i = 0u; i < 0x50'000; ++i) {

			auto glyph = this->console.colored_text.get_unicode_glyph(i, this->console.colored_text.character_size, false);
			char32_t value = i;
			//new_string << qpl::to_string(i) << std::string(": ") << qpl::to_u32_string(value) << std::string(" (") << qpl::to_string(glyph.textureRect.left) << std::string(" ") << qpl::to_string(glyph.textureRect.top) << std::string(" ") << qpl::to_string(glyph.textureRect.width) << std::string(" ") << qpl::to_string(glyph.textureRect.height) << std::string("\n");
			new_string << qpl::to_u32_string(value);

			bool double_wide = qpl::unicode_character_length(i) == 2u;
			auto glyph_big = glyph.advance / default_advance.advance >= 1.5;

			if (double_wide != glyph_big) {
				qpl::println("sadge");
			}
			if (glyph_big) {
				double_wides.push_back(i);
			}
			
			
			if (i % 100 == 0u) {
				new_string << '\n';
			}
		}
		this->console.add(new_string);

		qpl::size before = 0u;
		std::vector<std::pair<qpl::size, qpl::size>> ranges;
		for (qpl::size i = 0u; i < double_wides.size(); ++i) {
			if (qpl::find(double_wides, i)) {
				if (ranges.size() && i == before + 1) {
					++ranges.back().second;
				}
				else {
					ranges.push_back(std::pair(i, 0ull));
				}
				before = i;
			}
		}

		qpl::println(ranges);
	}

	void init() override {
		this->console.set_font("consola");
		this->console.set_unicode_font("unifont");
		this->console.set_border_texture(qsf::get_texture("border"));

		this->clear_color = qpl::rgb(12, 12, 12);

		this->call_on_resize();

		this->add_random();
		//this->add_count_up();
		this->console.set_input_color(qpl::aqua);
		this->console.start_accepting_input();
	}
	void call_on_resize() override {
		this->console.set_dimension(this->dimension());
	}

	void updating() override {
		this->update(this->console);

		if (this->console.line_entered) {

			auto input = this->console.get_last_input_line();
			qpl::println("text = \"", input, "\"");
			if (input == L"func") {

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
	framework.add_font("unifont", "resources/unifont.ttf");
	framework.add_texture("border", "resources/border.png");
	framework.set_dimension({ 1400u, 950u });

	//framework.add_state<main_state>();
	framework.add_state<console_state>();
	framework.game_loop();
}
catch (std::exception& any) {
	qpl::println("caught exception:\n", any.what());
	qpl::system_pause();
}