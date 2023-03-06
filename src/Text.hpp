//This text is a modified version of SFML/Graphics/Text.hpp


////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2023 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <qpl/qpl.hpp>


struct glyph_quad_vertex {
    qpl::vec2f position;
    qpl::vec2f tex_coord;
};

using glyph_quad = std::array<glyph_quad_vertex, 6u>;

struct text {

    text();
    text(const text&);
    text& operator=(const text&);
    text(text&&) noexcept;
    text& operator=(text&&) noexcept;

    void set_font(const sf::Font& font);
    void set_font(sf::Font&& font) = delete;
    void set_character_size(qpl::u32 size);
    void set_line_spacing(qpl::f32 spacingFactor);
    void set_letter_spacing(qpl::f32 spacingFactor);
    void set_style(qpl::u32 style);
    qpl::u32 get_character_size() const;
    qpl::f32 get_line_spacing() const;
    qpl::f32 get_line_spacing_pixels() const;
    qpl::u32 get_style() const;
    qpl::f32 get_outline_thickness() const;

    void draw(sf::RenderTarget& target, sf::RenderStates states) const;
    void add(const qpl::styled_string<qpl::u32_string>& string);
    void create(const qpl::styled_string<qpl::u32_string>& string);
    void clear();

    void pop_last_character();

    template<typename T>
    text& operator<<(const T& value) {
        qpl::styled_string<qpl::u32_string> string;
        string.clear_copy_style(this->last_element);
        string.elements[0u].text = qpl::to_u32_string(value);
        this->add(string);
        return *this;
    }

    //std::unordered_map<sf::Glyph, std::array<qsf::vertex, 6u>> glyph_vertices;

    const sf::Font* font{};
    qpl::styled_string<qpl::u32_string>::element last_element;
    qpl::u32 character_size{ 30 };
    qpl::f32 letter_spacing_factor{ 1.f };
    qpl::f32 line_spacing_factor{ 1.f };
    qpl::u32 style{ sf::Text::Style::Regular };
    qpl::rgba fill_color{ qpl::rgba::white()};
    qpl::rgba outline_color{ qpl::rgba::black() };
    qpl::f32 outline_thickness{ 0.f };
    qsf::vertex_array vertices{ sf::PrimitiveType::Triangles };
    qsf::vertex_array outline_vertices{ sf::PrimitiveType::Triangles };
    qpl::hitbox hitbox;
    qpl::vec2f text_position;
    qpl::size rows = 0u;
};