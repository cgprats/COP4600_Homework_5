#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/stat.h>

// The Shell Class
class Shell {
	private:
		std::string currentDirectory = "";
		std::vector<std::string> history;
		std::vector<int> backgroundPid;
	public:
		Shell(std::string startingDirectory);
		std::string ConvertToAbsolute(std::string location);
		void SetCurrentDirectory(std::string currentDirectory);
		std::string GetCurrentDirectory();
		void ExecuteCommand(std::string command);
		void AddToHistory(std::string command);
		std::vector<std::string> GetHistory();
		void ImportHistory();
		void WriteHistory();
		void PrintHistory();
		void ClearHistory();
		void ReplayCommand(int n);
		void ExecSystem(std::vector<std::string> splitCommand);
		void KillSystem(int pid);
		void KillAll();
		void RepeatedCommand(std::vector<std::string> splitCommand);
		void CheckFile(std::string filename);
		void CreateFile(std::string filename);
		void CopyContents(std::string sourceFile, std::string destinationFile);
		void CopyToSubdir(std::string sourceDir, std::string destinationDir);
		void CopyDir(std::string sourceDir, std::string destinationDir);
};

// The Prompt Prototype
void Prompt(Shell);

// The Main Function
int main() {
	//Create the Instance of mysh
	Shell mysh(get_current_dir_name());

	//Start the Command Prompt
	Prompt(mysh);

	//Return 0 to Cleanly Exit the Program
	return 0;
}

// The Command Prompt
void Prompt(Shell mysh) {
	//Create Local Variables
	std::string command;

	//Continue Prompting for Command Until User Exits
	while (command.compare("byebye")) {
		//Ask User for Command
		std::cout << "# ";
		std::getline(std::cin >> std::ws, command);

		//Execute the Given Command
		mysh.ExecuteCommand(command);
	}

}

// Shell Constructor
Shell::Shell(std::string startingDirectory) {
	currentDirectory = startingDirectory;
	ImportHistory();
}

// Convert a Relative Path into an Absolute Path
std::string Shell::ConvertToAbsolute(std::string directory) {
	//Convert to Absolute if Relative
	if (directory.at(0) != '/') {
		directory = currentDirectory + '/' + directory;
	}

	return directory;
}

// Set the Working Directory
void Shell::SetCurrentDirectory(std::string newCurrentDirectory) {
	//Convert Relative Path to Absolute
	newCurrentDirectory = ConvertToAbsolute(newCurrentDirectory);

	//Create a Char Array to Check if Directory Exists
	char checkDir[newCurrentDirectory.size() + 1];
	newCurrentDirectory.copy(checkDir, newCurrentDirectory.size() + 1);
	checkDir[newCurrentDirectory.size()] = '\0';

	//If directory exists, change directory
	if (realpath(checkDir, NULL) != NULL) {
		currentDirectory = newCurrentDirectory;
	}

	//If directory doesn't exist, print error
	else {
		std::cout << "Error: Directory Does Not Exist" << std::endl;
	}

	//Remove Trailing / if it Exists
	if (currentDirectory.at(currentDirectory.size() - 1) == '/') {
		currentDirectory.erase(currentDirectory.size() - 1);
	}
}

// Get the Working Directory
std::string Shell::GetCurrentDirectory() {
	return currentDirectory;
}

