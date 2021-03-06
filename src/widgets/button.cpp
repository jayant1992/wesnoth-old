/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "global.hpp"

#include "widgets/button.hpp"
#include "game_config.hpp"
#include "font.hpp"
#include "marked-up_text.hpp"
#include "image.hpp"
#include "log.hpp"
#include "serialization/string_utils.hpp"
#include "sound.hpp"
#include "video.hpp"
#include "wml_separators.hpp"

#include "filesystem.hpp"

static lg::log_domain log_display("display");
#define ERR_DP LOG_STREAM(err, log_display)

namespace gui {

const int font_size = font::SIZE_SMALL;
const int horizontal_padding = font::SIZE_SMALL;
const int checkbox_horizontal_padding = font::SIZE_SMALL / 2;
const int vertical_padding = font::SIZE_SMALL / 2;

button::button(CVideo& video, const std::string& label, button::TYPE type,
               std::string button_image_name, SPACE_CONSUMPTION spacing,
               const bool auto_join, std::string overlay_image)
	: widget(video, auto_join), type_(type), label_(label),
	  image_(NULL), pressedImage_(NULL), activeImage_(NULL), pressedActiveImage_(NULL),
	  disabledImage_(NULL), pressedDisabledImage_(NULL),
	  overlayImage_(NULL), overlayPressedImage_(NULL), overlayActiveImage_(NULL),
	  state_(NORMAL), pressed_(false),
	  spacing_(spacing), base_height_(0), base_width_(0),
	  button_image_name_(), button_overlay_image_name_(overlay_image)
{
	if (button_image_name.empty()) {

		switch (type_) {
		case TYPE_PRESS:
			button_image_name_ = "buttons/button_normal/button_H22";
			break;
		case TYPE_TURBO:
			button_image_name_ = "buttons/button_menu/menu_button_copper_H20";
			break;
		case TYPE_CHECK:
			button_image_name_ = "buttons/checkbox";
			break;
		case TYPE_RADIO:
			button_image_name_ = "buttons/radiobox";
			break;
		default:
			break;
		}
	} else {
		button_image_name_ = "buttons/" + button_image_name;
	}

	load_images();
}

void button::load_images() {

	std::string size_postfix;

	switch (location().h) {
		case 25:
			size_postfix = "_25";
			break;
		case 30:
			size_postfix = "_30";
			break;
		case 60:
			size_postfix = "_60";
			break;
		default:
			break;
	}

	surface button_image(image::get_image(button_image_name_ + ".png"));
	surface pressed_image(image::get_image(button_image_name_ + "-pressed.png"));
	surface active_image(image::get_image(button_image_name_ + "-active.png"));
	surface disabled_image;
	if (file_exists(game_config::path + "/images/" + button_image_name_ + "-disabled.png"))
		disabled_image.assign((image::get_image(button_image_name_ + "-disabled.png")));
	surface pressed_disabled_image, pressed_active_image, touched_image;

	if (!button_overlay_image_name_.empty()) {
		overlayImage_.assign(image::get_image(button_overlay_image_name_ + size_postfix + ".png"));
		overlayPressedImage_.assign(image::get_image(button_overlay_image_name_ + size_postfix + "-pressed.png"));

		if (file_exists(game_config::path + "/images/" + button_overlay_image_name_ + size_postfix + "-active.png"))
			overlayActiveImage_.assign(image::get_image(button_overlay_image_name_ + size_postfix + "-active.png"));

		if (file_exists(game_config::path + "/images/" + button_overlay_image_name_ + size_postfix + "-disabled.png"))
			overlayDisabledImage_.assign(image::get_image(button_overlay_image_name_ + size_postfix + "-disabled.png"));
		if (overlayDisabledImage_.null())
			overlayDisabledImage_ = image::get_image(button_overlay_image_name_ + size_postfix + ".png~GS()");

		if (file_exists(game_config::path + "/images/" + button_overlay_image_name_ + size_postfix + "-disabled-pressed.png"))
			overlayPressedDisabledImage_.assign(image::get_image(button_overlay_image_name_ + size_postfix + "-disabled-pressed.png"));
		if (overlayPressedDisabledImage_.null())
			overlayPressedDisabledImage_ = image::get_image(button_overlay_image_name_ + size_postfix + "-pressed.png~GS()");
	} else {
		overlayImage_.assign(NULL);
	}

	if (disabled_image == NULL) {
		disabled_image = image::get_image(button_image_name_ + ".png~GS()");
	}

	if (pressed_image.null())
		pressed_image.assign(button_image);

	if (active_image.null())
		active_image.assign(button_image);

	if (type_ == TYPE_CHECK || type_ == TYPE_RADIO) {
		touched_image.assign(image::get_image(button_image_name_ + "-touched.png"));
		if (touched_image.null())
			touched_image.assign(pressed_image);

		pressed_active_image.assign(image::get_image(button_image_name_ + "-active-pressed.png"));
		if (pressed_active_image.null())
			pressed_active_image.assign(pressed_image);

		if (file_exists(game_config::path + "/images/" + button_image_name_ + size_postfix + "-disabled-pressed.png"))
			pressed_disabled_image.assign(image::get_image(button_image_name_ + "-disabled-pressed.png"));
		if (pressed_disabled_image.null())
			pressed_disabled_image = image::get_image(button_image_name_ + "-pressed.png~GS()");
	}

	if (button_image.null()) {
		ERR_DP << "error initializing button!\n";
		throw error();
	}

	base_height_ = button_image->h;
	base_width_ = button_image->w;

	if (type_ != TYPE_IMAGE) {
		set_label(label_);
	}

	if(type_ == TYPE_PRESS || type_ == TYPE_TURBO) {
		image_.assign(scale_surface(button_image,location().w,location().h));
		pressedImage_.assign(scale_surface(pressed_image,location().w,location().h));
		activeImage_.assign(scale_surface(active_image,location().w,location().h));
		disabledImage_.assign(scale_surface(disabled_image,location().w,location().h));
	} else {
		image_.assign(scale_surface(button_image,button_image->w,button_image->h));
		activeImage_.assign(scale_surface(active_image,button_image->w,button_image->h));
		disabledImage_.assign(scale_surface(disabled_image,button_image->w,button_image->h));
		pressedImage_.assign(scale_surface(pressed_image,button_image->w,button_image->h));
		if (type_ == TYPE_CHECK || type_ == TYPE_RADIO) {
			pressedDisabledImage_.assign(scale_surface(pressed_disabled_image,button_image->w,button_image->h));
			pressedActiveImage_.assign(scale_surface(pressed_active_image, button_image->w, button_image->h));
			touchedImage_.assign(scale_surface(touched_image, button_image->w, button_image->h));
		}
	}

	if (type_ == TYPE_IMAGE){
		calculate_size();
	}
}

button::~button()
{
}

void button::calculate_size()
{
	if (type_ == TYPE_IMAGE){
		SDL_Rect loc_image = location();
		loc_image.h = image_->h;
		loc_image.w = image_->w;
		set_location(loc_image);
		return;
	}
	SDL_Rect const &loc = location();
	bool change_size = loc.h == 0 || loc.w == 0;

	if (!change_size) {
		unsigned w = loc.w - (type_ == TYPE_PRESS || type_ == TYPE_TURBO ? horizontal_padding : checkbox_horizontal_padding + base_width_);
		if (type_ != TYPE_IMAGE)
		{
			int fs = font_size;
			int style = TTF_STYLE_NORMAL;
			std::string::const_iterator i_beg = label_.begin(), i_end = label_.end(),
				i = font::parse_markup(i_beg, i_end, &fs, NULL, &style);
			if (i != i_end) {
				std::string tmp(i, i_end);
				label_.erase(i - i_beg, i_end - i_beg);
				label_ += font::make_text_ellipsis(tmp, fs, w, style);
			}
		}
	}

	if (type_ != TYPE_IMAGE){
		textRect_ = font::draw_text(NULL, screen_area(), font_size,
		                            font::BUTTON_COLOR, label_, 0, 0);
	}

	if (!change_size)
		return;

	set_height(std::max(textRect_.h+vertical_padding,base_height_));
	if(type_ == TYPE_PRESS || type_ == TYPE_TURBO) {
		if(spacing_ == MINIMUM_SPACE) {
			set_width(textRect_.w + horizontal_padding);
		} else {
			set_width(std::max(textRect_.w+horizontal_padding,base_width_));
		}
	} else {
		if(label_.empty()) {
			set_width(base_width_);
		} else {
			set_width(checkbox_horizontal_padding + textRect_.w + base_width_);
		}
	}
}

void button::set_check(bool check)
{
	if (type_ != TYPE_CHECK && type_ != TYPE_RADIO)
		return;
	STATE new_state;

	if (check) {
		new_state = (state_ == ACTIVE || state_ == PRESSED_ACTIVE)? PRESSED_ACTIVE : PRESSED;
	} else {
		new_state = (state_ == ACTIVE || state_ == PRESSED_ACTIVE)? ACTIVE : NORMAL;
	}

	if (state_ != new_state) {
		state_ = new_state;
		set_dirty();
	}
}

void button::set_active(bool active)
{
	if ((state_ == NORMAL) && active) {
		state_ = ACTIVE;
		set_dirty();
	} else if ((state_ == ACTIVE) && !active) {
		state_ = NORMAL;
		set_dirty();
	}
}

bool button::checked() const
{
	return state_ == PRESSED || state_ == PRESSED_ACTIVE || state_ == TOUCHED_PRESSED;
}

void button::enable(bool new_val)
{
	if(new_val != enabled())
	{
		pressed_ = false;
		// check buttons should keep their state
		if(type_ != TYPE_CHECK) {
			state_ = NORMAL;
		}
		widget::enable(new_val);
	}
}

void button::draw_contents()
{
	surface image = image_;
	const int image_w = image_->w;

	int offset = 0;
	switch(state_) {
	case ACTIVE:
		image = activeImage_;
		break;
	case PRESSED:
		image = pressedImage_;
		if (type_ == TYPE_PRESS)
			offset = 1;
		break;
	case PRESSED_ACTIVE:
		image = pressedActiveImage_;
		break;
	case TOUCHED_NORMAL:
	case TOUCHED_PRESSED:
		image = touchedImage_;
		break;
	default:
		break;
	}

	SDL_Rect const &loc = location();
	SDL_Rect clipArea = loc;
	const int texty = loc.y + loc.h / 2 - textRect_.h / 2 + offset;
	int textx;

	if (type_ != TYPE_CHECK && type_ != TYPE_RADIO)
		textx = loc.x + image->w / 2 - textRect_.w / 2 + offset;
	else {
		clipArea.w += image_w + checkbox_horizontal_padding;
		textx = loc.x + image_w + checkbox_horizontal_padding / 2;
	}

	SDL_Color button_color = font::BUTTON_COLOR;

	if (!enabled()) {

		if (state_ == PRESSED || state_ == PRESSED_ACTIVE)
			image = pressedDisabledImage_;
		else image = disabledImage_;

		button_color = font::GRAY_COLOR;
	}

	if (!overlayImage_.null()) {

		surface noverlay = make_neutral_surface(
				enabled() ? overlayImage_ : overlayDisabledImage_);

		if (!overlayPressedImage_.null()) {
			switch (state_) {
			case ACTIVE:
				if (!overlayActiveImage_.null())
					noverlay = make_neutral_surface(overlayActiveImage_);
				break;
			case PRESSED:
			case PRESSED_ACTIVE:
			case TOUCHED_NORMAL:
			case TOUCHED_PRESSED:
				noverlay = make_neutral_surface( enabled() ?
						overlayPressedImage_ : overlayPressedDisabledImage_);
				break;
			default:
				break;
			}
		}

		surface nimage = make_neutral_surface(image);
		blit_surface(noverlay, NULL, nimage, NULL);
		image = nimage;
	}

	video().blit_surface(loc.x, loc.y, image);
	if (type_ != TYPE_IMAGE){
		clipArea.x += offset;
		clipArea.y += offset;
		clipArea.w -= 2*offset;
		clipArea.h -= 2*offset;
		font::draw_text(&video(), clipArea, font_size, button_color, label_, textx, texty);
	}

	update_rect(loc);
}

bool button::hit(int x, int y) const
{
	return point_in_rect(x,y,location());
}

static bool not_image(const std::string& str) { return !str.empty() && str[0] != IMAGE_PREFIX; }

void button::set_image(const std::string& image_file)
{
	button_image_name_ = image_file;
	load_images();
	set_dirty();
}

void button::set_overlay(const std::string& image_file)
{
	button_overlay_image_name_ = image_file;
	load_images();
	set_dirty();
}

void button::set_label(const std::string& val)
{
	label_ = val;

	//if we have a list of items, use the first one that isn't an image
	if (std::find(label_.begin(), label_.end(), COLUMN_SEPARATOR) != label_.end()) {
		const std::vector<std::string>& items = utils::split(label_, COLUMN_SEPARATOR);
		const std::vector<std::string>::const_iterator i = std::find_if(items.begin(),items.end(),not_image);
		if(i != items.end()) {
			label_ = *i;
		}
	}

	calculate_size();

	set_dirty(true);
}

void button::mouse_motion(SDL_MouseMotionEvent const &event)
{
	if (hit(event.x, event.y)) {
		// the cursor is over the widget
		if (state_ == NORMAL)
			state_ = ACTIVE;
		else if (state_ == PRESSED && (type_ == TYPE_CHECK || type_ == TYPE_RADIO))
			state_ = PRESSED_ACTIVE;
	} else {
		// the cursor is not over the widget

		if (type_ == TYPE_CHECK || type_ == TYPE_RADIO) {

			switch (state_) {
				case TOUCHED_NORMAL:
					state_ = NORMAL;
					break;
				case TOUCHED_PRESSED:
					state_ = PRESSED;
					break;
				case PRESSED_ACTIVE:
					state_ = PRESSED;
					break;
				case ACTIVE:
					state_ = NORMAL;
					break;
				default:
					break;
			}
		} else if ((type_ != TYPE_IMAGE) || state_ != PRESSED)
				state_ = NORMAL;
	}
}

void button::mouse_down(SDL_MouseButtonEvent const &event)
{
	if (hit(event.x, event.y) && event.button == SDL_BUTTON_LEFT) {

		switch (type_) {
			case TYPE_RADIO:
			case TYPE_CHECK:
				if (state_ == PRESSED_ACTIVE)
					state_ = TOUCHED_PRESSED;
				else if (state_ == ACTIVE)
					state_ = TOUCHED_NORMAL;
				break;
			case TYPE_TURBO:
				sound::play_UI_sound(game_config::sounds::button_press);
				state_ = PRESSED;
				break;
			default:
				state_ = PRESSED;
				break;
		}
	}
}

void button::release(){
	state_ = NORMAL;
	draw_contents();
}

void button::mouse_up(SDL_MouseButtonEvent const &event)
{
	if (!(hit(event.x, event.y) && event.button == SDL_BUTTON_LEFT))
		return;

	// the user has stopped pressing the mouse left button while on the widget
	switch (type_) {
	case TYPE_CHECK:

		switch (state_) {
			case TOUCHED_NORMAL:
				state_ = PRESSED_ACTIVE;
				pressed_ = true;
				break;
			case TOUCHED_PRESSED:
				state_ = ACTIVE;
				pressed_ = true;
				break;
			default:
				break;
		}
		if (pressed_) sound::play_UI_sound(game_config::sounds::checkbox_release);
		break;
	case TYPE_RADIO:
		if (state_ == TOUCHED_NORMAL || state_ == TOUCHED_PRESSED) {
			state_ = PRESSED_ACTIVE;
			pressed_ = true;
		//	exit(0);
			sound::play_UI_sound(game_config::sounds::checkbox_release);
		}
		//} else if (state_ == TOUCHED_PRESSED) {
		//	state_ = PRESSED_ACTIVE;
		//}
		break;
	case TYPE_PRESS:
		if (state_ == PRESSED) {
			state_ = ACTIVE;
			pressed_ = true;
			sound::play_UI_sound(game_config::sounds::button_press);
		}
		break;
	case TYPE_TURBO:
		state_ = ACTIVE;
		break;
	case TYPE_IMAGE:
		pressed_ = true;
		sound::play_UI_sound(game_config::sounds::button_press);
		break;
	}
}

void button::handle_event(const SDL_Event& event)
{
	if (hidden() || !enabled())
		return;

	STATE start_state = state_;

	if (!mouse_locked())
	{
		switch(event.type) {
			case SDL_MOUSEBUTTONDOWN:
				mouse_down(event.button);
				break;
			case SDL_MOUSEBUTTONUP:
				mouse_up(event.button);
				break;
			case SDL_MOUSEMOTION:
				mouse_motion(event.motion);
				break;
			default:
				return;
		}
	}

	if (start_state != state_)
		set_dirty(true);
}

bool button::pressed()
{
	if (type_ != TYPE_TURBO) {
		const bool res = pressed_;
		pressed_ = false;
		return res;
	} else
		return state_ == PRESSED || state_ == PRESSED_ACTIVE || state_ == TOUCHED_PRESSED;
}

}
