#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <unistd.h>
#include <sys/wait.h>

// The Shell Class
class Shell {
	private:
		std::string currentDirectory = "";
		std::vector<std::string> history;
		std::vector<int> backgroundPid;
	public:
		Shell(std::string startingDirectory);
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

// Set the Working Directory
void Shell::SetCurrentDirectory(std::string newCurrentDirectory) {
	//Convert Relative Path to Absolute
	if (newCurrentDirectory.at(0) != '/') {
		newCurrentDirectory = currentDirectory + '/' + newCurrentDirectory;
	}

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
	if (splitCommand[1].at(0) != '/') {
		splitCommand[1] = currentDirectory + '/' + splitCommand[1];
	}

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