// Execute the Given Command
void Shell::ExecuteCommand(std::string command) {
	//Add the Command to History
	AddToHistory(command);

	//Split the Command into Vector
	std::stringstream stream(command);
	std::string tempstr;
	std::vector<std::string> splitCommand;
	while (std::getline(stream, tempstr, ' ')) {
		splitCommand.push_back(tempstr);
	}

	//Change the Working Directory
	if (!splitCommand[0].compare("movetodir")) {
		if (splitCommand.size() == 2) {
			SetCurrentDirectory(splitCommand[1]);
		}
		else {
			std::cout << "Incorrect Amount of Parameters!" << std::endl;
			std::cout << "The correct syntax is movetodir directory" << std::endl;
		}
	}

	//Print Current Location
	else if (!splitCommand[0].compare("whereami")) {
		std::cout << GetCurrentDirectory() << std::endl;
	}

	//Perform History Related Operations
	else if (!splitCommand[0].compare("history")) {
		//Clear the History
		if (splitCommand.size() == 2 && !splitCommand[1].compare("-c")) {
			ClearHistory();
		}
		//Print the History
		else if (splitCommand.size() == 1) {
			PrintHistory();
		}
		else {
			std::cout << "Incorrect Amount of Parameters!" << std::endl;
			std::cout << "The correct syntax is history [-c]" << std::endl;
		}
	}

	//Write to the History File and Exit
	else if (!splitCommand[0].compare("byebye")) {
		WriteHistory();
	}

	//Re-Execute a Command from History
	else if (!splitCommand[0].compare("replay")) {
		ReplayCommand(std::stoi(splitCommand[1]));
	}

	//Start a Program Either in the Foreground or Background
	else if (!splitCommand[0].compare("start") || !splitCommand[0].compare("background")) {
		ExecSystem(splitCommand);
	}

	//Kill a Process
	else if (!splitCommand[0].compare("dalek")) {
		if (splitCommand.size() == 2) {
			KillSystem(std::stoi(splitCommand[1]));
		}
		else {
			std::cout << "Incorrect Amount of Parameters!" << std::endl;
			std::cout << "The correct syntax is dalek pid" << std::endl;
		}
	}

	//Repeatedly Run a Process
	else if (!splitCommand[0].compare("repeat")) {
		RepeatedCommand(splitCommand);
	}

	//Kill All Processes Started by the Shell
	else if (!splitCommand[0].compare("dalekall")) {
		KillAll();
	}

	//Check if a File Exists
	else if (!splitCommand[0].compare("dwelt")) {
		if (splitCommand.size() == 2) {
			CheckFile(splitCommand[1]);
		}
		else {
			std::cout << "Incorrect Amount of Parameters!" << std::endl;
			std::cout << "The correct syntax is dwelt file" << std::endl;
		}
	}

	//Create a New File
	else if (!splitCommand[0].compare("maik")) {
		if (splitCommand.size() == 2) {
			CreateFile(splitCommand[1]);
		}
		else {
			std::cout << "Incorrect Amount of Parameters!" << std::endl;
			std::cout << "The correct syntax is maik file" << std::endl;
		}
	}

	//Copy File Data
	else if (!splitCommand[0].compare("coppy")) {
		if (splitCommand.size() == 3) {
			CopyContents(splitCommand[1], splitCommand[2]);
		}
		else {
			std::cout << "Incorrect Amount of Parameters!" << std::endl;
			std::cout << "The correct syntax is coppy from-file to-file" << std::endl;
		}
	}

	//Copy Directory
	else if (!splitCommand[0].compare("coppyabode")) {
		if (splitCommand.size() == 3) {
			CopyToSubdir(splitCommand[1], splitCommand[2]);
		}
		else {
			std::cout << "Incorrect Amount of Parameters!" << std::endl;
			std::cout << "The correct syntax is coppyabode source-dir target-dir" << std::endl;
		}
	}

	//Handle an Unknown Command
	else {
		std::cout << "Command Not Found" << std::endl;
	}
}

// Add A Command to the History Vector
void Shell::AddToHistory(std::string command) {
	history.push_back(command);
}

// Return the History Vector
std::vector<std::string> Shell::GetHistory() {
	return history;
}

// Import the History from File
void Shell::ImportHistory() {
	std::ifstream historyFile;
	historyFile.open("history.txt");
	std::string historicCommand;
	while(std::getline(historyFile, historicCommand)) {
		history.push_back(historicCommand);
	}
}


// Write the History File
void Shell::WriteHistory() {
	std::ofstream historyFile;
	historyFile.open("history.txt");
	for (unsigned int i = 0; i < history.size(); i++) {
		historyFile << history[i] << std::endl;
	}
	historyFile.close();
}

// Print the Reversed History
void Shell::PrintHistory() {
	for (int i = history.size() - 1, j = 0; i >= 0; i--, j++) {
		std::cout << j << ": " << history[i] << std::endl;
	}
}

// Clear the Command History
void Shell::ClearHistory() {
	history.clear();
}

// Rerun a Command From History
void Shell::ReplayCommand(int n) {
	//Print the Command to be Replayed
	std::cout << history[history.size() - 2 - n] << std::endl;
	//Execute the Command to be Replayed
	ExecuteCommand(history[history.size() - 2 - n]);
}

// Execute a System Command
void Shell::ExecSystem(std::vector<std::string> splitCommand) {
	//Convert Relative Path to Absolute
	splitCommand[1] = ConvertToAbsolute(splitCommand[1]);

	//Convert the Split Command Vector into an Array of Char Pointers
	std::vector<char*> commandVector;
	for (unsigned int i = 1; i < splitCommand.size(); i++) {
		commandVector.push_back(const_cast<char*>(splitCommand[i].c_str()));
	}
	commandVector.push_back(NULL);

	//Create a Command Double Pointer from Command Vector for Use in execv
	char **command = commandVector.data();

	//Execute the Program
	pid_t pid;
	pid = fork();

	//Handle Fork Error
	if (pid < 0) {
		std::cout << "Failed to Fork Process" << std::endl;
	}

	//Execute Child Process
	else if (pid == 0) {
		//If Command is Not Found, Print Error and Exit Child
		if (execv(command[0], command) == -1) {
			std::cout << "Command Not Found or Cannot be Executed" << std::endl;
			exit(EXIT_FAILURE);
		}
	}

	//Run Program in Foreground
	else if (!splitCommand[0].compare("start")) {
		wait(NULL);
	}

	//Run Program in Background
	else if (!splitCommand[0].compare("background")) {
		std::cout << "pid: " << pid << std::endl;
		backgroundPid.push_back(pid);
	}
}

