/*
 * Copyright (c) 2014 Jared González
 *
 * Permission is hereby granted, free of charge, to any
 * person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the
 * Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the
 * Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice
 * shall be included in all copies or substantial portions of
 * the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
 * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
 * OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * File:   com_jarego_jayatana_jkey2xkey.c
 * Author: Jared González
 */
#include "com_jarego_jayatana_jkey2xkey.h"

#include <X11/Xlib.h>
#include <X11/keysym.h>

/**
 * Convertir acelerador de menu Java aun acelerador X
 */
int jkeycode_to_xkey_map(int keycode) {
	switch (keycode) {
	case JK_ENTER:
		return XK_Return;
	case JK_BACK_SPACE:
		return XK_BackSpace;
	case JK_TAB:
		return XK_Tab;
	case JK_CANCEL:
		return XK_Cancel;
	case JK_CLEAR:
		return XK_Clear;
		/*case JK_SHIFT: return XK_;
		 case JK_CONTROL: return XK_;
		 case JK_ALT: return XK_;*/
	case JK_PAUSE:
		return XK_Pause;
	case JK_CAPS_LOCK:
		return XK_Caps_Lock;
	case JK_ESCAPE:
		return XK_Escape;
	case JK_SPACE:
		return XK_space;
	case JK_PAGE_UP:
		return XK_Page_Up;
	case JK_PAGE_DOWN:
		return XK_Page_Down;
	case JK_END:
		return XK_End;
	case JK_HOME:
		return XK_Home;
	case JK_LEFT:
		return XK_Left;
	case JK_UP:
		return XK_Up;
	case JK_RIGHT:
		return XK_Right;
	case JK_DOWN:
		return XK_Down;
	case JK_COMMA:
		return XK_comma;
	case JK_MINUS:
		return XK_minus;
	case JK_PERIOD:
		return XK_period;
	case JK_SLASH:
		return XK_slash;
		/*case JK_0: return XK_;
		 case JK_1: return XK_;
		 case JK_2: return XK_;
		 case JK_3: return XK_;
		 case JK_4: return XK_;
		 case JK_5: return XK_;
		 case JK_6: return XK_;
		 case JK_7: return XK_;
		 case JK_8: return XK_;
		 case JK_9: return XK_;*/
	case JK_SEMICOLON:
		return XK_semicolon;
	case JK_EQUALS:
		return XK_equal;
		/*case JK_A: return XK_;
		 case JK_B: return XK_;
		 case JK_C: return XK_;
		 case JK_D: return XK_;
		 case JK_E: return XK_;
		 case JK_F: return XK_;
		 case JK_G: return XK_;
		 case JK_H: return XK_;
		 case JK_I: return XK_;
		 case JK_J: return XK_;
		 case JK_K: return XK_;
		 case JK_L: return XK_;
		 case JK_M: return XK_;
		 case JK_N: return XK_;
		 case JK_O: return XK_;
		 case JK_P: return XK_;
		 case JK_Q: return XK_;
		 case JK_R: return XK_;
		 case JK_S: return XK_;
		 case JK_T: return XK_;
		 case JK_U: return XK_;
		 case JK_V: return XK_;
		 case JK_W: return XK_;
		 case JK_X: return XK_;
		 case JK_Y: return XK_;
		 case JK_Z: return XK_;*/
	case JK_OPEN_BRACKET:
		return XK_bracketleft;
	case JK_BACK_SLASH:
		return XK_backslash;
	case JK_CLOSE_BRACKET:
		return XK_bracketright;
		/*case JK_NUMPAD0: return XK_;
		 case JK_NUMPAD1: return XK_;
		 case JK_NUMPAD2: return XK_;
		 case JK_NUMPAD3: return XK_;
		 case JK_NUMPAD4: return XK_;
		 case JK_NUMPAD5: return XK_;
		 case JK_NUMPAD6: return XK_;
		 case JK_NUMPAD7: return XK_;
		 case JK_NUMPAD8: return XK_;
		 case JK_NUMPAD9: return XK_;*/
	case JK_MULTIPLY:
		return XK_multiply;
		/*case JK_ADD: return XK_;
		 case JK_SEPARATER: return XK_;
		 case JK_SEPARATOR: return XK_;
		 case JK_SUBTRACT: return XK_;
		 case JK_DECIMAL: return XK_; */
	case JK_DIVIDE:
		return XK_division;
	case JK_DELETE:
		return XK_Delete;
	case JK_NUM_LOCK:
		return XK_Num_Lock;
	case JK_SCROLL_LOCK:
		return XK_Scroll_Lock;
		/*case JK_F1: return XK_;
		 case JK_F2: return XK_;
		 case JK_F3: return XK_;
		 case JK_F4: return XK_;
		 case JK_F5: return XK_;
		 case JK_F6: return XK_;
		 case JK_F7: return XK_;
		 case JK_F8: return XK_;
		 case JK_F9: return XK_;
		 case JK_F10: return XK_;
		 case JK_F11: return XK_;
		 case JK_F12: return XK_;
		 case JK_F13: return XK_;
		 case JK_F14: return XK_;
		 case JK_F15: return XK_;
		 case JK_F16: return XK_;
		 case JK_F17: return XK_;
		 case JK_F18: return XK_;
		 case JK_F19: return XK_;
		 case JK_F20: return XK_;
		 case JK_F21: return XK_;
		 case JK_F22: return XK_;
		 case JK_F23: return XK_;
		 case JK_F24: return XK_;*/
	case JK_PRINTSCREEN:
		return XK_Print;
	case JK_INSERT:
		return XK_Insert;
	case JK_HELP:
		return XK_Help;
	case JK_META:
		return XK_Meta_R;
	case JK_BACK_QUOTE:
		return XK_quoteright;
	case JK_QUOTE:
		return XK_quoteleft;
	case JK_KP_UP:
		return XK_KP_Up;
	case JK_KP_DOWN:
		return XK_KP_Down;
	case JK_KP_LEFT:
		return XK_KP_Left;
	case JK_KP_RIGHT:
		return XK_KP_Right;
	case JK_DEAD_GRAVE:
		return XK_dead_grave;
	case JK_DEAD_ACUTE:
		return XK_dead_acute;
	case JK_DEAD_CIRCUMFLEX:
		return XK_dead_circumflex;
	case JK_DEAD_TILDE:
		return XK_dead_tilde;
	case JK_DEAD_MACRON:
		return XK_dead_macron;
	case JK_DEAD_BREVE:
		return XK_dead_breve;
	case JK_DEAD_ABOVEDOT:
		return XK_dead_abovedot;
	case JK_DEAD_DIAERESIS:
		return XK_dead_diaeresis;
	case JK_DEAD_ABOVERING:
		return XK_dead_abovering;
	case JK_DEAD_DOUBLEACUTE:
		return XK_dead_doubleacute;
	case JK_DEAD_CARON:
		return XK_dead_caron;
	case JK_DEAD_CEDILLA:
		return XK_dead_cedilla;
	case JK_DEAD_OGONEK:
		return XK_dead_ogonek;
	case JK_DEAD_IOTA:
		return XK_dead_iota;
	case JK_DEAD_VOICED_SOUND:
		return XK_dead_voiced_sound;
	case JK_DEAD_SEMIVOICED_SOUND:
		return XK_dead_semivoiced_sound;
	case JK_AMPERSAND:
		return XK_ampersand;
	case JK_ASTERISK:
		return XK_asterisk;
	case JK_QUOTEDBL:
		return XK_quotedbl;
	case JK_LESS:
		return XK_less;
	case JK_GREATER:
		return XK_greater;
	case JK_BRACELEFT:
		return XK_braceleft;
	case JK_BRACERIGHT:
		return XK_braceright;
	case JK_AT:
		return XK_at;
	case JK_COLON:
		return XK_colon;
		/* case JK_CIRCUMFLEX: return XK_; */
	case JK_DOLLAR:
		return XK_dollar;
	case JK_EURO_SIGN:
		return XK_EuroSign;
	case JK_EXCLAMATION_MARK:
		return XK_exclamdown;
	case JK_INVERTED_EXCLAMATION_MARK:
		return XK_exclam;
	case JK_LEFT_PARENTHESIS:
		return XK_parenleft;
	case JK_NUMBER_SIGN:
		return XK_numbersign;
	case JK_PLUS:
		return XK_plus;
	case JK_RIGHT_PARENTHESIS:
		return XK_parenright;
	case JK_UNDERSCORE:
		return XK_underscore;
	case JK_WINDOWS:
		return XK_Super_L;
		/* case JK_CONTEXT_MENU: return XK_;
		 case K_FINAL: return XK_;
		 case JK_CONVERT: return XK_;
		 case JK_NONCONVERT: return XK_;
		 case JK_ACCEPT: return XK_;
		 case JK_MODECHANGE: return XK_;
		 case JK_KANA: return XK; */
	case JK_KANJI:
		return XK_Kanji;
		/* case JK_ALPHANUMERIC: return XK_; */
		//case JK_KATAKANA: return XK_KATAKANA;
	case JK_HIRAGANA:
		return XK_Hiragana;
		/*case JK_FULL_WIDTH: return XK_;
		 case JK_HALF_WIDTH: return XK_;
		 case JK_ROMAN_CHARACTERS: return XK_;
		 case JK_ALL_CANDIDATES: return XK_;
		 case JK_PREVIOUS_CANDIDATE: return XK_; */
	case JK_CODE_INPUT:
		return XK_Codeinput;
		//case JK_JAPANESE_KATAKANA: return XK_KATAKANA;
	case JK_JAPANESE_HIRAGANA:
		return XK_Hiragana;
		/*case JK_JAPANESE_ROMAN: return XK_; */
	case JK_KANA_LOCK:
		return XK_Kana_Lock;
		/*case JK_INPUT_METHOD_ON_OFF: return XK_;
		 case JK_CUT: return XK_;
		 case JK_COPY: return XK_;
		 case JK_PASTE: return XK_;
		 case JK_UNDO: return XK_;
		 case JK_AGAIN: return XK_;*/
	case JK_FIND:
		return XK_Find;
		/*case JK_PROPS: return XK_;
		 case JK_STOP: return XK_;
		 case JK_COMPOSE: return XK_;
		 case JK_ALT_GRAPH: return XK_;*/
	case JK_BEGIN:
		return XK_Begin;
	default:
		return 0;
	}
}

/**
 * Convertir acelerador de menu Java aun acelerador X
 */
char *jkeycode_to_xkey(int keycode) {
	int code = 0;

	if (keycode >= JK_A && keycode <= JK_Z)
		code = keycode - JK_A + XK_A;
	else if (keycode >= JK_F1 && keycode <= JK_F12)
		code = keycode - JK_F1 + XK_F1;
	else if (keycode >= JK_0 && keycode <= JK_9)
		code = keycode - JK_0 + XK_0;
	else
		code = jkeycode_to_xkey_map(keycode);

	if (code == 0)
		return NULL;

	return XKeysymToString(code);
}
