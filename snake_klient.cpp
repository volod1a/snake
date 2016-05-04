#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "freeglut.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")


#include <iostream>
#include <ctime>
#include <WS2tcpip.h>
#include <string>
#include <iomanip>
#include <conio.h>
#include <gl/freeglut.h>
#include <atlconv.h>


#define debug true
#define response_time 20
using namespace std;


SOCKET Connect;
SOCKET* Connections;
SOCKET Listen;
int
ClientCount = 0,
settings[6], temp[6], stuff[8], myid;

static bool
IsFirstApple = true,
client_working_as_srv = true,
settings_Sended = false;

int
width = 30,
height = 30,
speed = 150, //70 - approx 1 b/s
Scale = 25, //1 block = 25 pix
w = Scale*width,
h = Scale*height,
gameTime = 60,
currentNumberOfClients = 1;


struct Snake{

	int x[100], y[100], num, dir, score;
	float color[3];
	string col;
	void initSnakestuff();

} s[4];

class Fructs{
public:
	int x, y;
	void New() {
		if (IsFirstApple) {
			x = width / 2.0;
			y = height / 2.0;
			IsFirstApple = false;
		}
		else  {
			x = rand() % width;
			y = rand() % height;
			stuff[0] = 0;
			stuff[1] = x;
			stuff[2] = y;
			for (int i = 0; i < currentNumberOfClients; i++)
			for (int j = 0; j < s[i].num; j++)
			if (x == s[i].x[j] && y == s[i].y[j])
			{
				x = rand() % width;
				y = rand() % height;
				stuff[1] = x;
				stuff[2] = y;
			}
			if (client_working_as_srv)
			for (int i = 1; i < currentNumberOfClients; i++)
				send(Connections[i], (char *)stuff, 3 * sizeof(int), NULL);
			else
			{
				send(Connect, (char *)stuff, 3 * sizeof(int), NULL);
			}
		}
	}

	void DrawApple()
	{
		glColor3f(0.0, 1.0, 0.0);
		glRectf(x*Scale, y*Scale, (x + 1)*Scale, (y + 1)*Scale);
	}

} apple;

void getWinner(bool isClientLeave)
{
	int max = s[0].score;
	bool draw = true;
	if (!isClientLeave){
		string winner = s[0].col;
		char answer[100] = "";
		for (int i = 0; i < currentNumberOfClients; i++)
		if (s[i].score > max) {
			max = s[i].score;
			winner = s[i].col;
			draw = false;
		}
		wstring buff = wstring(winner.begin(), winner.end());
		if (!draw)MessageBox(NULL, buff.c_str(), (LPCWSTR)L"Game end,winner is:", MB_OK);
		else MessageBox(NULL, (LPCWSTR)L"Draw!", (LPCWSTR)L"Game end", MB_OK);
	}
	else if (currentNumberOfClients == 2)
	{
		MessageBox(NULL, L"You win", L"Connection lost!", MB_OK);
		glutLeaveMainLoop();
	}


}

void timeOut()
{
	Sleep(1000 * gameTime);
	cout << "TIME OUT" << endl;
	getWinner(false);
	glutLeaveMainLoop();
	_getch();
}

void DrawSnake()
{

	for (int j = 0; j < currentNumberOfClients; j++){
		glColor3f(s[j].color[0], s[j].color[1], s[j].color[2]);
		for (int i = 0; i < s[j].num; i++)
		{
			glRectf(s[j].x[i] * Scale, s[j].y[i] * Scale, (s[j].x[i] + 0.9)*Scale, (s[j].y[i] + 0.9)*Scale);
		}
	}
}



void gameOver(int s_id)
{
	s[s_id].num = 3;
	s[s_id].score = 0;

	if (client_working_as_srv)
	{
		s[s_id].x[0] = rand() % width;
		s[s_id].y[0] = rand() % height;
		stuff[0] = 1;
		stuff[1] = s[s_id].x[0];
		stuff[2] = s[s_id].y[0];
		stuff[3] = s_id;
		for (int i = 0; i < currentNumberOfClients; i++)
		for (int j = 0; j < s[i].num; j++)
		{
			if ((s[s_id].x[0] == s[i].x[j] || s[s_id].y[0] == s[i].y[j])){

				s[s_id].x[0] = rand() % width;
				s[s_id].y[0] = rand() % height;

				stuff[1] = s[s_id].x[0];
				stuff[2] = s[s_id].y[0];

			}
			
		}

		for (int i = 0; i < currentNumberOfClients; i++){
			send(Connections[i], (char *)stuff, 4 * sizeof(int), NULL);
		}
	}



	DrawSnake();
}

