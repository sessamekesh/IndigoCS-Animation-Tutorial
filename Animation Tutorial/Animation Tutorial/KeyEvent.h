#pragma once

enum class KEY_EVENT_TYPE
{
	PRESSED,
	RELEASED
};

class KeyEvent
{
public:
	char Key;
	KEY_EVENT_TYPE Type;

public:
	KeyEvent(char key, KEY_EVENT_TYPE type)
		: Key(key)
		, Type(type)
	{}
	KeyEvent(const KeyEvent&) = default;
};