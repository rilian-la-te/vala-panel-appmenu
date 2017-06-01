/*
 * Copyright (c) 2014 Jared Gonzalez
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
 * Author: Jared Gonzalez
 */
#include "com_jarego_jayatana_jkey2xkey.h"

#include <stdlib.h>
#include <string.h>
#include <xkbcommon/xkbcommon.h>
/**
 * Convertir acelerador de menu Java aun acelerador X
 */
int jkeycode_to_xkey_map(int keycode)
{
	switch (keycode)
	{
	case JK_ENTER:
		return XKB_KEY_Return;
	case JK_BACK_SPACE:
		return XKB_KEY_BackSpace;
	case JK_TAB:
		return XKB_KEY_Tab;
	case JK_CANCEL:
		return XKB_KEY_Cancel;
	case JK_CLEAR:
		return XKB_KEY_Clear;
	/*case JK_SHIFT: return XKB_KEY_;
	 case JK_CONTROL: return XKB_KEY_;
	 case JK_ALT: return XKB_KEY_;*/
	case JK_PAUSE:
		return XKB_KEY_Pause;
	case JK_CAPS_LOCK:
		return XKB_KEY_Caps_Lock;
	case JK_ESCAPE:
		return XKB_KEY_Escape;
	case JK_SPACE:
		return XKB_KEY_space;
	case JK_PAGE_UP:
		return XKB_KEY_Page_Up;
	case JK_PAGE_DOWN:
		return XKB_KEY_Page_Down;
	case JK_END:
		return XKB_KEY_End;
	case JK_HOME:
		return XKB_KEY_Home;
	case JK_LEFT:
		return XKB_KEY_Left;
	case JK_UP:
		return XKB_KEY_Up;
	case JK_RIGHT:
		return XKB_KEY_Right;
	case JK_DOWN:
		return XKB_KEY_Down;
	case JK_COMMA:
		return XKB_KEY_comma;
	case JK_MINUS:
		return XKB_KEY_minus;
	case JK_PERIOD:
		return XKB_KEY_period;
	case JK_SLASH:
		return XKB_KEY_slash;
	/*case JK_0: return XKB_KEY_;
	 case JK_1: return XKB_KEY_;
	 case JK_2: return XKB_KEY_;
	 case JK_3: return XKB_KEY_;
	 case JK_4: return XKB_KEY_;
	 case JK_5: return XKB_KEY_;
	 case JK_6: return XKB_KEY_;
	 case JK_7: return XKB_KEY_;
	 case JK_8: return XKB_KEY_;
	 case JK_9: return XKB_KEY_;*/
	case JK_SEMICOLON:
		return XKB_KEY_semicolon;
	case JK_EQUALS:
		return XKB_KEY_equal;
	/*case JK_A: return XKB_KEY_;
	 case JK_B: return XKB_KEY_;
	 case JK_C: return XKB_KEY_;
	 case JK_D: return XKB_KEY_;
	 case JK_E: return XKB_KEY_;
	 case JK_F: return XKB_KEY_;
	 case JK_G: return XKB_KEY_;
	 case JK_H: return XKB_KEY_;
	 case JK_I: return XKB_KEY_;
	 case JK_J: return XKB_KEY_;
	 case JK_K: return XKB_KEY_;
	 case JK_L: return XKB_KEY_;
	 case JK_M: return XKB_KEY_;
	 case JK_N: return XKB_KEY_;
	 case JK_O: return XKB_KEY_;
	 case JK_P: return XKB_KEY_;
	 case JK_Q: return XKB_KEY_;
	 case JK_R: return XKB_KEY_;
	 case JK_S: return XKB_KEY_;
	 case JK_T: return XKB_KEY_;
	 case JK_U: return XKB_KEY_;
	 case JK_V: return XKB_KEY_;
	 case JK_W: return XKB_KEY_;
	 case JK_X: return XKB_KEY_;
	 case JK_Y: return XKB_KEY_;
	 case JK_Z: return XKB_KEY_;*/
	case JK_OPEN_BRACKET:
		return XKB_KEY_bracketleft;
	case JK_BACK_SLASH:
		return XKB_KEY_backslash;
	case JK_CLOSE_BRACKET:
		return XKB_KEY_bracketright;
	/*case JK_NUMPAD0: return XKB_KEY_;
	 case JK_NUMPAD1: return XKB_KEY_;
	 case JK_NUMPAD2: return XKB_KEY_;
	 case JK_NUMPAD3: return XKB_KEY_;
	 case JK_NUMPAD4: return XKB_KEY_;
	 case JK_NUMPAD5: return XKB_KEY_;
	 case JK_NUMPAD6: return XKB_KEY_;
	 case JK_NUMPAD7: return XKB_KEY_;
	 case JK_NUMPAD8: return XKB_KEY_;
	 case JK_NUMPAD9: return XKB_KEY_;*/
	case JK_MULTIPLY:
		return XKB_KEY_multiply;
	/*case JK_ADD: return XKB_KEY_;
	 case JK_SEPARATER: return XKB_KEY_;
	 case JK_SEPARATOR: return XKB_KEY_;
	 case JK_SUBTRACT: return XKB_KEY_;
	 case JK_DECIMAL: return XKB_KEY_; */
	case JK_DIVIDE:
		return XKB_KEY_division;
	case JK_DELETE:
		return XKB_KEY_Delete;
	case JK_NUM_LOCK:
		return XKB_KEY_Num_Lock;
	case JK_SCROLL_LOCK:
		return XKB_KEY_Scroll_Lock;
	/*case JK_F1: return XKB_KEY_;
	 case JK_F2: return XKB_KEY_;
	 case JK_F3: return XKB_KEY_;
	 case JK_F4: return XKB_KEY_;
	 case JK_F5: return XKB_KEY_;
	 case JK_F6: return XKB_KEY_;
	 case JK_F7: return XKB_KEY_;
	 case JK_F8: return XKB_KEY_;
	 case JK_F9: return XKB_KEY_;
	 case JK_F10: return XKB_KEY_;
	 case JK_F11: return XKB_KEY_;
	 case JK_F12: return XKB_KEY_;
	 case JK_F13: return XKB_KEY_;
	 case JK_F14: return XKB_KEY_;
	 case JK_F15: return XKB_KEY_;
	 case JK_F16: return XKB_KEY_;
	 case JK_F17: return XKB_KEY_;
	 case JK_F18: return XKB_KEY_;
	 case JK_F19: return XKB_KEY_;
	 case JK_F20: return XKB_KEY_;
	 case JK_F21: return XKB_KEY_;
	 case JK_F22: return XKB_KEY_;
	 case JK_F23: return XKB_KEY_;
	 case JK_F24: return XKB_KEY_;*/
	case JK_PRINTSCREEN:
		return XKB_KEY_Print;
	case JK_INSERT:
		return XKB_KEY_Insert;
	case JK_HELP:
		return XKB_KEY_Help;
	case JK_META:
		return XKB_KEY_Meta_R;
	case JK_BACK_QUOTE:
		return XKB_KEY_quoteright;
	case JK_QUOTE:
		return XKB_KEY_quoteleft;
	case JK_KP_UP:
		return XKB_KEY_KP_Up;
	case JK_KP_DOWN:
		return XKB_KEY_KP_Down;
	case JK_KP_LEFT:
		return XKB_KEY_KP_Left;
	case JK_KP_RIGHT:
		return XKB_KEY_KP_Right;
	case JK_DEAD_GRAVE:
		return XKB_KEY_dead_grave;
	case JK_DEAD_ACUTE:
		return XKB_KEY_dead_acute;
	case JK_DEAD_CIRCUMFLEX:
		return XKB_KEY_dead_circumflex;
	case JK_DEAD_TILDE:
		return XKB_KEY_dead_tilde;
	case JK_DEAD_MACRON:
		return XKB_KEY_dead_macron;
	case JK_DEAD_BREVE:
		return XKB_KEY_dead_breve;
	case JK_DEAD_ABOVEDOT:
		return XKB_KEY_dead_abovedot;
	case JK_DEAD_DIAERESIS:
		return XKB_KEY_dead_diaeresis;
	case JK_DEAD_ABOVERING:
		return XKB_KEY_dead_abovering;
	case JK_DEAD_DOUBLEACUTE:
		return XKB_KEY_dead_doubleacute;
	case JK_DEAD_CARON:
		return XKB_KEY_dead_caron;
	case JK_DEAD_CEDILLA:
		return XKB_KEY_dead_cedilla;
	case JK_DEAD_OGONEK:
		return XKB_KEY_dead_ogonek;
	case JK_DEAD_IOTA:
		return XKB_KEY_dead_iota;
	case JK_DEAD_VOICED_SOUND:
		return XKB_KEY_dead_voiced_sound;
	case JK_DEAD_SEMIVOICED_SOUND:
		return XKB_KEY_dead_semivoiced_sound;
	case JK_AMPERSAND:
		return XKB_KEY_ampersand;
	case JK_ASTERISK:
		return XKB_KEY_asterisk;
	case JK_QUOTEDBL:
		return XKB_KEY_quotedbl;
	case JK_LESS:
		return XKB_KEY_less;
	case JK_GREATER:
		return XKB_KEY_greater;
	case JK_BRACELEFT:
		return XKB_KEY_braceleft;
	case JK_BRACERIGHT:
		return XKB_KEY_braceright;
	case JK_AT:
		return XKB_KEY_at;
	case JK_COLON:
		return XKB_KEY_colon;
	/* case JK_CIRCUMFLEX: return XKB_KEY_; */
	case JK_DOLLAR:
		return XKB_KEY_dollar;
	case JK_EURO_SIGN:
		return XKB_KEY_EuroSign;
	case JK_EXCLAMATION_MARK:
		return XKB_KEY_exclamdown;
	case JK_INVERTED_EXCLAMATION_MARK:
		return XKB_KEY_exclam;
	case JK_LEFT_PARENTHESIS:
		return XKB_KEY_parenleft;
	case JK_NUMBER_SIGN:
		return XKB_KEY_numbersign;
	case JK_PLUS:
		return XKB_KEY_plus;
	case JK_RIGHT_PARENTHESIS:
		return XKB_KEY_parenright;
	case JK_UNDERSCORE:
		return XKB_KEY_underscore;
	case JK_WINDOWS:
		return XKB_KEY_Super_L;
	/* case JK_CONTEXT_MENU: return XKB_KEY_;
	 case K_FINAL: return XKB_KEY_;
	 case JK_CONVERT: return XKB_KEY_;
	 case JK_NONCONVERT: return XKB_KEY_;
	 case JK_ACCEPT: return XKB_KEY_;
	 case JK_MODECHANGE: return XKB_KEY_;
	 case JK_KANA: return XKB_KEY; */
	case JK_KANJI:
		return XKB_KEY_Kanji;
	/* case JK_ALPHANUMERIC: return XKB_KEY_; */
	// case JK_KATAKANA: return XKB_KEY_KATAKANA;
	case JK_HIRAGANA:
		return XKB_KEY_Hiragana;
	/*case JK_FULL_WIDTH: return XKB_KEY_;
	 case JK_HALF_WIDTH: return XKB_KEY_;
	 case JK_ROMAN_CHARACTERS: return XKB_KEY_;
	 case JK_ALL_CANDIDATES: return XKB_KEY_;
	 case JK_PREVIOUS_CANDIDATE: return XKB_KEY_; */
	case JK_CODE_INPUT:
		return XKB_KEY_Codeinput;
	// case JK_JAPANESE_KATAKANA: return XKB_KEY_KATAKANA;
	case JK_JAPANESE_HIRAGANA:
		return XKB_KEY_Hiragana;
	/*case JK_JAPANESE_ROMAN: return XKB_KEY_; */
	case JK_KANA_LOCK:
		return XKB_KEY_Kana_Lock;
	/*case JK_INPUT_METHOD_ON_OFF: return XKB_KEY_;
	 case JK_CUT: return XKB_KEY_;
	 case JK_COPY: return XKB_KEY_;
	 case JK_PASTE: return XKB_KEY_;
	 case JK_UNDO: return XKB_KEY_;
	 case JK_AGAIN: return XKB_KEY_;*/
	case JK_FIND:
		return XKB_KEY_Find;
	/*case JK_PROPS: return XKB_KEY_;
	 case JK_STOP: return XKB_KEY_;
	 case JK_COMPOSE: return XKB_KEY_;
	 case JK_ALT_GRAPH: return XKB_KEY_;*/
	case JK_BEGIN:
		return XKB_KEY_Begin;
	default:
		return 0;
	}
}

/**
 * Convertir acelerador de menu Java aun acelerador X
 */
char *jkeycode_to_xkey(int keycode)
{
	int code = 0;

	if (keycode >= JK_A && keycode <= JK_Z)
		code = keycode - JK_A + XKB_KEY_A;
	else if (keycode >= JK_F1 && keycode <= JK_F12)
		code = keycode - JK_F1 + XKB_KEY_F1;
	else if (keycode >= JK_0 && keycode <= JK_9)
		code = keycode - JK_0 + XKB_KEY_0;
	else
		code = jkeycode_to_xkey_map(keycode);

	if (code == 0)
		return NULL;

	char ret[40];
	size_t rets = xkb_keysym_to_utf8(code, ret, 1024);
	char *retf  = (char *)calloc((rets + 1), sizeof(char));
	strncpy(retf, ret, rets);
	return retf;
}
