#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector>
#include <thread>
#include "windows.h"

#pragma pack(push, 1) // Отключаем выравнивание байтов

// Структура для заголовка BMP файла
struct BMPHeader {
	uint16_t fileType;        // Тип файла, должно быть 'BM' (0x4D42)
	uint32_t fileSize;       // Размер файла в байтах
	uint16_t reserved1;       // Зарезервировано (0)
	uint16_t reserved2;       // Зарезервировано (0)
	uint32_t dataOffset;      // Смещение до начала данных в байтах
	uint32_t headerSize;      // Размер заголовка в байтах
	int32_t width;            // Ширина изображения в пикселях
	int32_t height;           // Высота изображения в пикселях
	uint16_t planes;          // Количество плоскостей, должно быть 1
	uint16_t bitsPerPixel;    // Глубина цвета (бит на пиксель)
	uint32_t compression;     // Метод сжатия, 0 для несжатых изображений
	uint32_t imageSize;       // Размер изображения в байтах
	int32_t xPixelsPerMeter;  // Горизонтальное разрешение, пикселей на метр
	int32_t yPixelsPerMeter;  // Вертикальное разрешение, пикселей на метр
	uint32_t colorsUsed;      // Количество используемых цветов (0 для 24-битных изображений)
	uint32_t colorsImportant; // Количество важных цветов (0)
};

#pragma pack(pop) // Восстанавливаем выравнивание байтов

// Функция для размытия вертикальной полосы изображения
void BlurVerticalStrip(std::vector<uint8_t>& imageData, int width, int height, int startColumn, int endColumn, BMPHeader header)
{
	int bytesPerPixel = header.bitsPerPixel / 8;

	for (int column = startColumn; column < endColumn; ++column) {
		for (int row = 0; row < height; ++row) {
			int pixelOffset = (row * width + column) * bytesPerPixel;

			if (column > 0 && column < width - 1) {
				// Усреднение цветов соседних пикселей по горизонтали
				for (int b = 0; b < bytesPerPixel; ++b) 
				{
					uint32_t sum = 0;

					// Суммируем значения цветов соседних пикселей
					for (int x = -1; x <= 1; ++x) {
						int neighborColumn = column + x;
						int neighborPixelOffset = (row * width + neighborColumn) * bytesPerPixel;

						if (neighborColumn >= 0 && neighborColumn < width) {
							sum += imageData[neighborPixelOffset + b];
						}
					}

					// Усредняем и записываем значение в текущий пиксель
					imageData[pixelOffset + b] = static_cast<uint8_t>(sum / 3); // Усреднение по 3 пикселям
				}
			}
		}
	}
}

int main() 
{
	setlocale(LC_ALL, "Russian");
	const char* inputBmpFileName = "input.bmp";
	const char* outputBmpFileName = "output.bmp";
	std::ifstream bmpFile(inputBmpFileName, std::ios::binary);

	if (!bmpFile) 
	{
		std::cerr << "Ошибка при открытии файла." << std::endl;
		return 1;
	}

	BMPHeader header;
	bmpFile.read(reinterpret_cast<char*>(&header), sizeof(BMPHeader));

	if (header.fileType != 0x4D42)
	{
		std::cerr << "Неподдерживаемый формат BMP файла." << std::endl;
		return 1;
	}

	std::vector<uint8_t> imageData(header.fileSize - header.dataOffset);
	bmpFile.read(reinterpret_cast<char*>(imageData.data()), imageData.size());

	bmpFile.close();

	int numThreads = 4;

	std::vector<std::thread> threads;
	int columnsPerThread = header.width / numThreads;
	int remainingColumns = header.width % numThreads;
	int startColumn = 0;

	for (int i = 0; i < numThreads; ++i) 
	{
		int endColumn = startColumn + columnsPerThread;
		if (i < remainingColumns) 
		{
			endColumn++; // Равномерное распределение остатка
		}
		threads.emplace_back(BlurVerticalStrip, std::ref(imageData), header.width, header.height, startColumn, endColumn);
		startColumn = endColumn;
	}

	for (auto& thread : threads) 
	{
		thread.join();
	}

	std::ofstream outputFile(outputBmpFileName, std::ios::binary);
	outputFile.write(reinterpret_cast<const char*>(&header), sizeof(BMPHeader));
	outputFile.write(reinterpret_cast<const char*>(imageData.data()), imageData.size());
	outputFile.close();

	std::cout << "Изображение успешно обработано и сохранено в " << outputBmpFileName << std::endl;

	return 0;
}