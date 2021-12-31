#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <atomic>
#include <signal.h>
// g++ -O3 -Wall -pedantic test.cpp -std=c++11 `pkg-config --cflags opencv4` `pkg-config --libs opencv4` -o rgb
//./rgb test.jpg /dev/sda
volatile sig_atomic_t end = 0;
void sig( int sigN ) {
	end = 1;
	std::cout << "Exiting..." << std::endl;
}
int main(int argc, char* argv[])
{
	signal(SIGINT, sig);
	signal(SIGTERM, sig);
	if (argc != 3)
	{
		std::cout << "Need image file path and dst"<< std::endl;
		exit(-1);
	}
	std::ofstream out;
	out.open(argv[2], std::ios::binary);
	if (!out.is_open())
	{
		std::cout << "Could not write the device..." << std::endl;
		exit(-1);
	}
	std::string path = cv::samples::findFile(argv[1]);
	cv::VideoCapture cap(path);
	if (!cap.isOpened())
	{
		std::cout << "Could not read the image: " << path << std::endl;
		exit(-1);
	}
	for(;;)
	{
		cv::Mat img,imgi;
		cap >> img;
		if (img.empty())
			break;
		if(end)
			break;
		cv::rotate(img, img, cv::ROTATE_90_COUNTERCLOCKWISE);
		cv::resize(img, imgi, cv::Size(128, 160), cv::INTER_LANCZOS4);
		cv::cvtColor(imgi, img, cv::COLOR_BGR2BGR565);
		for (int i = 0; i < img.rows; ++i)
		for (int j = 0; j < img.cols; ++j){
			uint16_t tmp = img.at<uint16_t>(i, j);
			tmp = (tmp << 8) | (tmp >> 8);
			out.write((char*)&tmp,2);
		}
	}
	cap.release();
	out.close();
	cv::destroyAllWindows();
	return 0;
}