void PrintText2D()
{
	string text = "Player ";
	int offset = 25;
	int x = 20;
	int y = h;
	glColor3f(0.0, 0.0, 0.0);

	for (int i = 0; i < currentNumberOfClients; i++) {
		y -= offset;
		text += s[i].col;
		text += std::to_string(s[i].score);
		glRasterPos2f(x, y);
		glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)text.c_str());
		offset += 20; text = "Player ";
	}
}


void Tick()
{
	for (int j = 0; j < currentNumberOfClients; j++)
	for (int i = s[j].num; i > 0; --i)
	{
		s[j].x[i] = s[j].x[i - 1];
		s[j].y[i] = s[j].y[i - 1];
	}


	for (int j = 0; j < currentNumberOfClients; j++)
		switch (s[j].dir) {
		case 0:
			s[j].y[0] += 1;
			break;
		case 1:
			s[j].x[0] -= 1;
			break;
		case 2:
			s[j].x[0] += 1;
			break;
		case 3:
			s[j].y[0] -= 1;
			break;
	}

	for (int j = 0; j < currentNumberOfClients; j++)
	if ((s[j].x[0] == apple.x) && (s[j].y[0] == apple.y))
	{
		s[j].num++;
		s[j].score += 1;
		PrintText2D();
		if (client_working_as_srv) {
			apple.New();
			apple.DrawApple(); //spawn new apple;
		}
	}
	for (int j = 0; j < currentNumberOfClients; j++)
	if (s[j].x[0] > width || s[j].x[0] < 0 || s[j].y[0] > height || s[j].y[0] < 0) gameOver(j); //checks if snake exits map borders
	for (int j = 0; j < currentNumberOfClients; j++){
		for (int i = 1; i < s[j].num; i++)
		if (s[j].x[0] == s[j].x[i] && s[j].y[0] == s[j].y[i])  gameOver(j); //checks if snake eat itself
	}

	for (int i = 0; i < currentNumberOfClients; i++){
		for (int j = 1; j < s[i].num; j++)
		if ((s[i].x[0] == s[i + 1].x[0]) && (s[i].y[0] == s[i + 1].y[0]))
		{

			gameOver(i + 1);

		}
		else if ((s[i].x[0] == s[i + 1].x[j]) && (s[i].y[0] == s[i + 1].y[j])) { gameOver(i); gameOver(i + 1); }
	}

	//}
	/*else {
	for (int x = 0; x < currentNumberOfClients; x++){
		for (int i = x + 1; i < currentNumberOfClients; i++)
		for (int j = 0; j < s[i].num; j++)
		{
			if (s[x].x[0] == s[i].x[0] && s[x].y[0] == s[i].y[0])
			{
			gameOver(x);
			gameOver(i); // head A hits head B
			}
			else if (s[x].x[0] == s[i].x[j] && s[x].y[0] == s[i].y[j]) gameOver(x);//head A hits tail B
	}
	}
	}*/
}



void DrawField()
{
	glColor3f(0.0, 0.7, 0.0);
	glBegin(GL_LINES);
	for (int i = 0; i<w; i += Scale)
	{
		glVertex2f(i, 0);
		glVertex2f(i, h);
	}
	for (int j = 0; j<h; j += Scale)
	{
		glVertex2f(0, j);
		glVertex2f(w, j);
	}
	glEnd();
}

void draw_scene() {

	glClear(GL_COLOR_BUFFER_BIT);

	DrawField();
	PrintText2D();
	DrawSnake();
	apple.DrawApple();

	glFlush();
	glutSwapBuffers();
}

void syncMainsnake()
{
	temp[0] = 4;
	temp[1] = s[myid].x[0];
	temp[2] = s[myid].y[0];
	temp[3] = myid;
	temp[4] = s[myid].num;
	temp[5] = s[myid].score;
	for (int j = 0; j < currentNumberOfClients; j++)
		send(Connections[j], (char *)temp, 6 * sizeof(int), NULL);

}

