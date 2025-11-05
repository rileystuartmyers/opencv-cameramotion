#include <iostream>
#include <vector>
#include <chrono>

#include <unistd.h>
#include <wiringPi.h>
#include <pigpio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>

const unsigned int PIN_HORIZONTAL = 14;
const unsigned int PIN_VERTICAL = 15;

void TestServoRange(const unsigned int PIN) {

	if (gpioInitialise() < 0)
	{
		std::cerr << "pigpio init failed" << std::endl;
		return;
	}

	for (int i = 500; i <= 2500; i += 100)
	{
		gpioServo(PIN, i);
		std::cout << "Servos at " << i << "." << std::endl;
		sleep(1);
	}

	return;

}

int main(int argc, char** argv) {

	if (gpioInitialise() < 0)
	{
		//std::cerr << "pigpio init failed" << std::endl;
		std::cout << "pigpio init failed" << std::endl;
		return 1;
	}

	int H_SPEED = 1500;
	int V_SPEED = 1500;

	gpioSetMode(PIN_HORIZONTAL, PI_OUTPUT);
	gpioSetMode(PIN_VERTICAL, PI_OUTPUT);

	cv::Mat color_image;
	cv::Mat grey_image;
	cv::VideoCapture cap(0, cv::CAP_V4L2);
	cap.set(cv::CAP_PROP_BUFFERSIZE, 1);

	if (!cap.isOpened()) {

		std::cout << "No video stream detected." << std::endl;
		return -1;

	}

	cv::CascadeClassifier faceCascade;
	faceCascade.load("resources/haarcascade_frontalface_default.xml");

	while (true) {

		cap.read(color_image);
		//cap >> color_image;

		if (color_image.empty()) {

			std::cout << "image empty..." << std::endl;
			//break;

		}

		cv::cvtColor(color_image, grey_image, cv::COLOR_BGR2GRAY);

		std::vector<cv::Rect> faces;
		faceCascade.detectMultiScale(grey_image, faces, 1.1, 10);

		if (faces.size() != 0) {

			int imageWidth = grey_image.cols;
			int imageHeight = grey_image.rows;

			int imageCenterX = imageWidth / 2;
			int imageCenterY = imageHeight / 2;

			int faceCenterX = ((faces[0].br().x + faces[0].tl().x) / 2);
			int faceCenterY = ((faces[0].br().y + faces[0].tl().y) / 2);

			int centerXMin = imageWidth * 0.45;
			int centerXMax = imageWidth * 0.55;

			int innerXBorder = imageWidth * 0.3;
			int outerXBorder = imageWidth * 0.7;

			int centerYMin = imageHeight * 0.4;
			int centerYMax = imageHeight * 0.6;

			// ***DEBUG LINE*** 
			faceCenterY = 0;

			if (faceCenterX >= centerXMin && faceCenterX <= centerXMax) {

				gpioServo(PIN_HORIZONTAL, 1500);

			} else if (faceCenterX >= centerXMin) {

				if (faceCenterX >= outerXBorder) {

					gpioServo(PIN_HORIZONTAL, 1330);

				} else {

					gpioServo(PIN_HORIZONTAL, 1400);
				}

			} else {

				if (faceCenterX <= innerXBorder) {

					gpioServo(PIN_HORIZONTAL, 1670);

				} else {

					gpioServo(PIN_HORIZONTAL, 1600);
				}

			}


			if (faceCenterY > 0) {

				std::cout << "PIN_VERTICAL MOVING UP" << std::endl;
				gpioServo(PIN_VERTICAL,  1000);

			} else if (faceCenterY < 0) {

				std::cout << "PIN_VERTICAL MOVING DOWN" << std::endl;
				gpioServo(PIN_VERTICAL, 2000);

			} else {

				gpioServo(PIN_VERTICAL, 1500);

			}

			printf("Center Coords: [%d, %d], Image Dims: [%d, %d]\n\n\n", faceCenterX, faceCenterY, imageWidth, imageHeight);
			std::chrono::milliseconds timespan(50);

		} else {

			gpioServo(PIN_HORIZONTAL, 1500);
			gpioServo(PIN_VERTICAL, 1500);

		}

	}

	gpioTerminate();

}
