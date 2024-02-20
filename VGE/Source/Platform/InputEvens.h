#pragma once

#include "Types.h"

namespace vge
{
	class Platform;

	enum class EventSource : u8
	{
		Keyboard,
		Mouse
	};

	class InputEvent
	{
	public:
		InputEvent(EventSource source) : _Source(source) {}

	public:
		inline EventSource GetSource() const { return _Source; }

	private:
		EventSource _Source;
	};

	enum class KeyCode : u8
	{
		Unknown,
		Space,
		Apostrophe, /* ' */
		Comma,      /* , */
		Minus,      /* - */
		Period,     /* . */
		Slash,      /* / */
		_0,
		_1,
		_2,
		_3,
		_4,
		_5,
		_6,
		_7,
		_8,
		_9,
		Semicolon, /* ; */
		Equal,     /* = */
		A,
		B,
		C,
		D,
		E,
		F,
		G,
		H,
		I,
		J,
		K,
		L,
		M,
		N,
		O,
		P,
		Q,
		R,
		S,
		T,
		U,
		V,
		W,
		X,
		Y,
		Z,
		LeftBracket,  /* [ */
		Backslash,    /* \ */
		RightBracket, /* ] */
		GraveAccent,  /* ` */
		Escape,
		Enter,
		Tab,
		Backspace,
		Insert,
		DelKey,
		Right,
		Left,
		Down,
		Up,
		PageUp,
		PageDown,
		Home,
		End,
		Back,
		CapsLock,
		ScrollLock,
		NumLock,
		PrintScreen,
		Pause,
		F1,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		F10,
		F11,
		F12,
		KP_0,
		KP_1,
		KP_2,
		KP_3,
		KP_4,
		KP_5,
		KP_6,
		KP_7,
		KP_8,
		KP_9,
		KP_Decimal,
		KP_Divide,
		KP_Multiply,
		KP_Subtract,
		KP_Add,
		KP_Enter,
		KP_Equal,
		LeftShift,
		LeftControl,
		LeftAlt,
		RightShift,
		RightControl,
		RightAlt
	};

	enum class KeyAction : u8
	{
		Down,
		Up,
		Repeat,
		Unknown
	};

	class KeyInputEvent : public InputEvent
	{
	public:
		KeyInputEvent(KeyCode code, KeyAction action) 
			: InputEvent(EventSource::Keyboard), _Code(code), _Action(action) 
		{}

	public:
		inline KeyCode GetCode() const { return _Code; }
		inline KeyAction GetAction() const { return _Action; }

	private:
		KeyCode _Code;
		KeyAction _Action;
	};

	enum class MouseButton : u8
	{
		Left,
		Right,
		Middle,
		Back,
		Forward,
		Unknown
	};

	enum class MouseAction : u8
	{
		Down,
		Up,
		Move,
		Unknown
	};

	class MouseInputEvent : public InputEvent
	{
	public:
		MouseInputEvent(MouseButton button, MouseAction action, float posX, float posY)
			: InputEvent(EventSource::Mouse), _Button(button), _Action(action), _PosX(posX), _PosY(posY)
		{}

	public:
		inline MouseButton GetButton() const { return _Button; }
		inline MouseAction GetAction() const { return _Action; }
		inline float GetPosX() const { return _PosX; }
		inline float GetPosY() const { return _PosY; }

	private:
		MouseButton _Button;
		MouseAction _Action;
		float _PosX;
		float _PosY;
	};
}	// namespace vge
