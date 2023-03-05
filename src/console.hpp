#pragma once

#include <qpl/qpl.hpp>
#include "Text.hpp"

struct scroll_bar {
	qsf::smooth_rectangle background;
	qsf::smooth_rectangle knob;

	qpl::hitbox hitbox;

	qpl::f32 knob_progress = 0.0f;
	qpl::f32 knob_range = 1.0f;
	qpl::f32 visible_knob_progress = 0.0f;
	qpl::size integer_step = qpl::size_max;

	qpl::animation background_hover_animation;
	qpl::rgb background_color = qpl::rgb::grey_shade(24);
	qpl::rgb background_color_hover = qpl::rgb::grey_shade(30);

	qpl::animation hover_animation;
	qpl::rgb knob_color = qpl::rgb::grey_shade(141);
	qpl::rgb knob_color_hover = qpl::rgb::grey_shade(200);

	bool hovering = false;
	bool hovering_background = false;
	bool dragging = false;
	bool clicked_on_background_below = false;
	bool clicked_on_background_above = false;
	bool released_dragging = false;
	bool value_changed = false;
	qpl::vec2f dragging_position;

	scroll_bar() {
		this->background.set_color(this->background_color);
		this->knob.set_color(this->knob_color);
		this->background.set_slope_dimension(5);
		this->knob.set_slope_dimension(5);
		this->hover_animation.set_duration(0.2);
		this->background_hover_animation.set_duration(0.2);
	}

	void set_dimension(qpl::vec2f dimension) {
		this->hitbox.dimension = dimension;
		this->update_positions();
	}
	void set_position(qpl::vec2f position) {
		this->hitbox.position = position;
		this->update_positions();
	}
	void set_hitbox(qpl::hitbox hitbox) {
		this->hitbox = hitbox;
		this->update_positions();
	}

	void set_visible_knob_progress(qpl::f64 progress) {
		this->visible_knob_progress = progress;
		auto height = this->hitbox.get_height() - this->knob.get_dimension().y;
		this->knob.set_position(this->hitbox.position + qpl::vec2f(0.0f, height * this->visible_knob_progress));
	}
	void update_knob() {
		//auto progress = this->stepped_progress();
		auto progress = this->stepped_progress_unless_dragging();
		this->knob.set_dimension(qpl::vec2f(this->hitbox.get_width(), this->hitbox.get_height() * this->knob_range));
		this->set_visible_knob_progress(progress);
	}
	void update_positions() {
		this->background.set_dimension(this->hitbox.dimension);
		this->background.set_position(this->hitbox.position);
		this->update_knob();
	}

	qpl::f32 stepped_progress() const {
		qpl::f32 progress = this->knob_progress;
		if (this->integer_step != qpl::size_max) {
			progress = std::round(this->knob_progress * this->integer_step) / (this->integer_step);
		}
		return progress;
	}
	qpl::f32 stepped_progress_unless_dragging() const {
		if (this->dragging) {
			return this->knob_progress;
		}
		else {
			return this->stepped_progress();
		}
	}
	qpl::f32 get_progress() const {
		return this->knob_progress;
	}
	qpl::size get_progress_step() const {
		//return qpl::size_cast(std::ceil(this->knob_progress * this->integer_step));
		return qpl::size_cast(std::round(this->knob_progress * (this->integer_step)));
	}

	void set_progress_integer_step(qpl::size step, qpl::size knob_steps) {
		this->knob_range = qpl::f64_cast(knob_steps) / step;
		this->integer_step = step - knob_steps;
	}
	void set_progress(qpl::f32 progress) {
		this->knob_progress = qpl::clamp_0_1(progress);
		this->update_knob();
	}
	void set_knob_height(qpl::f32 height) {
		this->knob_range = height;
		this->update_knob();
	}

	void set_knob_range(qpl::f32 range) {
		this->knob_range = range;
	}

