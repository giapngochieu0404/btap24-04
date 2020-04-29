#include <stdio.h>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>

#pragma comment(lib, "ws2_32")
#pragma warning(disable:4996)

#define WM_SOCKET WM_USER + 1

typedef struct
{
	SOCKET client;
	bool isRegisted;
} CLIENT_INFO;

CLIENT_INFO clients[64];
int numClients = 0;

BOOL CALLBACK WinProc(HWND, UINT, WPARAM, LPARAM);
void RemoveClient(SOCKET);

int main()
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(9000);

	SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	bind(listener, (SOCKADDR*)&addr, sizeof(addr));
	listen(listener, 5);

	WNDCLASS wndclass;
	const CHAR* providerClass = "AsyncSelect";
	HWND window;

	wndclass.style = 0;
	wndclass.lpfnWndProc = (WNDPROC)WinProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = NULL;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = (LPCWSTR)providerClass;

	if (RegisterClass(&wndclass) == 0)
		return NULL;


	if ((window = CreateWindow((LPCWSTR)providerClass, L"", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, NULL, NULL)) == NULL)
		return NULL;

	WSAAsyncSelect(listener, window, WM_SOCKET, FD_ACCEPT);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

BOOL CALLBACK WinProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	if (wMsg == WM_SOCKET)
	{
		if (WSAGETSELECTERROR(lParam))
		{
			RemoveClient(wParam);
			closesocket(wParam);
			return TRUE;
		}

		if (WSAGETSELECTEVENT(lParam) == FD_ACCEPT)
		{
			SOCKET client = accept(wParam, NULL, NULL);
			printf("New client accepted: %d\n", client);

			CLIENT_INFO tmp;
			tmp.client = client;
			tmp.isRegisted = FALSE;

			clients[numClients] = tmp;
			numClients++;

			const char* helloMsg = "Nhap username va password: \n";
			send(client, helloMsg, strlen(helloMsg), 0);

			WSAAsyncSelect(client, hWnd, WM_SOCKET, FD_READ | FD_CLOSE);
		}

		if (WSAGETSELECTEVENT(lParam) == FD_READ)
		{
			char buf[256];

			
			int ret = recv(wParam, buf, sizeof(buf), 0);
			buf[ret] = 0;

			//Tim socket cua client thu hien truyen du lieu
			int pos;
			for (int i = 0; i < numClients; i++)
				if (clients[i].client == wParam)
				{
					pos = i;
					break;
				}

			//Kiem tra dang nhap, neu chua dang nhap thi yeu cau nhap username va password
			if (!clients[pos].isRegisted)
			{
				char user[16], pwd[32], tmp[16];

				int ret = sscanf(buf, "%s %s %s", user, pwd, tmp);
				if (ret == 2)
				{
					char fbuf[64], fuser[32], fpwd[32];
					int flag = 0;

					FILE* f = fopen("D:\\Downloads\\check_user.txt", "r");
					while (!feof(f))
					{
						fgets(fbuf, sizeof(fbuf), f);
						ret = sscanf(fbuf, "%s %s", fuser, fpwd);
						if (ret == 2)
						{
							if (strcmp(user, fuser) == 0 && strcmp(pwd, fpwd) == 0)
							{
								flag = 1;
								break;
							}
						}
						else
							continue;
					}

					if (flag == 1)
					{
						clients[pos].isRegisted = TRUE;

						const char* okMsg = "Dang nhap thanh cong. Nhap lenh de thuc hien.\n";
						send(wParam, okMsg, strlen(okMsg), 0);
					}
					else {
						const char* errorMsg = "Khong ton tai username va password. Vui long dang nhap lai!!!\n";
						send(wParam, errorMsg, strlen(errorMsg), 0);
					}
				}
				else {
					const char* errorMsg = "Dang nhap khong thanh cong. Vui long dang nhap lai\n";
					send(wParam, errorMsg, strlen(errorMsg), 0);
				}
			}
			//Neu da dang nhap
			else {
				//Thuc hien lenh

				if (ret <= 0)
				{
					RemoveClient(clients[pos].client);
					closesocket(clients[pos].client);
				}

				if (buf[ret - 1] == '\n')
					buf[ret - 1] = 0;

				strcat(buf, " > D:\\Downloads\\output.txt");
				system(buf);


			}
		}
	}
}


void RemoveClient(SOCKET client)
{
	int i = 0;
	for (; i < numClients; i++)
		if (clients[i].client == client)
			break;

	if (i < numClients)
	{
		if (i < numClients - 1)
			clients[i] = clients[numClients - 1];
		numClients--;
	}
}