void KeyboardEvent(unsigned char key, int a, int b)
{
	temp[0] = 2;
	temp[2] = myid;
	switch (key)
	{
	
	case 'w':  s[myid].dir = 0; temp[1] = s[myid].dir;
		if (!client_working_as_srv) { send(Connect, (char *)temp, 3 * sizeof(int), NULL); DrawSnake(); }
		else {
			for (int i = 0; i <= ClientCount; i++)
				send(Connections[i], (char *)temp, 3 * sizeof(int), NULL); syncMainsnake();

		}
		break;
	
	case 'a': 	s[myid].dir = 1; temp[1] = s[myid].dir;
		if (!client_working_as_srv) { send(Connect, (char *)temp, 3 * sizeof(int), NULL); DrawSnake(); }
		else {
			for (int i = 0; i <= ClientCount; i++)
				send(Connections[i], (char *)temp, 3 * sizeof(int), NULL); syncMainsnake();

		}
		break;
	
	case 's':  s[myid].dir = 3; temp[1] = s[myid].dir;
		if (!client_working_as_srv) { send(Connect, (char *)temp, 3 * sizeof(int), NULL); DrawSnake(); }
		else {
			for (int i = 0; i <= ClientCount; i++)
				send(Connections[i], (char *)temp, 3 * sizeof(int), NULL); syncMainsnake();

		}
		break;
	
	case 'd':  s[myid].dir = 2; temp[1] = s[myid].dir;
		if (!client_working_as_srv) { send(Connect, (char *)temp, 3 * sizeof(int), NULL); DrawSnake(); }
		else {
			for (int i = 0; i <= ClientCount; i++)
				send(Connections[i], (char *)temp, 3 * sizeof(int), NULL); syncMainsnake();

		}
		break;

	}

}

void timer(int = 0)
{
	draw_scene();

	Tick();

	glutTimerFunc(speed, timer, 0);
}

void SendMessageToClient(int ID)
{
	if (!settings_Sended)
	{
		settings[0] = width; settings[1] = height;
		settings[2] = speed; settings[3] = gameTime;
		settings[4] = currentNumberOfClients; settings[5] = currentNumberOfClients - 1;
		send(Connect, (char *)settings, 6 * sizeof(int), NULL);
		cout << "sended : " << (char *)settings << endl;


		settings_Sended = true;
	}
	for (;; response_time)
	{

		temp[0] = 3;
		for (int i = 0; i < currentNumberOfClients; i++)
		{
			temp[1] = s[i].x[0];
			temp[2] = s[i].y[0];
			temp[3] = i;
			temp[4] = s[i].num;
			temp[5] = s[i].score;
			for (int j = 0; j < currentNumberOfClients; j++)
				send(Connections[j], (char *)temp, 6 * sizeof(int), NULL);
		}
		if (recv(Connections[ID], (char *)temp, 1, MSG_PEEK) == SOCKET_ERROR) getWinner(true);
		if (recv(Connections[ID], (char *)temp, 2 * sizeof(int), NULL))
		{

			cout << "Client " << ID << " answered" << endl;
			if (temp[0] == 2) s[ID].dir = temp[1];

		}

	}

}

void SendMessageToServer()
{

	int count = 0;
	for (;; response_time)
	{
		if (recv(Connect, (char *)temp, 1, MSG_PEEK) == SOCKET_ERROR) getWinner(true);

		if (recv(Connect, (char *)temp, 6 * sizeof(int), NULL))
		{
			if (temp[0] == 0 && temp[0] != 2) {
				cout << "apple:" << temp[1] << " " << temp[2] << endl;
				apple.x = temp[1];
				apple.y = temp[2];
				apple.DrawApple();
			}

			if (temp[0] == 4)
			{
				cout << "Sync coords of " << temp[3] << " snake" << endl;
				s[temp[3]].x[0] = temp[1];
				s[temp[3]].y[0] = temp[2];
				s[temp[3]].num = temp[4];
				s[temp[3]].score = temp[5];

			}
			if (temp[0] == 1 && temp[0] != 3)
			{
				s[temp[3]].x[0] = temp[1];
				s[temp[3]].y[0] = temp[2];
				cout << "Snake :" << temp[3] << " changing position to:" << temp[1] << "," << temp[2] << endl;
			}

			if (temp[0] == 2)
			{
				cout << " Snake " << temp[2] << " changing dir to " << temp[1] << endl;
				s[temp[2]].dir = temp[1];
				DrawSnake();
			}
			if (temp[0] == 3)
			{
				cout << "Sync coords of " << temp[3] << " snake" << endl;
				s[temp[3]].x[0] = temp[1];
				s[temp[3]].y[0] = temp[2];
				s[temp[3]].num = temp[4];
				s[temp[3]].score = temp[5];

			}
		}
	}
}

