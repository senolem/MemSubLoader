#include "MemSubLoader.hpp"

// Find address containing audio ID
void Subtitles::findAddress(uintptr_t &address, int offset, HANDLE hProcess)
{
	SIZE_T bytesRead;
	if (ReadProcessMemory(hProcess, (LPCVOID)address, &address, 4, &bytesRead) )
		address += offset;
}

void Subtitles::search_memory(HANDLE hProcess)
{
	address_audio = bAddress_audio;
	address_play = bAddress_play;
	for(int i = 0; i < offset_audio.size();i++)
		findAddress(address_audio,offset_audio[i],hProcess);
	for(int i = 0; i < offset_play.size();i++)
		findAddress(address_play,offset_play[i],hProcess);
}

bool Subtitles::check_audio(HANDLE hProcess)
{
	SIZE_T bytesRead;
	if(!ReadProcessMemory(hProcess, (LPCVOID)address_play, &is_playing, 1, &bytesRead))
		std::cout<<"play error: "<<GetLastError()<<std::endl;
	if(!ReadProcessMemory(hProcess, (LPCVOID)address_audio, &AudioID, 4, &bytesRead))
		std::cout<<"audio error: "<<GetLastError()<<std::endl;
	if (is_playing)
	{
		if(AudioID != lastAudioID && AudioID > 0)
		{
		lastAudioID = AudioID;

		for(int i = 0; i < ID.size(); i++)
		{
			if(AudioID == ID[i])
			{
				textToDraw = Text[i].c_str();
				InvalidateRect(subtitlesHWND, NULL, FALSE);
			}
		}
		}
		return true;
	}
	else
		return false;
}

void Subtitles::file_memory(std::wifstream& file)
{
	int num;
	int offset;
	file >> bAddress_audio;

	file >> num;
	for(int i = 0; i<num;i++)
	{
		file >> offset;

		offset_audio.push_back(offset);
	}

	file >> bAddress_play;
	file >> num;

	for(int i = 0; i<num;i++)
	{
		file >> offset;

		offset_play.push_back(offset);
	}
}

void Subtitles::file_text(std::wifstream& file)
{
	std::wstring ws;
	int num;
	while(getline(file, ws))
	{
		if (ws == L"END")
			break;

		int num;
		std::wstring text;
		std::wistringstream iss(ws);
		iss >> num;
		std::getline(iss, text);

		ID.push_back(num);
		Text.push_back(text);
	}
}

// Open File Explorer
bool openFileExplorer(HWND hwnd, wchar_t *filePath, int filePathSize, int button)
{
	OPENFILENAME ofn = {
		sizeof(OPENFILENAME)
	};

	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = filePath;
	ofn.nMaxFile = filePathSize;
	ofn.Flags = OFN_FILEMUSTEXIST;

	switch(button) {
		case GAME_BUTTON:
		{
			ofn.lpstrFilter = L"Executable Files (*.exe)\0*.exe\0All Files (*.*)\0*.*\0";
		}
		break;

		case SUBTITLES_BUTTON:
		{
			ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
		}
		break;

		case MENU_LOAD:
		{
			ofn.lpstrFilter = L"Configuration Files (*.dat)\0*.dat\0All Files (*.*)\0*.*\0";
		}
		break;

		case MENU_SAVE:
		{
			ofn.lpstrFilter = L"Configuration Files (*.dat)\0*.dat\0All Files (*.*)\0*.*\0";
		}
		break;
	}
	if (button == MENU_SAVE)
	{
		if (GetSaveFileName(&ofn))
			return true;
		else
			return false;
	}
	else {
		if (GetOpenFileName(&ofn))
			return true;
		else
			return false;
	}
}

bool openFontDialog(HWND hwnd, LOGFONT &lf, HFONT &subtitlesFont)
{
	CHOOSEFONT cf;
	HFONT tmp;
	ZeroMemory(&cf, sizeof(CHOOSEFONT));
	cf.lStructSize = sizeof(CHOOSEFONT);
	cf.hwndOwner = hwnd;
	cf.lpLogFont = &lf;
	cf.Flags = CF_EFFECTS | CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
	cf.rgbColors = config.subtitlesColor;

	if (ChooseFont(&cf))
	{
		config.subtitlesColor = cf.rgbColors;
		tmp = CreateFontIndirect(&lf);
		
		if (tmp)
		{
			if (subtitlesFont)
			{
				DeleteObject(subtitlesFont);
			}

			subtitlesFont = tmp;
			return true;
		}
	}

	return false;
}

bool openColorDialog(HWND hwnd, COLORREF &subtitlesColor)
{
	CHOOSECOLOR cc;
	static COLORREF customColors[16] = { 0 };

	ZeroMemory(&cc, sizeof(CHOOSECOLOR));
	cc.lStructSize = sizeof(CHOOSECOLOR);
	cc.hwndOwner = hwnd;
	cc.lpCustColors = customColors;
	cc.rgbResult = subtitlesColor;
	cc.Flags = CC_FULLOPEN | CC_RGBINIT;

	if (ChooseColor(&cc))
	{
		subtitlesColor = cc.rgbResult;
		return true;
	}

	return false;
}

bool saveConfig(const Config &config, wchar_t *filename)
{
	if (wcslen(filename) < 4 || wcscmp(filename + wcslen(filename) - 4, L".dat") != 0)
	{
		if (wcslen(filename) + 4 < MAX_PATH)
		{
			wcsncat(filename, L".dat", MAX_PATH - wcslen(filename) - 1);
		}
	}

	FILE *file = _wfopen(filename, L"wb");
	if (file) {
		fwrite(&config, sizeof(Config), 1, file);
		fclose(file);
		return true;
	}
	return false;
}

bool loadConfig(Config &config, const wchar_t *filename)
{
	FILE *file = _wfopen(filename, L"rb");
	if (file) {
		fread(&config, sizeof(Config), 1, file);
		fclose(file);
		return true;
	}
	return false;
}

bool setAutoloadConfigPath(const wchar_t *path)
{
	wchar_t executablePath[MAX_PATH];

	if (getAutoloadPath(executablePath))
	{
		FILE *file = _wfopen(executablePath, L"w");
		if (file)
		{
			fwprintf(file, L"%s", path);
			fclose(file);
			return true;
		}
		return false;
	}
	return false;
}

bool getAutoloadConfigPath(wchar_t *path)
{
	wchar_t executablePath[MAX_PATH];

	if (getAutoloadPath(executablePath))
	{
		FILE *file = _wfopen(executablePath, L"r");
		if (file)
		{
			if (fgetws(path, MAX_PATH, file) != NULL)
			{
				fclose(file);
				return true;
			}
			fclose(file);
		}
		return false;
	}
	return false;
}

bool getAutoloadPath(wchar_t *executablePath)
{
	if (GetModuleFileName(NULL, executablePath, MAX_PATH) != 0)
	{
		wchar_t *lastBackslash = wcsrchr(executablePath, L'\\');
		if (lastBackslash != nullptr)
		{
			*lastBackslash = L'\0';
		}

		wcscat(executablePath, L"\\autoload.dat");
		return true;
	}
	return false;
}

void cleanup(void)
{
	DeleteObject(hFont);
	DeleteObject(titleFont);
	DeleteObject(subtitlesHFont);
	DeleteObject(logoBitmap);
	GdiplusShutdown(gdiplusToken);
}
