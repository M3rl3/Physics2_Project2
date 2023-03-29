#include "cMazeMaker_W2023.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <psapi.h>
#include <sstream>

cMazeMaker_W2023::cMazeMaker_W2023()
{
	this->m_maze_size[2] = {0};

	this->m_start_axis = 0;
	this->m_start_side = 0;

	static const unsigned int UP = 0;
	static const unsigned int DOWN = 1;
	static const unsigned int LEFT = 2;
	static const unsigned int RIGHT = 3;

	return;
}

cMazeMaker_W2023::~cMazeMaker_W2023() {

}

// Select a random direction based on our options, append it to the current path, and move there
bool cMazeMaker_W2023::m_randomMove(bool first_move)
{
	int random_neighbor;
	std::vector< std::vector< int > > unvisited_neighbors;

	for (int direction = 0; direction < 4; direction++) 
	{
		int possible_pmd[2] = { 0, 0 };

		if (direction == UP) 
		{
			possible_pmd[1] = -1;
		}
		else if (direction == DOWN) 
		{
			possible_pmd[1] = 1;
		}
		else if (direction == LEFT) 
		{
			possible_pmd[0] = -1;
		}
		else 
		{
			possible_pmd[0] = 1;
		}

		if (this->m_dfs_path.back()[0] + possible_pmd[0] * 2 > 0 &&
			this->m_dfs_path.back()[0] + possible_pmd[0] * 2 < this->m_maze_size[0] - 1 &&
			this->m_dfs_path.back()[1] + possible_pmd[1] * 2 > 0 &&
			this->m_dfs_path.back()[1] + possible_pmd[1] * 2 < this->m_maze_size[1] - 1) 
		{
			if ( !maze[this->m_dfs_path.back()[1] + possible_pmd[1] * 2] 
				      [this->m_dfs_path.back()[0] + possible_pmd[0] * 2][1] ) 
			{
				std::vector< int > possible_move_delta = { possible_pmd[0], possible_pmd[1] };

				unvisited_neighbors.push_back(possible_move_delta);
			}
		}
	}

	if (unvisited_neighbors.size() > 0) 
	{
		random_neighbor = rand() % unvisited_neighbors.size();

		for (int a = 0; a < !first_move + 1; a++) 
		{
			std::vector< int > new_location;

			new_location.push_back(this->m_dfs_path.back()[0] + unvisited_neighbors[random_neighbor][0]);
			new_location.push_back(this->m_dfs_path.back()[1] + unvisited_neighbors[random_neighbor][1]);

			this->m_dfs_path.push_back(new_location);

			this->maze[this->m_dfs_path.back()[1]][this->m_dfs_path.back()[0]][0] = false;
			this->maze[this->m_dfs_path.back()[1]][this->m_dfs_path.back()[0]][1] = true;
		}

		return true;
	}
	else 
	{
		return false;
	}
}

bool cMazeMaker_W2023::m_validInteger(char* cstr)
{
	std::string str(cstr);

	for (char& c : str) 
	{
		if (!isdigit(c)) 
		{
			return false;
		}
	}

	return true;
}

// The fun part ;)
bool cMazeMaker_W2023::GenerateMaze(unsigned int width, unsigned int height)
{
	// Record start time
	std::chrono::steady_clock::time_point start = std::chrono::high_resolution_clock::now();

	this->m_maze_size[0] = width;
	this->m_maze_size[1] = height;

	if ( (width < 5) || (height < 5) )
	{
		return false;
	}

	// The width and height must be greater than or equal to 5 or it won't work
	// The width and height must be odd or else we will have extra walls
	for (int a = 0; a < 2; a++) 
	{
		if (this->m_maze_size[a] < 5) 
		{
			this->m_maze_size[a] = 5;
		}
		else if (this->m_maze_size[a] % 2 == 0) 
		{
			this->m_maze_size[a]--;
		}
	}

	this->m_initializeMaze();
	this->m_randomPoint(false);
	this->m_randomPoint(true);

	bool first_move = true;
	bool success = true;


	while (( int )this->m_dfs_path.size() > 1 - first_move) 
	{
		if (!success) 
		{
			this->m_dfs_path.pop_back();

			if (!first_move && this->m_dfs_path.size() > 2) 
			{
				this->m_dfs_path.pop_back();
			}
			else 
			{
				break;
			}

			success = true;
		}

		while (success) 
		{
			success = this->m_randomMove(first_move);

			if (first_move) 
			{
				first_move = false;
			}
		}
	}

	// Record end time
	std::chrono::steady_clock::time_point end = std::chrono::high_resolution_clock::now();

	// make a copy to refer later
	m_elapsed_time = end - start;

	return true;
}

