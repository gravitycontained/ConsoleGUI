
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



// Headers

#include <algorithm>
#include <cmath>

#include "Text.hpp"

namespace {
    void addLine(qsf::vertex_array& vertices,
                 qpl::f32 line_length,
                 qpl::f32 lineTop,
                 const qpl::rgba& color,
                 qpl::f32 offset,
                 qpl::f32 thickness,
                 qpl::f32 outline_thickness = 0) {
        qpl::f32 top = std::floor(lineTop + offset - (thickness / 2) + 0.5f);
        qpl::f32 bottom = top + std::floor(thickness + 0.5f);

        vertices.add(qsf::vertex(qpl::vec2f(-outline_thickness, top - outline_thickness), color, qpl::vec2f(1, 1)));
        vertices.add(qsf::vertex(qpl::vec2f(line_length + outline_thickness, top - outline_thickness), color, qpl::vec2f(1, 1)));
        vertices.add(qsf::vertex(qpl::vec2f(-outline_thickness, bottom + outline_thickness), color, qpl::vec2f(1, 1)));
        vertices.add(qsf::vertex(qpl::vec2f(-outline_thickness, bottom + outline_thickness), color, qpl::vec2f(1, 1)));
        vertices.add(qsf::vertex(qpl::vec2f(line_length + outline_thickness, top - outline_thickness), color, qpl::vec2f(1, 1)));
        vertices.add(qsf::vertex(qpl::vec2f(line_length + outline_thickness, bottom + outline_thickness), color, qpl::vec2f(1, 1)));
    }

    
    glyph_quad get_glyph_quad(const sf::Glyph& glyph, qpl::f32 italic_shear) {
        qpl::f32 padding = 1.0;

        qpl::f32 left = glyph.bounds.left - padding;
        qpl::f32 top = glyph.bounds.top - padding;
        qpl::f32 right = glyph.bounds.left + glyph.bounds.width + padding;
        qpl::f32 bottom = glyph.bounds.top + glyph.bounds.height + padding;

        qpl::f32 u1 = qpl::f32_cast(glyph.textureRect.left) - padding;
        qpl::f32 v1 = qpl::f32_cast(glyph.textureRect.top) - padding;
        qpl::f32 u2 = qpl::f32_cast(glyph.textureRect.left + glyph.textureRect.width) + padding;
        qpl::f32 v2 = qpl::f32_cast(glyph.textureRect.top + glyph.textureRect.height) + padding;

        qpl::size ctr = 0u;
        glyph_quad result;
        result[0u] = glyph_quad_vertex{ qpl::vec2f(left - italic_shear * top, top), qpl::vec2f(u1, v1) };
        result[1u] = glyph_quad_vertex{ qpl::vec2f(right - italic_shear * top, top), qpl::vec2f(u2, v1) };
        result[2u] = glyph_quad_vertex{ qpl::vec2f(left - italic_shear * bottom, bottom), qpl::vec2f(u1, v2) };
        result[3u] = glyph_quad_vertex{ qpl::vec2f(left - italic_shear * bottom, bottom), qpl::vec2f(u1, v2) };
        result[4u] = glyph_quad_vertex{ qpl::vec2f(right - italic_shear * top, top), qpl::vec2f(u2, v1) };
        result[5u] = glyph_quad_vertex{ qpl::vec2f(right - italic_shear * bottom, bottom), qpl::vec2f(u2, v2) };
        return result;
    }

    // Add a glyph quad to the vertex array
    void add_glyph_quad(qsf::vertex_array& vertices, qpl::vec2f position, const qpl::rgba& color, const sf::Glyph& glyph, qpl::f32 italic_shear) {
        auto quad = get_glyph_quad(glyph, italic_shear);

        for (auto& i : quad) {
            qsf::vertex vertex;
            vertex.position = i.position + position;
            vertex.tex_coords = i.tex_coord;
            vertex.color = color;
            vertices.add(vertex);
        }
    }
}

text::text() {
    this->clear();
}
text::text(const text&) = default;
text& text::operator=(const text&) = default;
text::text(text&&) noexcept = default;
text& text::operator=(text&&) noexcept = default;