void initSnakestuff()
{
	//lets start gk
	s[0].x[0] = 0; s[1].x[0] = 0; s[2].x[0] = width - 1; s[3].x[0] = width - 1;
	s[0].y[0] = 0; s[1].y[0] = height - 1; s[2].y[0] = height - 1; s[3].y[0] = 0;

	s[0].dir = 0; s[1].dir = 2; s[2].dir = 3; s[3].dir = 1;
	s[0].color[0] = 0.0; s[0].color[1] = 0.0; s[0].color[2] = 1.0; s[0].col = "BLUE:";
	s[1].color[0] = 0.0; s[1].color[1] = 0.0; s[1].color[2] = 0.0; s[1].col = "BLACK:";
	s[2].color[0] = 255.0; s[2].color[1] = 255.0; s[2].color[2] = 0.0; s[2].col = "YELLOW:";
	s[3].color[0] = 165.0; s[3].color[1] = 42.0; s[3].color[2] = 42.0; s[3].col = "BROWN:";
	for (int i = 0; i < 4; i++)
	{
		s[i].num = 3;
		s[i].score = 0;
	}
};

int main(int argc, char **argv) {

	char server_ip[50];
	cout << "Choose mode: 0 - client , 1 - client as server:";
	cin >> client_working_as_srv;
	if (client_working_as_srv) {
		ClientCount = 1;
		myid = 0;
		currentNumberOfClients = 2;
		if (debug)
		{

			cout << "Input sizes of field W,H:";
			cin >> width >> height;
			cout << "Input time(sec):";
			cin >> gameTime;
			cout << "Input number of players:";
			cin >> currentNumberOfClients;
			cout << "Input speed (by default 70 - approx 1 b/s):";
			cin >> speed;
		}
	}
	srand(time(0));
	apple.New();
	initSnakestuff();

	WSAData data;
	WORD version = MAKEWORD(2, 2);
	int res = WSAStartup(version, &data);
	if (res != 0)
	{
		cout << "Cannot init server..";
		return 0;
	}
	if (client_working_as_srv) {
		struct addrinfo hints;
		struct addrinfo *result;

		Connections = (SOCKET*)calloc(64, sizeof(SOCKET));
		ZeroMemory(&hints, sizeof(hints));

		hints.ai_family = AF_INET;
		hints.ai_flags = AI_PASSIVE;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		getaddrinfo(NULL, "777", &hints, &result);

		Listen = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		bind(Listen, result->ai_addr, result->ai_addrlen);
		listen(Listen, SOMAXCONN);
		freeaddrinfo(result);
		cout << "Server is listening on 127.0.0.1:777." << endl;
		for (;; response_time)
		{
			if (Connect = accept(Listen, NULL, NULL))
			{
				cout << "Client connected!" << endl;
				Connections[ClientCount] = Connect;
				(client_working_as_srv) ? 1 : ClientCount++;
				send(Connections[ClientCount - 1], (char *)stuff, 2 * sizeof(int), NULL);
				CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)SendMessageToClient, (LPVOID)(ClientCount), NULL, NULL);
				CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)timeOut, NULL, NULL, NULL);
			}
			if (ClientCount == currentNumberOfClients - 1) break;
		}
	}
	else {
		ClientCount = 2;
		cout << "Input server ip:";
		if (!debug) cin >> server_ip;
		else strcpy(server_ip, "127.0.0.1");
		Connect = socket(AF_INET, SOCK_STREAM, 0);
		if (Connect < 0)
		{
			cout << "Socket error!" << endl;
			system("PAUSE");
			return -1;
		}
		sockaddr_in dest_addr;
		dest_addr.sin_family = AF_INET;
		dest_addr.sin_port = htons(777);
		dest_addr.sin_addr.s_addr = inet_addr(server_ip);
		if (connect(Connect, (sockaddr *)&dest_addr, sizeof(dest_addr)))
		{
			cout << "Connection error!" << endl;
			system("PAUSE");
			return -1;
		}

		cout << "Connected to " << server_ip << endl;
		if (recv(Connect, (char *)temp, 6 * sizeof(int), NULL))
		{
			if (!settings_Sended){
				cout << "Server answered:" << endl;

				for (int i = 0; i < 6; i++) cout << temp[i] << endl;
				width = temp[0]; height = temp[1]; speed = temp[2];
				gameTime = temp[3]; currentNumberOfClients = temp[4]; 
				myid = temp[4] - 1;
				settings_Sended = true;

			}
		}
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)SendMessageToServer, (LPVOID)(ClientCount), NULL, NULL);
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)timeOut, NULL, NULL, NULL);

	}
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(w, h);
	if (client_working_as_srv)
		glutInitWindowPosition(10, 150);
	else
		glutInitWindowPosition(770, 150);
	glutCreateWindow("Snake");
	glClearColor(1.0, 1.0, 0.6, 1.0);  //background color
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, w, 0, h);
	glutDisplayFunc(draw_scene);
	glutKeyboardFunc(KeyboardEvent);
	glutTimerFunc(speed, timer, 0); //tickrate
	glutMainLoop();
	_getch();
}
