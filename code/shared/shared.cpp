#include <pch.h>
#include "shared.h"

bool g_bQuit = false;
SteamNetworkingMicroseconds g_logTimeZero = 0;

std::mutex mutexUserInputQueue;
std::queue< std::string > queueUserInput;

std::thread* s_pThreadUserInput = nullptr;

void LocalUserInput_Init()
{
	s_pThreadUserInput = new std::thread([]()
		{
			while (!g_bQuit)
			{
				char szLine[4000];
				if (!fgets(szLine, sizeof(szLine), stdin))
				{
					// Well, you would hope that you could close the handle
					// from the other thread to trigger this.  Nope.
					if (g_bQuit)
						return;
					g_bQuit = true;
					Printf("Failed to read on stdin, quitting\n");
					break;
				}

				mutexUserInputQueue.lock();
				queueUserInput.push(std::string(szLine));
				mutexUserInputQueue.unlock();
			}
		});
}

void LocalUserInput_Kill()
{
	// Does not work.  We won't clean up, we'll just nuke the process.
	//	g_bQuit = true;
	//	_close( fileno( stdin ) );
	//
	//	if ( s_pThreadUserInput )
	//	{
	//		s_pThreadUserInput->join();
	//		delete s_pThreadUserInput;
	//		s_pThreadUserInput = nullptr;
	//	}
}

// You really gotta wonder what kind of pedantic garbage was
// going through the minds of people who designed std::string
// that they decided not to include trim.
// https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring

// trim from start (in place)
static inline void ltrim(std::string& s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
		return !std::isspace(ch);
		}));
}

// trim from end (in place)
static inline void rtrim(std::string& s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
		return !std::isspace(ch);
		}).base(), s.end());
}

// Read the next line of input from stdin, if anything is available.
bool LocalUserInput_GetNext(std::string& result)
{
	bool got_input = false;
	mutexUserInputQueue.lock();
	while (!queueUserInput.empty() && !got_input)
	{
		result = queueUserInput.front();
		queueUserInput.pop();
		ltrim(result);
		rtrim(result);
		got_input = !result.empty(); // ignore blank lines
	}
	mutexUserInputQueue.unlock();
	return got_input;
}