// Initialize the maze vector with a completely-filled grid with the size the user specified
void cMazeMaker_W2023::m_initializeMaze(void)
{
	for (int a = 0; a < this->m_maze_size[1]; a++) 
	{
		for (int b = 0; b < this->m_maze_size[0]; b++) 
		{
			bool is_border;

			if (a == 0 || a == this->m_maze_size[1] - 1 ||
				b == 0 || b == this->m_maze_size[0] - 1) 
			{
				is_border = true;
			}
			else 
			{
				is_border = false;
			}

			std::vector< bool > new_cell = { true, is_border };

			if (( unsigned int )a + 1 > this->maze.size()) 
			{
				std::vector< std::vector< bool > > new_row = { new_cell };

				this->maze.push_back(new_row);
			}
			else 
			{
				this->maze[a].push_back(new_cell);
			}
		}
	}
}

void cMazeMaker_W2023::PrintMaze(const char* printChar)
{
	for (unsigned int a = 0; a < this->maze.size(); a++) 
	{
		for (unsigned int b = 0; b < this->maze[a].size(); b++) 
		{
			if (this->maze[a][b][0]) 
			{
				std::cout << printChar;
//				std::cout << "X";
//				std::cout << '\u2500';
//				std::cout << (char)0xC5;
//				std::cout << (char)0xDA; //   (218)
//				std::cout << (char)0xBF; //   (191)
//				std::cout << (char)0xC0; //   (192)
//				std::cout << (char)0xD9; //   (217)
			}
			else 
			{
				std::cout << "  ";
			}
		}

		std::cout << std::endl;
	}
}

void cMazeMaker_W2023::PrintMaze(void)
{
	for (unsigned int i = 0; i < this->maze.size(); i++)
	{
		for (unsigned int j = 0; j < this->maze[i].size(); j++)
		{
			if (this->maze[i][j][0])
			{
				if (PlaceWallCallBack != NULL) {
					PlaceWallCallBack();
				}
			}
			else
			{
				if (EmptySpaceCallBack != NULL) {
					EmptySpaceCallBack();
				}
			}
		}

		if (NewLineCallBack != NULL) {
			NewLineCallBack();
		}
	}
}

void cMazeMaker_W2023::PlaceWallCallback(void(*Callback)(void))
{
	PlaceWallCallBack = Callback;
}

void cMazeMaker_W2023::EmptySpaceCallback(void(*Callback)(void))
{
	EmptySpaceCallBack = Callback;
}

void cMazeMaker_W2023::NewLineCallback(void(*Callback)(void))
{
	NewLineCallBack = Callback;
}

// Set a random point (start or end)
void cMazeMaker_W2023::m_randomPoint(bool part)
{
	int axis;
	int side;

	if (!part) 
	{
		axis = rand() % 2;
		side = rand() % 2;

		this->m_start_axis = axis;
		this->m_start_side = side;
	}
	else 
	{
		bool done = false;

		while (!done) 
		{
			axis = rand() % 2;
			side = rand() % 2;

			if (axis != this->m_start_axis ||
				side != this->m_start_side) 
			{
				done = true;
			}
		}
	}

	std::vector< int > location = { 0, 0 };

	if (!side) 
	{
		location[!axis] = 0;
	}
	else 
	{
		location[!axis] = this->m_maze_size[!axis] - 1;
	}

	location[axis] = 2 * ( rand() % ( ( this->m_maze_size[axis] + 1 ) / 2 - 2 ) ) + 1;

	if (!part) 
	{
		this->m_dfs_path.push_back(location);
	}

	this->maze[location[1]][location[0]][0] = false;
	this->maze[location[1]][location[0]][1] = true;

	return;
}

