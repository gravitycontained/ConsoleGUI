#pragma once

#include <qpl/qpl.hpp>
#include "Text.hpp"
#include "scroll_bar.hpp"

struct console {
	text text;
	qpl::styled_string<qpl::u32_string> string;
	qpl::styled_string<qpl::u32_string> input_string;
	qsf::view view;
	qpl::vec2f dimension;
	qpl::isize zooms = 0;
	qpl::isize view_row = 0;
	qpl::size visible_rows = 0u;
	scroll_bar scroll_bar; 
	qpl::f64 cursor_interval_duration = 1.0;
	qsf::rectangle cursor;
	qpl::small_clock cursor_blink_timer;

	qpl::animation scroll_transition_animation;
	qpl::f64 scroll_transition_start = 0.0;
	qpl::f64 scroll_transition_end = 0.0;
	qpl::f64 scroll_bar_transition_start = 0.0;
	qpl::f64 scroll_bar_transition_end = 0.0;

	qpl::vec2f character_size;

	qpl::size before_input_vertices_size = qpl::size_max;
	qpl::size before_input_outline_vertices_size = qpl::size_max;
	qpl::vec2f before_input_text_position;
	qpl::vec2s cursor_position;
	bool accept_input = false;
	bool enter_pressed = false;


	void init() {
		this->scroll_transition_animation.set_duration(0.4);
		this->view.position.x = -10.0f;
		this->text.character_size = 18u;
		this->update_cursor_dimension();
		this->update_cursor_position();
	}

	void set_font(std::string font) {
		this->text.set_font(qsf::get_font(font));
		this->text.set_character_size(20u);
		this->init();
		this->calculate_default_character_size();
		//this->input.set_font(font);
	}
	void calculate_default_character_size() {
		this->character_size.y = this->text.get_line_spacing_pixels();
		this->character_size.x = this->text.font->getGlyph(U'x', this->text.character_size, false).bounds.width;
	}

	void track_before_input_values() {
		this->before_input_vertices_size = this->text.vertices.size();
		this->before_input_outline_vertices_size = this->text.outline_vertices.size();
		this->before_input_text_position = this->text.text_position;
	}
	void start_accepting_input() {
		this->accept_input = true;
		this->track_before_input_values();
		this->cursor_position = { 0, 0 };
		this->update_cursor_position(true);
	}
	void stop_accepting_input() {
		this->accept_input = false;
		this->string << this->input_string;
		this->input_string.clear();
	}
	void clamp_view_y(bool transition = true) {
		if (transition) {
			this->scroll_transition_start = this->view.position.y;
		}

		auto max_rows = qpl::isize_cast((this->dimension.y * this->view.scale.y) / this->text.get_line_spacing_pixels());
		//this->view_row = qpl::clamp(0ll, this->view_row, qpl::max(0ll, qpl::signed_cast(this->text.rows) - max_rows));
		this->view_row = qpl::max(0ll, this->view_row);

		if (transition) {
			this->scroll_transition_end = this->view_row * this->text.get_line_spacing_pixels();
			this->scroll_transition_animation.reset_and_start();
		}
	}
	void set_dimension(qpl::vec2f dimension) {
		this->dimension = dimension;
		auto width = 30.0f;
		auto margin = 5.0f;
		this->scroll_bar.set_position({ this->dimension.x - (width + margin), margin });
		this->scroll_bar.set_dimension(qpl::vec2f(width, this->dimension.y - margin * 2));
		this->update_visible_rows_count();
		this->clamp_view_y();
		this->scroll_bar.update_knob();
	}

	void prepare_scroll() {
		this->scroll_bar_transition_start = this->scroll_bar.visible_knob_progress;
		this->scroll_bar.set_progress(this->view_row / qpl::f64_cast(this->text.rows - this->visible_rows));
		this->scroll_bar_transition_end = this->scroll_bar.get_progress();

		this->clamp_view_y();
	}

	void update_visible_rows_count() {
		this->visible_rows = std::floor(this->dimension.y / this->text.get_line_spacing_pixels());
		this->scroll_bar.set_progress_integer_step(this->text.rows + 1, this->visible_rows);
	}
	void end_animation() {
		this->scroll_transition_start = this->scroll_transition_end;
		this->view.position.y = this->scroll_transition_end;
		this->scroll_bar.set_visible_knob_progress(this->scroll_bar_transition_end);
		this->scroll_transition_animation.reset();
	}