// Kill a System Process
void Shell::KillSystem(int pid) {
	//Print Success Message
	if (!kill(pid, 9)) {
		std::cout << "Exterminating pid: " << pid << std::endl;
	}

	//Print Error Message on Failure
	else {
		std::cout << "Failed to exterminate pid: " << pid << std::endl;
	}

	//Remove Process from Vector of Backgrounded Processes
	for (unsigned int i = 0; i < backgroundPid.size(); i++) {
		if (pid == backgroundPid[i]) {
			backgroundPid.erase(backgroundPid.begin() + i);
		}
	}
}

// Kills All Backgrounded Processes
void Shell::KillAll() {
	//Print Amount of Processes to Kill
	std::cout << "Exterminating " << backgroundPid.size() << " Processes:" << std::endl;

	//Exterminate Processes until Background Processes Vector is Empty
	while (backgroundPid.size() > 0) {
		KillSystem(backgroundPid[0]);
	}

	//Print Completion Message
	std::cout << "All Processes Exterminated" << std::endl;
}

// Runs a Command in the Background a Specified Number of Times
void Shell::RepeatedCommand(std::vector<std::string> splitCommand) {
	//Get the Number of Repetitions
	int numRepetitions = std::stoi(splitCommand[1]);

	//Create the New Command String
	splitCommand.erase(splitCommand.begin());
	splitCommand[0] = "background";

	//Execute the Repeated Command
	for (int i = 0; i < numRepetitions; i++) {
		ExecSystem(splitCommand);
	}
}

// Checks if a File Exists and if it is a Directory
void Shell::CheckFile(std::string filename) {
	//Handle Relative Paths
	filename = ConvertToAbsolute(filename);

	//Create Buffer for Stat
	struct stat statbuf;

	//The File Exists
	if (!stat(filename.c_str(), &statbuf)) {
		//The File is a Directory
		if (statbuf.st_mode & S_IFDIR) {
			std::cout << "Abode is." << std::endl;
		}

		//The File is a Regular File
		else if (statbuf.st_mode & S_IFREG) {
			std::cout << "Dwelt indeed." << std::endl;
		}
	}

	//The File Does Not Exist
	else {
		std::cout << "Dwelt not." << std::endl;
	}
}

// Creates a File if it Does Not Exist
void Shell::CreateFile(std::string filename) {
	//Handle Relative Paths
	filename = ConvertToAbsolute(filename);

	//Check if File Exists
	struct stat statbuf;

	//The File Does Not Exist
	if (stat(filename.c_str(), &statbuf)) {
		std::ofstream outFile;
		outFile.open(filename);
		outFile << "Draft" << std::endl;
		outFile.close();
	}

	//The File Exists
	else {
		std::cout << "The File Already Exists. Not Performing Any Action." << std::endl;
	}
}

// Copy the Contents from One File to Another
void Shell::CopyContents(std::string sourceFile, std::string destinationFile) {
	//Handle Relative Paths
	sourceFile = ConvertToAbsolute(sourceFile);
	destinationFile = ConvertToAbsolute(destinationFile);

	//Find the Path of the Destination
	std::string destinationPath;
	for (unsigned int i = 0; i < destinationFile.length(); i++) {
		if (destinationFile[i] == '/') {
			destinationPath = destinationFile.substr(0, i);
		}
	}

	//Check if the Source and Destination Files Exist
	struct stat statbuf;

	//The Source File Exists and is a Regular File
	if (!stat(sourceFile.c_str(), &statbuf) && statbuf.st_mode & S_IFREG) {
		//The Destination File Does Not Exist and Its Parent Directory Exists
		if (stat(destinationFile.c_str(), &statbuf) && !stat(destinationPath.c_str(), &statbuf)) {
			//Set Input File
			std::ifstream inFile;
			inFile.open(sourceFile, std::ios::binary);

			//Set Output File
			std::ofstream outFile;
			outFile.open(destinationFile, std::ios::binary);

			//Copy the Data
			outFile << inFile.rdbuf();

			//Close the Files
			outFile.close();
			inFile.close();
		}

		//The Destination's Parent Directory Does Not Exist
		else if (stat(destinationPath.c_str(), &statbuf)) {
			std::cout << "The Destination File's Parent Directory Does Not Exist!" << std::endl;
		}

		//The Destination File Exists
		else if (!stat(destinationFile.c_str(), &statbuf)) {
			std::cout << "The Destination File Already Exists. Not Performing Any Action." << std::endl;
		}
	}

	//The Source File Does Not Exist
	else {
		std::cout << "Source File Does Not Exist or is Not a Regular File!" << std::endl;
	}
}

