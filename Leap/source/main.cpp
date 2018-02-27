//#include <Windows.h>
#include "LeapCapture.h"
#include <fstream>
#include "RandomizedForest.h"
#include "GestureVector.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <WS2tcpip.h>
#include <string>
#pragma comment(lib, "ws2_32.lib")

void Capture(std::string mode);
void Test(std::string treeFile);
void Train(std::string arffFile, std::string treeName);
std::vector<GestureVector> parffArse(std::string path);
void trainForest(std::vector<GestureVector> gesture, RandomizedForest forest, std::string filename);
void DisplayLetter(int letter);
SOCKET sock = INVALID_SOCKET;
void OpenSocket();
void CloseSocket();

std::string title = "Sign Language Letters";
void PassGestureToSocket(std::string gesture)
{
	//The gesture comes over as a integer from the random forest code
	//Need to change from integer to character by lookup table
	//each integer corresponds to a letter
	//write code to pass gesture over socket


	char buffer[4096];
	//Send the information
	int sendResult = send(sock, gesture.c_str(), gesture.size() + 1, 0);
	//std::cout << "sent...\n";
	//if (sendResult != SOCKET_ERROR)
	//{
	//	//Response
	//	ZeroMemory(buffer, 4096);
	//	int bytesRecieved = recv(sock, buffer, 4096, 0);
	//	std::cout << "error" << std::endl;
	//}
	//std::cout << "done" << std::endl;
}

