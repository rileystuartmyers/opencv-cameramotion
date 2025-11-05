#include <iostream>
#include <map>
#include <vector>
#include <chrono>

#include <unistd.h>
#include <wiringPi.h>
#include <pigpio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>

const unsigned int PIN_HORIZONTAL = 14;

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

class Servo {
	
	private:
		
		int PIN;
		int pwmSignal = 1500;

		std::map<std::string, int> directions = {

			{"LEFT", 1400},
			{"STILL", 1500},
			{"RIGHT", 1600}

		};
		
	public:
	
		Servo(int _PIN = 14, std::map<std::string, int> _directions = {}) {

			if (gpioInitialise() < 0) {

				std::cerr << "pigpio init failed" << std::endl;
				return;

			}

			if (_directions.size() != 0) {

				directions = _directions;

			}
			
			PIN = _PIN;
			gpioSetMode(PIN, PI_OUTPUT);

		};

		void move(std::string direction) {

			if (directions.find(direction) == directions.end()) {

				std::cerr << "invalid direction" << std::endl;
				return;

			}

			int pwmInput = directions[direction];

			if (pwmInput != pwmSignal) {

				pwmSignal = pwmInput;
				gpioServo(PIN, pwmSignal);
			
			}

		}

		void stop() {

			if (pwmSignal != directions["STILL"]) {

				pwmSignal = directions["STILL"];
				gpioServo(PIN, pwmSignal);

			}

		}

		int getSignal() {

			return pwmSignal;

		}

};

int main(int argc, char** argv) {

	int HORIZONTAL_PIN = 14;
	Servo horizontal_servo(HORIZONTAL_PIN);

	cv::Mat color_image;
	cv::Mat grey_image;
	cv::VideoCapture cap(0, cv::CAP_V4L2);
	cap.set(cv::CAP_PROP_BUFFERSIZE, 1);

	if (!cap.isOpened()) {

		std::cout << "No video stream detected." << std::endl;
		return -1;

	}

	cap.read(color_image);

	float centerMargin = 0.10;
	int imageWidth = color_image.cols;
	int imageHeight = color_image.rows;

	int imageCenterX = imageWidth / 2;
	int centerXMin = imageCenterX - (imageWidth * centerMargin);
	int centerXMax = imageCenterX + (imageWidth * centerMargin);

	cv::CascadeClassifier faceCascade;
	faceCascade.load("resources/haarcascade_frontalface_default.xml");
	std::vector<cv::Rect> faces;

	while (true) {

		cap.read(color_image);

		if (color_image.empty()) {

			std::cerr << "image empty..." << std::endl;
			break;

		}

		cv::cvtColor(color_image, grey_image, cv::COLOR_BGR2GRAY);

		faces.clear();
		faceCascade.detectMultiScale(grey_image, faces, 1.1, 10);

		if (faces.size() != 0) {

			int faceCenterX = ((faces[0].br().x + faces[0].tl().x) / 2);

			if (faceCenterX > centerXMax) {

				horizontal_servo.move("LEFT");

			} else if (faceCenterX < centerXMin) {

				horizontal_servo.move("RIGHT");

			} else {

				horizontal_servo.stop();

			}

		} else {

			horizontal_servo.stop();

		}

		std::chrono::milliseconds timespan(100);

	}

	gpioTerminate();

}