	void update_hover(const qsf::event_info& event) {

		auto hitbox = this->knob.get_hitbox().increased(5);

		this->hovering = hitbox.contains(event.mouse_position());

		if (this->hovering && event.left_mouse_clicked()) {
			this->dragging_position = event.mouse_position();
			this->dragging = true;
		}

		if (this->hovering || this->dragging) {
			this->hover_animation.go_forwards();
		}
		else {
			this->hover_animation.go_backwards();
		}
		this->hover_animation.update(event);
		if (this->hover_animation.is_running()) {
			auto p = this->hover_animation.get_progress();
			auto curve = qpl::smooth_slope(p);
			this->knob.set_color(this->knob_color.interpolated(this->knob_color_hover, curve));
		}
	}
	void update_background_hover(const qsf::event_info& event) {
		this->hovering_background = !this->hovering && this->hitbox.increased(5).contains(event.mouse_position());
		if (this->hovering_background && event.left_mouse_clicked()) {
			if (event.mouse_position().y < this->knob.get_position().y) {
				this->clicked_on_background_above = true;
			}
			else {
				this->clicked_on_background_below = true;
			}
		}

		if (this->hovering_background) {
			this->background_hover_animation.go_forwards();
		}
		else {
			this->background_hover_animation.go_backwards();
		}
		this->background_hover_animation.update(event);
		if (this->background_hover_animation.is_running()) {
			auto p = this->background_hover_animation.get_progress();
			auto curve = qpl::smooth_slope(p);
			this->background.set_color(this->background_color.interpolated(this->background_color_hover, curve));
		}
	}
	void update(const qsf::event_info& event) {
		this->value_changed = false;
		this->released_dragging = false;
		this->clicked_on_background_above = false;
		this->clicked_on_background_below = false;
		if (this->knob_range >= 1.0f) {
			return;
		}

		this->update_hover(event);
		this->update_background_hover(event);

		if (this->dragging) {
			auto delta = event.mouse_position().y - this->dragging_position.y;

			auto height = this->hitbox.dimension.y - this->knob.get_dimension().y;
			auto percentage = delta / height;

			auto before = this->stepped_progress();
			this->knob_progress = qpl::clamp_0_1(this->knob_progress + percentage);

			this->update_knob();
			if (before != this->stepped_progress()) {
				this->value_changed = true;
			}

			this->dragging_position = event.mouse_position();
		}
		if (event.left_mouse_released()) {
			if (this->dragging) {
				this->released_dragging = true;
			}
			this->dragging = false;
		}
	}
	void draw(qsf::draw_object& draw) const {
		if (this->knob_range >= 1.0f) {
			return;
		}
		draw.draw(this->background);
		draw.draw(this->knob);
	}
};

struct console {
	text text;
	qpl::styled_string<std::basic_string<qpl::u32>> string;
	sf::Font font;
	qsf::view view;
	qpl::vec2f screen_dimension;
	qpl::isize zooms = 0;
	qpl::isize view_row = 0;
	qpl::size visible_rows = 0u;
	scroll_bar scroll_bar;

	qpl::animation scroll_transition_animation;
	qpl::f64 scroll_transition_start = 0.0;
	qpl::f64 scroll_transition_end = 0.0;
	qpl::f64 scroll_bar_transition_start = 0.0;
	qpl::f64 scroll_bar_transition_end = 0.0;

	console() {
		//this->text.set_line_spacing(1.2);
		this->scroll_transition_animation.set_duration(0.4);
		this->view.position.x = -5.0f;
		this->text.character_size = 18u;
	}

	void load_font(std::string path) {
		this->font.loadFromFile(path);
		this->text.set_font(this->font);
		this->text.set_character_size(20u);
	}

