#pragma once

#include <qpl/qpl.hpp>

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
