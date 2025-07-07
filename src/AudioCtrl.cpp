#include <AudioCtrl.hpp>

#include <Audio.hpp>
#include <Com.hpp>

#include <iostream>
#include <chrono>
#include <thread>

constexpr size_t INITIAL_PORTS_LEN = 16;
constexpr ULONG COMM_PORT_ARDUINO = 5;

// Read config file
// Construct Audio Tree with slider assignments
// Add capability to set all audio recursively via DeviceEnumerator
// Lock DeviceEnumerator to not interfere with notifier? Maybe just fix EX handling...

// Map Output Device to slider
// Map EXE Path to slider
// Map EXE Name to slider

int main() {
	Audio::init();
	DeviceEnumerator deviceEnumerator = DeviceEnumerator();

	Com com = Com("\\\\.\\COM5");

	std::vector<uint8_t> buffer = std::vector<uint8_t>(2);
	std::vector<uint8_t> tempBuf = std::vector<uint8_t>(1);

	com.send(tempBuf);

	while (true) {
		com.waitForData();

		while (true) {
			buffer.resize(2);
			com.receive(buffer);

			if (buffer.size() == 0) break;
			if (buffer.size() != 2) continue;

			if ((buffer[0] >> 7) != 0 || buffer[1] >> 7 != 1) {
				com.receive(tempBuf);
				continue;
			}

			uint8_t slider = 0;
			int value = 0;

			slider = (buffer[0] >> 4) & 0b111;
			value = ((buffer[0] & 0b1111) << 6) | (buffer[1] & 0b111111);

			if (slider == 0) std::cout << std::endl;
			else std::cout << '\t';
			if (slider == 5) com.send(tempBuf);
			std::cout << "Slider " << (int)slider << ": " << value;

			if (slider == 0) {
				deviceEnumerator.setMainVolume(value / 1023.0f);
			}

			//if (slider == 5) std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
	}
}

#ifdef _WIN32
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR pCmdLine, _In_ int nCmdShow) {
	return main();
}
#endif