	void update_cursor_position(bool reset_timer = false) {

		auto off = this->cursor_position * this->character_size;
		this->cursor.set_position(this->before_input_text_position + off - qpl::vec(0.0f, this->text.character_size));
		if (reset_timer) {
			this->cursor_blink_timer.reset();
		}
	}
	void update_input_text_graphics() {
		if (this->before_input_vertices_size == qpl::size_max) {
			return;
		}
		if (this->before_input_outline_vertices_size == qpl::size_max) {
			return;
		}
		this->text.text_position = this->before_input_text_position;
		this->text.vertices.resize(this->before_input_vertices_size);
		this->text.outline_vertices.resize(this->before_input_outline_vertices_size);
		this->text.add(this->input_string);
		this->update_cursor_position(false);
	}
	void add_text_input(const qpl::u32_string& string) {
		auto split = qpl::string_split(qpl::to_basic_string<wchar_t>(string), L'\n');
		if (split.size() > 1) {
			this->cursor_position.y += (split.size() - 1);
			this->cursor_position.x = split.back().length();
		}
		else {
			this->cursor_position.x += string.length();
		}

		this->input_string << string;
		this->update_input_text_graphics();

		this->update_cursor_position(false);
	}
	void pop_last_character() {
		if (this->input_string.size() && this->input_string.elements.back().text.length()) {
			this->input_string.elements.back().text.pop_back();
			this->update_input_text_graphics();
		}
	}

	void update_cursor_dimension() {
		this->cursor.set_dimension(qpl::vec(3.0f, this->text.get_line_spacing_pixels()));
	}
	void handle_zoom() {
		this->text.create(this->string);
		this->track_before_input_values();

		if (this->accept_input) {
			this->text.add(this->input_string);
		}
		this->update_visible_rows_count();
		this->scroll_bar.set_progress(this->view_row / qpl::f64_cast(this->text.rows - this->visible_rows));
		this->scroll_bar.set_visible_knob_progress(this->view_row / qpl::f64_cast(this->scroll_bar.integer_step));
		this->view.position.y = this->view_row * this->text.get_line_spacing_pixels();
		this->clamp_view_y(false);


		this->calculate_default_character_size();
		this->update_cursor_dimension();
		this->update_cursor_position(false);
	}
	qpl::size get_input_text_width(qpl::size y) const {
		auto split = qpl::string_split(qpl::to_basic_string<wchar_t>(this->input_string.string()), L'\n');
		return split.at(y).length();
	}
	qpl::size get_input_text_height() const {
		auto split = qpl::string_split(qpl::to_basic_string<wchar_t>(this->input_string.string()), L'\n');
		return split.size();
	}
	void update_text_input(const qsf::event_info& event) {
		this->enter_pressed = false;
		if (!this->accept_input) {
			return;
		}
		bool special_input = false;

		if (event.key_holding(sf::Keyboard::LControl)) {
			if (event.key_pressed(sf::Keyboard::V)) {
				this->add_text_input(qpl::to_u32_string(qsf::copy_from_clipboard()));
				special_input = true;
			}
		}
		if (event.key_pressed(sf::Keyboard::Backspace)) {
			this->pop_last_character();
			special_input = true;
		}
		if (event.key_pressed(sf::Keyboard::Enter)) {
			this->add_text_input(qpl::to_u32_string('\n'));
			special_input = true;
			this->enter_pressed = true;
		}
		if (event.key_pressed(sf::Keyboard::Right)) {
			if (this->cursor_position.x < this->get_input_text_width(this->cursor_position.y)) {
				++this->cursor_position.x;
				this->update_cursor_position(true);
			}
		}
		if (event.key_pressed(sf::Keyboard::Left)) {
			if (this->cursor_position.x) {
				--this->cursor_position.x;
				this->update_cursor_position(true);
			}
		}
		if (event.key_pressed(sf::Keyboard::Up)) {
			if (this->cursor_position.y) {
				--this->cursor_position.y;
				this->update_cursor_position(true);
			}
			else {
				//todo add select last command
			}
		}
		if (event.key_pressed(sf::Keyboard::Down)) {
			if (this->cursor_position.y < this->get_input_text_height()) {
				++this->cursor_position.y;
				this->update_cursor_position(true);
			}
			else {
				this->input_string.clear();
				this->update_input_text_graphics();
				special_input = true;
			}
		}
		if (event.is_text_entered() && !special_input) {
			this->add_text_input(event.u32_text_entered());
		}
	}

