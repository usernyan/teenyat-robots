#ifndef	FILE_DIALOG_H 
#define FILE_DIALOG_H
#include <filesystem>
#include <vector>
namespace fs = std::filesystem;

class FileDialog {
	public:
	fs::path cur_dir;
	bool user_choice_set { false };
	fs::path user_choice;
	bool visible { false };
	std::string user_input_dir {""};
	FileDialog();
	void ChangeDir(fs::path dir);
	void GuiRender();
	void OpenEntry(fs::directory_entry e);
	void UpDir();
	bool ResetChoice();
};

#endif