void cMazeMaker_W2023::GetMazeAreaSquare(unsigned int widthIndexCentre,
	unsigned int heightIndexCentre,
	unsigned int halfSizeInCells,
	std::vector< std::vector< bool > >& mazeRegion)
{
	mazeRegion.clear();

	unsigned int mazeSize = halfSizeInCells * 2;

	for (unsigned int rowIndex = 0; rowIndex < mazeSize; rowIndex++)
	{
		// Make an empty row
		mazeRegion.push_back(std::vector<bool>());

		for (unsigned int columnIndex = 0; columnIndex < mazeSize; columnIndex++)
		{
			// Add an empty region
			mazeRegion[rowIndex].push_back(false);

		}//for ( unsigned int columnIndex
	}//for ( unsigned int rowIndex

	// Pick the portion you want from the larger maze
	for (unsigned int rowIndex = 0; rowIndex < mazeSize; rowIndex++)
	{
		for (unsigned int columnIndex = 0; columnIndex < mazeSize; columnIndex++)
		{
			int actualRowIndex = (int)rowIndex + (int)widthIndexCentre - (int)halfSizeInCells;
			int actualColumnIndex = (int)columnIndex + (int)heightIndexCentre - (int)halfSizeInCells;

			if ((actualRowIndex > 0) && (actualRowIndex < this->maze.size()))
			{
				// Row index is OK
				if ((actualColumnIndex > 0) && (actualColumnIndex < this->maze[actualRowIndex].size()))
				{
					// Column index is OK
					mazeRegion[rowIndex][columnIndex] = this->maze[actualRowIndex][actualColumnIndex][0];
				}
				else
				{
					// invalid column index, so leave blank (empty space)
					mazeRegion[rowIndex][columnIndex] = 0;
				}//if ( ( actualColumnIndex...
			}
			else
			{
				// invalid row index, so leave blank (empty space)
				mazeRegion[rowIndex][columnIndex] = 0;
			}//if ( ( actualRowIndex...


		}//for ( unsigned int columnIndex
	}//for ( unsigned int rowIndex

	return;
}


bool cMazeMaker_W2023::getMemoryUse(std::string& sMemoryInfo)
{

	sProcessMemoryCounters memoryInfo;

	if (!this->getMemoryUse(memoryInfo))
	{
		return false;
	}

	std::stringstream ssInfo;

	ssInfo
		<< "Process information for PID: " << memoryInfo.processID << "\n"
		<< "\tPageFaultCount: " << memoryInfo.PageFaultCount << "\n"
		<< "\tPeakWorkingSetSize: " << memoryInfo.PeakWorkingSetSize << "\n"
		<< "\tWorkingSetSize: " << memoryInfo.WorkingSetSize << "\n"
		<< "\tQuotaPeakPagedPoolUsage: " << memoryInfo.QuotaPeakPagedPoolUsage << "\n"
		<< "\tQuotaPagedPoolUsage: " << memoryInfo.QuotaPagedPoolUsage << "\n"
		<< "\tQuotaPeakNonPagedPoolUsage: " << memoryInfo.QuotaPeakNonPagedPoolUsage << "\n"
		<< "\tQuotaNonPagedPoolUsage: " << memoryInfo.QuotaNonPagedPoolUsage << "\n"
		<< "\tPagefileUsage: " << memoryInfo.PagefileUsage << "\n"
		<< "\tPeakPagefileUsage: " << memoryInfo.PeakPagefileUsage << "\n";

	sMemoryInfo = ssInfo.str();

	return true;
}

void cMazeMaker_W2023::getElapsedTime(std::chrono::duration<double>& timeTaken)
{
	timeTaken = m_elapsed_time;
}

bool cMazeMaker_W2023::getMemoryUse(sProcessMemoryCounters& memoryInfo)
{
	// https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getcurrentprocessid
	// https://learn.microsoft.com/en-us/windows/win32/psapi/collecting-memory-usage-information-for-a-process

	DWORD processID = GetCurrentProcessId();

	HANDLE hProcess;
	PROCESS_MEMORY_COUNTERS pmc;

	// Print information about the memory usage of the process.

	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
		PROCESS_VM_READ,
		FALSE, processID);
	if (NULL == hProcess)
	{
		return false;
	}

	std::stringstream ssInfo;
	if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
	{
		memoryInfo.processID = processID;
		memoryInfo.cb = pmc.cb;
		memoryInfo.PageFaultCount = pmc.PageFaultCount;
		memoryInfo.PeakWorkingSetSize = pmc.PeakWorkingSetSize;
		memoryInfo.WorkingSetSize = pmc.WorkingSetSize;
		memoryInfo.QuotaPeakPagedPoolUsage = pmc.QuotaPeakPagedPoolUsage;
		memoryInfo.QuotaPagedPoolUsage = pmc.QuotaPagedPoolUsage;
		memoryInfo.QuotaPeakNonPagedPoolUsage = pmc.QuotaPeakNonPagedPoolUsage;
		memoryInfo.QuotaNonPagedPoolUsage = pmc.QuotaNonPagedPoolUsage;
		memoryInfo.PagefileUsage = pmc.PagefileUsage;
		memoryInfo.PeakPagefileUsage = pmc.PeakPagefileUsage;
	}


	CloseHandle(hProcess);

	return true;
}