std::string GestureToString(int gesture) {
	std::string alphabet[] = { "~", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z" };
	return alphabet[gesture];
}

void OpenSocket()
{
	//Initialize Winsock
	std::string ipAddress = "127.0.0.1";
	int port = 9999;

	WSADATA data;
	WORD ver = MAKEWORD(2, 2);
	int wsResult = WSAStartup(ver, &data);
	if (wsResult != 0) {
		std::cerr << "Can't start WinSock, error #" << wsResult << std::endl;
		return;
	}

	//Socket Creation
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) {
		std::cerr << "Cant create socket, error #" << WSAGetLastError() << std::endl;
		WSACleanup();
		return;
	}

	//Socket and Port Info
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

	//Connect to server
	//int connResult = 
	while (connect(sock, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR)
	{
		std::cerr << "Can't connect to server, error #" << WSAGetLastError() << std::endl;
	//	
	}
	//if (connResult == SOCKET_ERROR) {
	//	std::cerr << "Can't connect to server, error #" << WSAGetLastError() << std::endl;
	//	CloseSocket();
	//	return;
	//}
}
void CloseSocket()
{
	closesocket(sock);
	WSACleanup();
}
void DisplayLetter(int letter)
{
	std::string image;
	if (letter < 10)
	{
		image = "0" + std::to_string(letter) + ".jpeg";
	}
	else
	{
		image = std::to_string(letter) + ".jpeg";
	}
	cv::Mat letterImage = cv::imread(image);
	cv::imshow(title, letterImage);
	cv::waitKey(33);
}
void trainForest(std::vector<GestureVector> gesture, RandomizedForest forest, std::string filename)
{
	//std::cout << "Training...\n" << std::flush;

	for (int i = 0; i < gesture.size(); ++i) {
		forest.train(gesture[i], gesture[i].getLabel());
	}

	forest.save(filename);
}
void Capture(std::string mode, std::string arffFile)
{
	int ctr = 0;
	LeapCapture lc;
	bool jFlag = false, zFlag = false;
	if (mode == "write")
	{
		std::cout << "write" << std::endl;
		lc.WriteArffFileHeader(arffFile);//"C:/Users/IASA-FRI/Documents/leapArffFiles/leapData.arff"
	}
	else if (mode == "append")
	{
		//outArffFile.open("test.arff", std::fstream::in | std::fstream::out | std::fstream::app);
		lc.AppendArffFile(arffFile);
	}
	while (1)
	{
		bool captured = lc.Capture();
		//ADD EXTENDED FINGERS TO ARFFWRITER FUNCTION AND TO @ATTRIBUTES
		for (char button = 65; button < 91; button++)
		{
			if (GetAsyncKeyState(button) && captured && !jFlag && !zFlag)
			{
				if (button == 'J' || button == 'Z')
				{
					if (button == 'J') {
						jFlag = true;
					}
					else {
						zFlag = true;
					}
					//std::cout << button << " is not a valid gesture right now!" << std::endl;
				}
				//to do real-time, we need to call the random forest here.
				//need to write a function that takes LeapCapture data and tests on forest
				else
				{
					std::cout << "before capture\n";
					lc.writeArffFile(button);
					std::cout << button << " captured!  " << (ctr%10+1) << std::endl;
				}
			}
			else if (jFlag || zFlag) {
				std::cout << "capturing dynamic gesture" << std::endl;
				if (GetAsyncKeyState(button) && captured) {
					if (button == 'J') {
						jFlag = false;
						lc.CalculateVelocity();
						std::cout << "write j" << std::endl;
						lc.writeArffFile(button);
						std::cout << button << " captured!  " << std::endl;
						lc.ClearVelocity();
					}
					else if (button == 'Z') {
						zFlag = false;
						lc.CalculateVelocity();
						std::cout << "write z" << std::endl;
						lc.writeArffFile(button);
						std::cout << button << " captured!  " << std::endl;
						lc.ClearVelocity();
					}
					else {
						std::cout << "Expecting J or Z!" << std::endl;
					}
				}
				else
				{

					lc.Capture(true);
				}
				
			}
		}
		//can this be a shorter sleep and still get the right data?
		Sleep(200);
		lc.clearVectors();
	}
}

std::vector<GestureVector> parffArse(std::string path)
{
	std::vector<GestureVector> data;
	std::ifstream ifs;
	std::string line("");
	ifs.open(path, std::ifstream::in);

	if (ifs.is_open()) {
		while (line != "@DATA") {
			if (ifs.eof()) { break; }
			//I think this getline is causing the first line to be absorbed
			//Also, if there are only a few lines the program will crash
			std::getline(ifs, line);
		}
		while (true) {
			std::getline(ifs, line);
			if (ifs.eof()) { 
				break; 
			}
			std::istringstream ss(line);
			std::string token;
			std::string::size_type sz;
			std::vector<float> val;
			unsigned int lab;
			while (std::getline(ss, token, ',')) {
				if (token[1] == 'G') {
					lab = std::stoi(token.substr(2, 2));
				}
				else {
					val.push_back(std::stof(token, &sz));
				}
			}
			data.push_back(GestureVector(val, lab));
			val.clear();
			lab = 0;
		}
		ifs.close();
	}
	else
	{
		std::cout << "Failed to open " << path << std::endl;
	}
	return data;
}
void Test(std::string treeFile)
{
	LeapCapture lc;
	std::cout << "Loading Random Forest...";
	RandomizedForest forest;
	forest.load(treeFile + ".rf");
	std::cout << "DONE." << std::endl;
	std::vector<float> data;
	std::string letter = "";
	//cv::namedWindow(title);
	while (1)
	{
		bool found = lc.Capture();
		int classify(0);
		if (found)
		{
			lc.GetGestureVector(data);
			GestureVector gesture(data, 1);
			classify = forest.classify(gesture);
			std::cout << "Classify: " << classify << std::endl;
			letter = GestureToString(classify);
			std::cout << letter << std::endl;
			data.clear();
		}
		else
		{
			letter = "/";
		}
		PassGestureToSocket(letter);
		//DisplayLetter(classify);
		lc.clearVectors();
	}
}
void Train(std::string arffFile, std::string treeName)
{
	unsigned int nb_labels = 27;
	unsigned vector_size;
	double minV = -2. * PI;
	//double maxV = 2. * PI;
	double maxV = 100.;
	unsigned int depth = 37;
	//unsigned int nb_trees = 15;

	std::cout << "Parsing arff file..." << std::endl;
	std::vector<GestureVector> gesture = parffArse(arffFile);
	if (!gesture.empty())
	{
		std::cout << "Training random forest...";
		vector_size = static_cast<unsigned>(gesture[0].getFeatures().size());
		//for (int trees_i = 4; trees_i <= 30; ++trees_i) 
		{
			RandomizedForest forest(nb_labels, false, depth, 15, vector_size, minV, maxV);
			trainForest(gesture, forest, treeName);
		}
	}
	else
	{
		std::cout << "Failed to parse arff file correctly..." << std::endl;
	}
}

int main(int argc, char* argv[])
{
	std::string mode(argv[1]);
	if (mode.compare("train") == 0)
	{
		std::string arffFile(argv[2]);
		std::string treeName(argv[3]);
		Train(arffFile, treeName);
	}
	else if (mode.compare("test") == 0)
	{
		std::string treeName(argv[2]);
		OpenSocket();
		Test(treeName);
		CloseSocket();
	}
	else if (mode.compare("capture new") == 0)
	{
		std::string arffFile(argv[2]);
		Capture("write", arffFile);
	}
	else if (mode.compare("capture append") == 0)
	{
		std::string arffFile(argv[2]);
		Capture("append", arffFile);
	}
	return 0;
}