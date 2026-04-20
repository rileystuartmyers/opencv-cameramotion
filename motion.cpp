#include <iostream>
#include <map>
#include <vector>
#include <chrono>

#include <unistd.h>
#include <wiringPi.h>
#include <pigpio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>

const unsigned int HORIZONTAL_PIN = 14;
const char* WINDOW_NAME = "Video Player";
const char* CASCADE_FILE_PATH = "resources/haarcascade_frontalface_default.xml";
const unsigned short int RECT_BORDER_THICKNESS = 2;
const unsigned int FRAME_PAUSE_TIME_MS = 100;
bool USE_GREYSCALE_IMAGE = true;

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

void VideoCapture_CheckForFailure(const cv::VideoCapture& cap) {
    if (!cap.isOpened()) {
        throw std::runtime_error("No video stream detected.");
    }
}

void ImageCapture_CheckForFailure(const cv::Mat& image) {
    if (image.empty()) {
        throw std::runtime_error("Image empty. Exiting...");
    }
}

void LoadCascade(cv::CascadeClassifier& faceCascade, const char* FilePath) {
    if (!faceCascade.load(FilePath)) {
        throw std::runtime_error("Failed to load cascade file. Exiting...");
    }
}

void SetFullscreen(const char* Window_Name) {
    std::cout << "set to fullscreen" << std::endl;
    cv::setWindowProperty(Window_Name, cv::WND_PROP_FULLSCREEN, cv::WINDOW_FULLSCREEN);
}

void SetColor() {
    USE_GREYSCALE_IMAGE = false;
}

void ArgumentCheck(int argc, char** argv, const char* Window_Name) {

    for (int i = 1; i < argc; ++i) {

        if (argv[i] == "-f" || argv[i] == "-fullscreen") {
            SetFullscreen(Window_Name);
        }

        if (argv[i] == "-c" || argv[i] == "-color") {
            SetColor();
        }

    }

};

int main(int argc, char** argv) {

	Servo horizontal_servo(HORIZONTAL_PIN);

    cv::namedWindow(WINDOW_NAME, cv::WINDOW_NORMAL);
	cv::Mat image;
	std::vector<cv::Rect> faces;
	
	cv::VideoCapture cap(0, cv::CAP_V4L2);
	VideoCapture_CheckForFailure(cap);
	cap.set(cv::CAP_PROP_BUFFERSIZE, 1);
	
	cv::CascadeClassifier faceCascade;
    LoadCascadeFromFile(faceCascade, CASCADE_FILE_PATH);
	

    ArgumentCheck(argc, argv, WINDOW_NAME);


	cap.read(image);

	float centerMargin = 0.10f;
	int imageWidth = image.cols;
	int imageHeight = image.rows;

	int imageCenterX = imageWidth / 2;
	int centerXMin = imageCenterX - (imageWidth * centerMargin);
	int centerXMax = imageCenterX + (imageWidth * centerMargin);

	int faceCenterX;

	while (true) {

		faces.clear();
		cap.read(image);

		if (image.empty()) {

			std::cerr << "image empty..." << std::endl;
			break;

		}

        if (USE_GREYSCALE_IMAGE == true) {
            cv::cvtColor(image, image, cv::COLOR_BGR2GRAY);
        }

		faceCascade.detectMultiScale(image, faces, 1.1, 10);

		if (faces.size() != 0) {

			faceCenterX = ((faces[0].br().x + faces[0].tl().x) / 2);

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

		std::chrono::milliseconds timespan(FRAME_PAUSE_TIME_MS);

	}

	gpioTerminate();

}
