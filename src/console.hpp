#pragma once

#include <qpl/qpl.hpp>
#include "Text.hpp"
#include "scroll_bar.hpp"

struct console {
	text text;
	qpl::styled_string<qpl::u32_string> string;
	qpl::styled_string<qpl::u32_string> input_string;
	std::vector<std::wstring> input_string_split;
	std::vector<std::wstring> string_split;
	std::vector<std::wstring> string_and_input_split;

	qpl::vec2is selection_rectangle_start;
	qpl::vec2is selection_rectangle_end;

	std::vector<qsf::rectangle> selection_rangles;

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
	qpl::size before_input_text_rows;
	qpl::vec2s cursor_position;
	bool accept_input = false;
	bool enter_pressed = false;
	bool text_entered = false;
	bool dragging = false;
	qpl::vec2f clicked_mouse_position;

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
		this->character_size.x = this->text.get_white_space_width();
	}

	void track_before_input_values() {
		this->before_input_vertices_size = this->text.vertices.size();
		this->before_input_outline_vertices_size = this->text.outline_vertices.size();
		this->before_input_text_position = this->text.text_position;
		this->before_input_text_rows = this->text.rows;
	}
	void start_accepting_input() {
		if (this->accept_input) {
			return;
		}
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

	void update_string_and_input_split() {
		this->string_and_input_split = this->string_split;
		this->string_and_input_split.reserve(this->string_split.size() + this->input_string_split.size());
		for (auto& i : this->input_string_split) {
			this->string_and_input_split.push_back(i);
		}
	}
	void update_string_split() {
		this->string_split = qpl::string_split_allow_empty(qpl::to_basic_string<wchar_t>(this->string.string()), L'\n');
	}
	void update_input_string_split() {
		this->input_string_split = qpl::string_split_allow_empty(qpl::to_basic_string<wchar_t>(this->input_string.string()), L'\n');
		this->update_string_and_input_split();
	}
	void update_cursor_position(bool reset_timer = false) {
		auto cursor_pos_with_tabs = this->cursor_position;
		if (this->cursor_position.y < this->input_string_split.size()) {
			auto count = qpl::count(this->input_string_split[this->cursor_position.y].substr(0u, this->cursor_position.x), L'\t');
			cursor_pos_with_tabs.x += count * 3u;
		}
		auto off = cursor_pos_with_tabs * this->character_size;
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
		this->text.rows = this->before_input_text_rows;
		this->text.add(this->input_string);
		this->update_cursor_position(false);
	}
	void add_text_input(const qpl::u32_string& string) {

		if (string == qpl::to_u32_string(U'\t')) {
			qpl::println("tab");
		}
		auto count = qpl::count(qpl::to_basic_string<wchar_t>(string), L'\n');
		if (count) {
			auto split = qpl::string_split_allow_empty(qpl::to_basic_string<wchar_t>(string), L'\n');
			this->cursor_position.y += count;
			if (split.size()) {
				this->cursor_position.x = split.back().length();
			}
			else {
				this->cursor_position.x = 0u;
			}
		}
		else {
			this->cursor_position.x += string.length();
		}

		this->input_string << string;
		this->update_input_text_graphics();
		this->update_input_string_split();

		this->update_cursor_position(true);
	}
	void pop_last_character() {
		if (this->input_string.size() && this->input_string.elements.back().text.length()) {
			this->input_string.elements.back().text.pop_back();
			this->update_input_string_split();
			this->update_input_text_graphics();

			if (this->cursor_position.x) {
				--this->cursor_position.x;
				this->update_cursor_position(true);
			}
			else if (this->cursor_position.y) {
				--this->cursor_position.y;
				this->cursor_position.x = this->get_input_text_width(this->cursor_position.y);
				this->update_cursor_position(true);
			}
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


	qpl::size get_text_height() const {
		return this->string_and_input_split.size();
	}
	qpl::size get_text_width(qpl::size y) const {
		if (y >= this->string_and_input_split.size()) {
			return 0u;
		}
		return this->string_and_input_split[y].length();
	}
	qpl::size get_input_text_width(qpl::size y) const {
		if (y >= this->input_string_split.size()) {
			return 0u;
		}
		return this->input_string_split[y].length();
	}
	qpl::size get_input_text_height() const {
		return this->input_string_split.size();
	}


	void update_text_input(const qsf::event_info& event) {
		this->enter_pressed = false;
		this->text_entered = false;
		if (!this->accept_input) {
			return;
		}
		bool special_input = false;

		if (event.key_holding(sf::Keyboard::LControl)) {
			if (event.key_pressed(sf::Keyboard::V)) {
				this->add_text_input(qpl::to_u32_string(qsf::copy_from_clipboard()));
				special_input = true;
				this->text_entered = true;
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
			this->text_entered = true;
		}
		if (event.key_pressed(sf::Keyboard::Right)) {
			if (this->cursor_position.x < this->get_input_text_width(this->cursor_position.y)) {
				++this->cursor_position.x;
				this->update_cursor_position(true);
			}
			else if (this->cursor_position.y < this->get_input_text_height() - 1) {
				++this->cursor_position.y;
				this->cursor_position.x = 0u;
				this->update_cursor_position(true);
			}
		}
		if (event.key_pressed(sf::Keyboard::Left)) {
			if (this->cursor_position.x) {
				--this->cursor_position.x;
				this->update_cursor_position(true);
			}
			else if (this->cursor_position.y) {
				--this->cursor_position.y;
				this->cursor_position.x = this->get_input_text_width(this->cursor_position.y);
				this->update_cursor_position(true);
			}
		}
		if (event.key_pressed(sf::Keyboard::Up)) {
			if (this->get_input_text_height() <= 1u) {
				//todo add select last command
			}
			else if (this->cursor_position.y) {
				--this->cursor_position.y;
				this->cursor_position.x = qpl::min(this->cursor_position.x, this->get_input_text_width(this->cursor_position.y));
				this->update_cursor_position(true);
			}
		}
		if (event.key_pressed(sf::Keyboard::Down)) {
			if (this->get_input_text_height() <= 1u) {
				this->input_string.clear();
				this->update_input_text_graphics();
				special_input = true;
			}
			else if (this->cursor_position.y < this->get_input_text_height() - 1) {
				++this->cursor_position.y;
				this->cursor_position.x = qpl::min(this->cursor_position.x, this->get_input_text_width(this->cursor_position.y));
				this->update_cursor_position(true);
			}
		}
		if (event.is_text_entered() && !special_input) {
			this->add_text_input(event.u32_text_entered());
			this->text_entered = true;
		}
	}

	qpl::vec2s position_to_text_position(qpl::vec2f position) const {
		return qpl::vec2s(position / this->character_size);
	}
	qpl::vec2s position_to_text_position_ceil(qpl::vec2f position) const {
		return qpl::vec2s((position / this->character_size).ceil());
	}

	void make_selection_rectangles() {
		auto min = this->selection_rectangle_start;
		auto max = this->selection_rectangle_end;

		if (max.y < min.y) {
			std::swap(min, max);
		}
		else if (max.x < min.x) {
			std::swap(min, max);
		}
		min.y = qpl::max(0ll, min.y);
		max.y = qpl::min(max.y, qpl::signed_cast(this->get_text_height()));

		auto size = qpl::max(0ll, (max.y - min.y) + 1);
		this->selection_rangles.resize(size);
		for (qpl::isize i = min.y + 1; i < max.y - 1; ++i) {
			qpl::println("i = ", i);

			auto index = i - min.y;
			if (index >= this->selection_rangles.size()) {
				qpl::println("out of bounds");
				qpl::system_pause();
			}
			auto pos = qpl::vec2s(0ull, i);
			this->selection_rangles[index].set_position(pos * this->character_size);
			this->selection_rangles[index].set_dimension(qpl::vec(this->get_text_width(i) * this->text.get_white_space_width(), this->text.get_line_spacing_pixels()));
			this->selection_rangles[index].set_color(qpl::rgb::cyan().with_alpha(50));

			qpl::println("hitbox = ", this->selection_rangles[index].get_hitbox());
		}
	}

	void update_selection_rectangle(const qsf::event_info& event) {
		if (event.left_mouse_clicked()) {
			this->clicked_mouse_position = event.mouse_position();
			this->dragging = true;

			this->selection_rectangle_start = this->position_to_text_position(event.mouse_position() + this->view.position);
		}
		if (this->dragging) {
			this->selection_rectangle_end = this->position_to_text_position_ceil(event.mouse_position() + this->view.position);
			this->make_selection_rectangles();
		}

		if (event.left_mouse_released()) {
			this->dragging = false;
		}
	}
	void update_cursor() {

		auto f = this->cursor_blink_timer.elapsed_f();
		qpl::f64 progress = 1.0;
		if (f > 0.5) {
			auto time = (f - 0.5) / this->cursor_interval_duration;
			progress = 1.0 - qpl::triangle_progression(std::fmod(time, 1.0));
		}
		this->cursor.set_color(this->cursor.get_color().with_alpha(progress * 255));
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

		if (this->text_entered) {
			auto input_pos = qpl::isize_cast(this->text.rows - this->visible_rows) + 1;
			auto input_pos_end = qpl::isize_cast(this->text.rows);
			if (this->view_row < input_pos) {
				this->view_row = input_pos;
				this->prepare_scroll();
			}
			if (this->view_row > input_pos_end) {
				this->view_row = input_pos_end;
				this->prepare_scroll();
			}
		}

		this->update_cursor();
		this->update_selection_rectangle(event);
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
		this->update_string_split();
		this->update_string_and_input_split();
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
			new_string << qpl::get_random_lowercase_uppercase_number_string(qpl::random(dim.x / 2.0, dim.x * 1.5)) << '\n';
		}
		this->add(new_string);
	}

	void draw(qsf::draw_object& draw) const {
		//qsf::rectangle rect;
		//rect.set_color(qpl::rgb(10, 15, 250));
		//rect.set_hitbox(this->text.hitbox);
		//draw.draw(rect, this->view);

		if (this->accept_input) {
			draw.draw(this->cursor, this->view);
		}
		draw.draw(this->text, this->view);
		draw.draw(this->selection_rangles, this->view);
		draw.draw(this->scroll_bar);
	}
};