	void clamp_view_y(bool transition = true) {
		if (transition) {
			this->scroll_transition_start = this->view.position.y;
		}

		auto max_rows = qpl::isize_cast((this->screen_dimension.y * this->view.scale.y) / this->text.get_line_spacing_pixels());
		//this->view_row = qpl::clamp(0ll, this->view_row, qpl::max(0ll, qpl::signed_cast(this->text.rows) - max_rows));
		this->view_row = qpl::max(0ll, this->view_row);

		if (transition) {
			this->scroll_transition_end = this->view_row * this->text.get_line_spacing_pixels();
			this->scroll_transition_animation.reset_and_start();
		}
	}
	void set_dimension(qpl::vec2f dimension) {
		this->screen_dimension = dimension;
		auto width = 30.0f;
		auto margin = 5.0f;
		this->scroll_bar.set_position({ this->screen_dimension.x - (width + margin), margin });
		this->scroll_bar.set_dimension(qpl::vec2f(width, this->screen_dimension.y - margin * 2));
	}

	void prepare_scroll() {
		this->scroll_bar_transition_start = this->scroll_bar.visible_knob_progress;
		this->scroll_bar.set_progress(this->view_row / qpl::f64_cast(this->text.rows - this->visible_rows));
		this->scroll_bar_transition_end = this->scroll_bar.get_progress();

		this->clamp_view_y();
	}

	void update_visible_rows_count() {
		this->visible_rows = std::ceil(this->screen_dimension.y / this->text.get_line_spacing_pixels());
		this->scroll_bar.set_progress_integer_step(this->text.rows, this->visible_rows);
	}
	void end_animation() {
		this->scroll_transition_start = this->scroll_transition_end;
		this->view.position.y = this->scroll_transition_end;
		this->scroll_bar.set_visible_knob_progress(this->scroll_bar_transition_end);
		this->scroll_transition_animation.reset();
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
						this->text.create(this->string);

						this->clamp_view_y(false);
						this->update_visible_rows_count();
						this->scroll_bar.set_progress(this->view_row / qpl::f64_cast(this->text.rows - this->visible_rows));
						this->scroll_bar.set_visible_knob_progress(this->view_row / qpl::f64_cast(this->scroll_bar.integer_step));
						this->view.position.y = this->view_row * this->text.get_line_spacing_pixels();
						++this->zooms;
					}

				}
				if (event.scrolled_up()) {
					this->end_animation();

					this->text.character_size++;
					this->text.create(this->string);

					this->clamp_view_y(false);
					this->update_visible_rows_count();
					this->scroll_bar.set_progress(this->view_row / qpl::f64_cast(this->text.rows - this->visible_rows));
					this->scroll_bar.set_visible_knob_progress(this->view_row / qpl::f64_cast(this->scroll_bar.integer_step));
					this->view.position.y = this->view_row * this->text.get_line_spacing_pixels();
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
	}


	void add(const qpl::styled_string<std::basic_string<qpl::u32>>& string) {
		qpl::clock clock;
		this->text.add(string);
		this->string << string;

		qpl::colored_string cs;
		cs.create_from_styled_string(string);

		this->update_visible_rows_count();
		this->scroll_bar.set_progress(this->view_row / qpl::f64_cast(this->scroll_bar.integer_step));

		qpl::println("Elapsed = ", clock.elapsed().small_descriptive_string());
		auto size = this->text.vertices.size() + this->text.outline_vertices.size();
	}
	void create(const qpl::styled_string<std::basic_string<qpl::u32>>& string) {
		this->text.clear();
		this->string.clear();
		this->add(string);
	}
	void add_random() {
		auto dim = qpl::vec(80, 10);

		qpl::styled_string<std::basic_string<qpl::u32>> new_string;

		for (qpl::size i = 0u; i < dim.y; ++i) {
			if (qpl::random_b(0.1)) {
				new_string << qpl::get_random_color();
			}
			new_string << qpl::get_random_lowercase_uppercase_number_string(dim.x) << '\n';
		}
		this->create(new_string);
	}

	void draw(qsf::draw_object& draw) const {
		//qsf::rectangle rect;
		//rect.set_color(qpl::rgb(10, 15, 250));
		//rect.set_hitbox(this->text.hitbox);
		//draw.draw(rect, this->view);

		draw.draw(this->text, this->view);
		draw.draw(this->scroll_bar);
	}
};
