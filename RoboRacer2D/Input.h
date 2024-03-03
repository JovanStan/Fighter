#pragma once
#include "Sprite.h"

class Input
{
public:
	enum Command
	{
		CM_INVALID = -1,

		CM_LEFT,
		CM_RIGHT,
		CM_STOP,
		CM_MOVE,
		CM_QUIT,
		CM_UI
	}; 
protected:
	Command m_command;
	Sprite** m_uiElements;
	unsigned int m_uiCount;
	const bool CheckForClick(Sprite* p_element, int x, int y) const;
public:
	Input();
	~Input();
	
	const Command GetCommand() const { return m_command; }
	void SetCommand(Command command) { m_command = command; }

	void keyboardKey(int key, int x, int y);
	void keyboardSpec(int key, int x, int y);
	void mouse(int button, int state, int x, int y);

	void AddUiElement(Sprite* m_pElement);
};