void text::set_font(const sf::Font& font) {
    this->font = &font;
}

void text::set_character_size(qpl::u32 size) {
    if (this->character_size != size) {
        this->character_size = size;
    }
}
void text::set_letter_spacing(qpl::f32 spacingFactor) {
    if (this->letter_spacing_factor != spacingFactor) {
        this->letter_spacing_factor = spacingFactor;
    }
}
void text::set_line_spacing(qpl::f32 spacingFactor) {
    if (this->line_spacing_factor != spacingFactor) {
        this->line_spacing_factor = spacingFactor;
    }
}
void text::set_style(qpl::u32 style) {
    if (this->style != style) {
        this->style = style;
    }
}
qpl::u32 text::get_character_size() const {
    return this->character_size;
}
qpl::f32 text::get_line_spacing() const {
    return this->line_spacing_factor;
}
qpl::f32 text::get_line_spacing_pixels() const {
    return this->font->getLineSpacing(this->character_size) * this->line_spacing_factor;
}
qpl::f32 text::get_white_space_width() const {
    auto whitespace_width = this->font->getGlyph(U' ', this->character_size, false).advance;
    auto letter_spacing = (whitespace_width / 3.f) * (this->letter_spacing_factor - 1.f);
    return whitespace_width + letter_spacing;
}
qpl::u32 text::get_style() const {
    return this->style;
}
void text::clear() {
    this->vertices.clear();
    this->outline_vertices.clear();
    this->text_position.x = 0.f;
    this->text_position.y = 0.f;
    this->rows = 0u;
}
void text::pop_last_character() {
    if (this->last_element.style & sf::Text::Style::Bold) {
        this->outline_vertices.resize(qpl::max(0ll, qpl::signed_cast(this->outline_vertices.size()) - 6));
    }
    this->vertices.resize(qpl::max(0ll, qpl::signed_cast(this->vertices.size()) - 6));
}
void text::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (this->font) {

        sf::RenderStates statesCopy(states);

        statesCopy.texture = &this->font->getTexture(this->character_size);

        if (this->outline_vertices.size()) {
            this->outline_vertices.draw(target, statesCopy);
        }

        this->vertices.draw(target, statesCopy);
    }
}
void text::create(const qpl::styled_string<qpl::u32_string>& string) {
    this->clear();
    this->add(string);
}

