#include <AudioCtrl.hpp>

#include <Audio.hpp>
#include <Com.hpp>

#include <iostream>
#include <chrono>
#include <thread>

constexpr auto INITIAL_PORTS_LEN = 16;

constexpr ULONG COMM_PORT_ARDUINO = 5;

// Map App Path to slider
// Adjust App volume

int main(int argc, char** argv) {
	Audio::init();
	DeviceEnumerator deviceEnumerator = DeviceEnumerator();

	Com com = Com("COM5");

	std::vector<uint8_t> buffer = std::vector<uint8_t>(2);
	std::vector<uint8_t> tempBuf = std::vector<uint8_t>(1);

	com.send(reinterpret_cast<char*>(tempBuf.data()), tempBuf.size());

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
			if (slider == 5) com.send(reinterpret_cast<char*>(tempBuf.data()), tempBuf.size());
			std::cout << "Slider " << (int)slider << ": " << value;

			if (slider == 0) {
				deviceEnumerator.setMainVolume(value / 1023.0f);
			}

			if (slider == 5) std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
	}
}