//Copy into a Subdirectory under the Destination Directory
void Shell::CopyToSubdir(std::string sourceDir, std::string destinationDir) {
	//Handle Relative Path
	sourceDir = ConvertToAbsolute(sourceDir);
	destinationDir = ConvertToAbsolute(destinationDir);

	//Find the Folder Name of the Source Directory
	std::string sourceDirName;
	for (unsigned int i = 0; i < sourceDir.length(); i++) {
		if (sourceDir[i] == '/') {
			sourceDirName = sourceDir.substr(i + 1);
		}
	}

	//Check if the Source Directory and Destination Directory Exist and are Directories
	struct stat statbuf;
	if (!stat(sourceDir.c_str(), &statbuf) && statbuf.st_mode & S_IFDIR) {
		if (!stat(destinationDir.c_str(), &statbuf) && statbuf.st_mode & S_IFDIR) {
			//Create the Necessary Subdirectory
			destinationDir = destinationDir + '/' + sourceDirName;
			mkdir(destinationDir.c_str(), 0750);
			//Call the CopyDir Function with the Corrected Directories
			CopyDir(sourceDir, destinationDir);
		}
		else {
			std::cout << "The Destination Directory Either Does Not Exist or is Not a Directory!" << std::endl;
		}
	}
	else {
		std::cout << "The Source Directory Either Does Not Exist or is Not a Directory!" << std::endl;
	}
}

//Copy a Directory and Its Contents into Another
void Shell::CopyDir(std::string sourceDir, std::string destinationDir) {
	//Handle Relative Paths
	sourceDir = ConvertToAbsolute(sourceDir);
	destinationDir = ConvertToAbsolute(destinationDir);

	//Check if the Source and Destination Directories Exist and are Directories
	struct stat statbuf;

	//The Source Directory Exists and is a Directory
	if (!stat(sourceDir.c_str(), &statbuf) && statbuf.st_mode & S_IFDIR) {
		//The Destination Directory Exists and is a Directory
		if (!stat(destinationDir.c_str(), &statbuf) && statbuf.st_mode & S_IFDIR) {
			//Open the Source and Destination Directories
			struct dirent *dp;
			DIR *sdp = opendir(sourceDir.c_str());
			DIR *ddp = opendir(destinationDir.c_str());
			if (sdp && ddp) {
				//Read While there are Files in the Source Directory
				while((dp = readdir(sdp)) != NULL) {
					//Do Not Work on . (Alias for Current Dir) or .. (Alias for Parent Dir) Directories
					if (strcmp(dp->d_name, ".") && strcmp(dp->d_name, "..")) {
						std::string newSourceDir = sourceDir + "/" + dp->d_name;
						//Recursively Call CopyDir when a Subdirectory is Hit
						if (!stat(newSourceDir.c_str(), &statbuf) && statbuf.st_mode & S_IFDIR) {
							std::string newDestinationDir = destinationDir + "/" + dp->d_name;
							//Create the Subdirectory at the Destination
							if (!mkdir(newDestinationDir.c_str(), 0750)) {
								CopyDir(newSourceDir, newDestinationDir);
							}
						}

						//Call CopyContents to Copy File
						else if (!stat(newSourceDir.c_str(), &statbuf) && statbuf.st_mode & S_IFREG) {
							std::string fileDestination = destinationDir + "/" + dp->d_name;
							CopyContents(newSourceDir, fileDestination);
						}
					}
				}

				//Close the Source and Destination Directories
				closedir(ddp);
				closedir(sdp);
			}

			else if (!sdp && !ddp) {
				std::cout << "Could Not Open Either the Source or Destination Directories" << std::endl;
			}

			else if (!sdp) {
				std::cout << "Could Not Open the Source Directory" << std::endl;
			}

			else {
				std::cout << "Could Not Open the Destination Directory" << std::endl;
			}
		}

		//The Destination Directory Does Not Exist or is Not a Directory
		else {
			std::cout << "The Destination Directory Either Does Not Exist or is Not a Directory!" << std::endl;
		}
	}

	//The Source Directory Does Not Exist or is Not a Directory
	else {
		std::cout << "The Source Directory Either Does Not Exist or is Not a Directory!" << std::endl;
	}
}