	void update(const qsf::event_info& event) {
		event.update(this->scroll_bar);

		if (this->scroll_bar.value_changed) {
			this->view_row = this->scroll_bar.get_progress_step();
			this->scroll_bar_transition_start = this->scroll_bar_transition_end = this->scroll_bar.visible_knob_progress;
			this->clamp_view_y();
		}
		if (this->scroll_bar.released_dragging) {
			this->scroll_bar_transition_start = this->scroll_bar_transition_end = this->scroll_bar.visible_knob_progress;
			this->prepare_scroll();
		}
		if (this->scroll_bar.clicked_on_background_above) {
			--this->view_row;
			this->prepare_scroll();
		}
		if (this->scroll_bar.clicked_on_background_below) {
			++this->view_row;
			this->prepare_scroll();
		}

		if (this->visible_rows) {
			if (event.key_holding(sf::Keyboard::LControl)) {
				if (event.scrolled_down()) {
					if (this->text.character_size > 2) {
						this->end_animation();

						this->text.character_size--;
						this->handle_zoom();
						++this->zooms;
					}

				}
				if (event.scrolled_up()) {
					this->end_animation();

					this->text.character_size++;
					this->handle_zoom();
					--this->zooms;
				}
			}
			else {
				if (event.scrolled_down()) {
					this->view_row += qpl::max(1ll, (3 + this->zooms));
					this->prepare_scroll();
				}
				if (event.scrolled_up()) {
					this->view_row -= qpl::max(1ll, (3 + this->zooms));
					this->prepare_scroll();
				}
			}
		}

		this->scroll_transition_animation.update(event);
		if (this->scroll_transition_animation.is_running()) {
			auto p = this->scroll_transition_animation.get_progress();
			auto curve = qpl::smooth_slope(p);
			this->view.position.y = qpl::linear_interpolation(this->scroll_transition_start, this->scroll_transition_end, curve);

			if (!this->scroll_bar.dragging) {
				this->scroll_bar.set_visible_knob_progress(qpl::linear_interpolation(this->scroll_bar_transition_start, this->scroll_bar_transition_end, curve));
			}
		}
		if (this->scroll_transition_animation.just_finished()) {
			this->scroll_transition_start = this->scroll_transition_end;
			this->scroll_bar_transition_start = this->scroll_bar_transition_end;
		}

		this->update_text_input(event);
	}


	void add(const qpl::styled_string<qpl::u32_string>& string) {
		qpl::clock clock;
		this->text.add(string);
		this->string << string;

		qpl::colored_string cs;
		cs.create_from_styled_string(string);

		this->update_visible_rows_count();
		this->scroll_bar.set_progress(this->view_row / qpl::f64_cast(this->scroll_bar.integer_step));
		this->update_cursor_position();

		qpl::println("Elapsed = ", clock.elapsed().small_descriptive_string());
		auto size = this->text.vertices.size() + this->text.outline_vertices.size();
	}
	void create(const qpl::styled_string<qpl::u32_string>& string) {
		this->text.clear();
		this->string.clear();
		this->add(string);
		this->update_cursor_position(true);
	}
	void add_random() {
		auto dim = qpl::vec(80, 100);

		qpl::styled_string<qpl::u32_string> new_string;

		for (qpl::size i = 0u; i < dim.y; ++i) {
			if (qpl::random_b(0.1)) {
				new_string << qpl::get_random_color();
			}
			new_string << qpl::get_random_lowercase_uppercase_number_string(dim.x) << '\n';
		}
		this->add(new_string);
	}

	void draw(qsf::draw_object& draw) const {
		//qsf::rectangle rect;
		//rect.set_color(qpl::rgb(10, 15, 250));
		//rect.set_hitbox(this->text.hitbox);
		//draw.draw(rect, this->view);

		if (this->accept_input) {
			auto time = this->cursor_blink_timer.elapsed_f();
			if (std::fmod(time, this->cursor_interval_duration) < this->cursor_interval_duration / 2) {
				draw.draw(this->cursor, this->view);
			}
		}
		draw.draw(this->text, this->view);
		draw.draw(this->scroll_bar);
	}
};