void text::add(const qpl::styled_string<qpl::u32_string>& string) {
    if (!this->font) {
        return;
    }

    this->hitbox = qpl::hitbox{};

    if (string.empty()) {
        return;
    }
    auto underline_offset = this->font->getUnderlinePosition(this->character_size);
    auto underline_thickness = this->font->getUnderlineThickness(this->character_size);


    auto line_spacing = this->get_line_spacing_pixels();
    
    if (this->text_position.y == 0.f) {
        this->text_position.y = this->font->getLineSpacing(this->character_size);
    }

    auto minX = qpl::f32_cast(this->character_size);
    auto minY = 0.f;
    auto maxX = 0.f;
    auto maxY = 0.f;
    qpl::u32 previous = 0;

    this->last_element.copy_style(string.elements.back());
    for (auto& element : string) {
        bool is_bold = element.style & sf::Text::Style::Bold;
        bool is_underlined = element.style & sf::Text::Style::Underlined;
        bool is_strike_through = element.style & sf::Text::Style::StrikeThrough;
        auto italic_shear = (element.style & sf::Text::Style::Italic) ? 0.2094395102393f : 0.f;
        sf::FloatRect default_bounds = this->font->getGlyph(U'x', this->character_size, is_bold).bounds;
        auto strike_through_offset = default_bounds.top + default_bounds.height / 2.f;
        auto whitespace_width = this->font->getGlyph(U' ', this->character_size, is_bold).advance;
        auto letter_spacing = (whitespace_width / 3.f) * (this->letter_spacing_factor - 1.f);
        whitespace_width += letter_spacing;

        for (qpl::size i = 0u; i < element.text.length(); ++i) {
            qpl::u32 c = element.text[i];

            //ignore \r
            if (c == U'\r') {
                continue;
            }

            // Apply the kerning offset
            this->text_position.x += this->font->getKerning(previous, c, this->character_size);

            // If we're using the underlined style and there's a new line, draw a line
            if (is_underlined && (c == U'\n' && previous != U'\n')) {
                addLine(this->vertices, this->text_position.x, this->text_position.y, element.color, underline_offset, underline_thickness);

                if (element.outline_thickness != 0) {
                    addLine(this->outline_vertices, this->text_position.x, this->text_position.y, element.outline_color, underline_offset, underline_thickness, element.outline_thickness);
                }
            }

            // If we're using the strike through style and there's a new line, draw a line across all characters
            if (is_strike_through && (c == U'\n' && previous != U'\n')) {
                addLine(this->vertices, this->text_position.x, this->text_position.y, element.color, strike_through_offset, underline_thickness);

                if (element.outline_thickness != 0) {
                    addLine(this->outline_vertices, this->text_position.x, this->text_position.y, element.outline_color, strike_through_offset, underline_thickness, element.outline_thickness);
                }
            }

            previous = c;

            if ((c == U' ') || (c == U'\n') || (c == U'\t')) {
                minX = std::min(minX, this->text_position.x);
                minY = std::min(minY, this->text_position.y);

                switch (c) {
                case U' ':
                    this->text_position.x += whitespace_width;
                    break;
                case U'\t':
                    this->text_position.x += whitespace_width * 4;
                    break;
                case U'\n':
                    this->text_position.y += line_spacing;
                    this->text_position.x = 0;
                    ++this->rows;
                    break;
                }

                maxX = std::max(maxX, this->text_position.x);
                maxY = std::max(maxY, this->text_position.y);
                continue;
            }

            if (element.outline_thickness != 0) {
                const sf::Glyph& glyph = this->font->getGlyph(c, this->character_size, is_bold, element.outline_thickness);
                add_glyph_quad(this->outline_vertices, this->text_position, element.outline_color, glyph, italic_shear);
            }


            const sf::Glyph& glyph = this->font->getGlyph(c, this->character_size, is_bold);
            add_glyph_quad(this->vertices, this->text_position, element.color, glyph, italic_shear);

            qpl::f32 left = glyph.bounds.left;
            qpl::f32 top = glyph.bounds.top;
            qpl::f32 right = glyph.bounds.left + glyph.bounds.width;
            qpl::f32 bottom = glyph.bounds.top + glyph.bounds.height;

            minX = std::min(minX, this->text_position.x + left - italic_shear * bottom);
            maxX = std::max(maxX, this->text_position.x + right - italic_shear * top);
            minY = std::min(minY, this->text_position.y + top);
            maxY = std::max(maxY, this->text_position.y + bottom);

            this->text_position.x += glyph.advance + letter_spacing;
        }
    }
    
    //if (this->outline_vertices.size()) {
    //    qpl::f32 outline = std::abs(std::ceil(element.outline_thickness));
    //    minX -= outline;
    //    maxX += outline;
    //    minY -= outline;
    //    maxY += outline;
    //}

    //if (is_underlined && (x > 0)) {
    //    addLine(this->vertices, x, y, element.color, underline_offset, underline_thickness);
    //
    //    if (element.outline_thickness != 0) {
    //        addLine(this->outline_vertices, x, y, element.outline_color, underline_offset, underline_thickness, element.outline_thickness);
    //    }
    //}
    //if (is_strike_through && (x > 0)) {
    //    addLine(this->vertices, x, y, element.color, strike_through_offset, underline_thickness);
    //
    //    if (element.outline_thickness != 0) {
    //        addLine(this->outline_vertices, x, y, element.outline_color, strike_through_offset, underline_thickness, element.outline_thickness);
    //    }
    //}

    this->hitbox.position.x = minX;
    this->hitbox.position.y = 0;
    this->hitbox.dimension.x = maxX - minX;
    this->hitbox.dimension.y = maxY - minY;
}