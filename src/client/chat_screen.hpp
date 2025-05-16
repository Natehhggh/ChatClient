

#include <iostream>
#include <ncurses.h>

class ChatScreen {
public:
	ChatScreen();
	~ChatScreen();

	void Init();

	void PostMessage(const char username[80],const char msg[80]);

	std::string CheckBoxInput();


private:
	int msg_y = 0;
	WINDOW * inputwin = nullptr;

};

