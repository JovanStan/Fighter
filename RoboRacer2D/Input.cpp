#include "stdafx.h"
#include "Input.h"

Input::Input()
{
	m_command = Command::CM_INVALID;
	m_uiElements = new Sprite*[10];
	m_uiCount = 0;
}
Input::~Input()
{
	delete[] m_uiElements;
	m_uiElements = NULL;
}

void Input::mouse(int button, int state, int x, int y)
{
	m_command = Command::CM_STOP;

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		for (unsigned int i = 0; i < m_uiCount; i++)
		{
			Sprite* element = m_uiElements[i];
			if (element->IsActive() == true)
			{
				if (CheckForClick(element, x, y))
				{
					element->IsClicked(true);
					m_command = Input::Command::CM_UI;
					return;
				}
			}
		}
		
		m_command = Command::CM_MOVE;
	}
	else
	{
		m_command = Command::CM_INVALID;
	}
}

void Input::keyboardKey(int key, int x, int y)
{
	m_command = Command::CM_STOP;

	switch (key){
	case 'd':
		m_command = Command::CM_RIGHT;
		break;
	case 'a':
		m_command = Command::CM_LEFT;
		break;
	case 'w':
		m_command = Command::CM_MOVE;
		break;
	default:
		m_command = Command::CM_STOP;
	}

}

void Input::keyboardSpec(int key, int x, int y)
{
	m_command = Command::CM_STOP;

	switch (key) {
	case GLUT_KEY_F1:    // F1: Toggle between full-screen and windowed mode
		break;
	case GLUT_KEY_RIGHT:
		m_command = Command::CM_RIGHT;
		break;
	case GLUT_KEY_LEFT:
		m_command = Command::CM_LEFT;
		break;
	case GLUT_KEY_UP:
		m_command = Command::CM_MOVE;
		break;
	default:
		m_command = Command::CM_INVALID;
	}
}

void Input::AddUiElement(Sprite* p_element)
{
	m_uiElements[m_uiCount] = p_element;
	m_uiCount++;
}

const bool Input::CheckForClick(Sprite* p_element, int x, int y) const
{
	float left = p_element->GetPosition().x;
	float right = p_element->GetPosition().x + p_element->GetSize().width;

	float top = p_element->GetPosition().y;
	float bottom = p_element->GetPosition().y + p_element->GetSize().height;

	if (x >= left && x <= right &&  y >= top && y <= bottom) return true;
	else                                           	         return false;
}