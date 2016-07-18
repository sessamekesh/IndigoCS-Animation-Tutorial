#pragma once

#include "KeyEvent.h"

class IKeyEventListener
{
public:
	virtual void OnKeyPress(KeyEvent e) = 0;
};