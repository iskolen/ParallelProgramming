#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector>
#include <thread>
#include <chrono>
#include <string>
#include <Windows.h>

#pragma pack(push, 1)
struct BMPHeader
{
	uint16_t fileType;
	uint32_t fileSize;
	uint16_t reserved1;
	uint16_t reserved2;
	uint32_t dataOffset;
	uint32_t headerSize;
	int32_t width;
	int32_t height;
	uint16_t planes;
	uint16_t bitsPerPixel;
	uint32_t compression;
	uint32_t imageSize;
	int32_t xPixelsPerMeter;
	int32_t yPixelsPerMeter;
	uint32_t colorsUsed;
	uint32_t colorsImportant;
};
#pragma pack(pop)

struct ThreadData
{
	std::vector<uint8_t>& imageData;
	int startColumn;
	int endColumn;
	int imageWidth;
	int imageHeight;
};

std::vector<uint8_t> ReadBmpImage(const char* fileName, BMPHeader& header)
{
	std::ifstream bmpFile(fileName, std::ios::binary);
	bmpFile.read(reinterpret_cast<char*>(&header), sizeof(BMPHeader));

	std::vector<uint8_t> imageData(header.fileSize - header.dataOffset);
	bmpFile.read(reinterpret_cast<char*>(imageData.data()), imageData.size());
	bmpFile.close();

	return imageData;
}

void ApplyGaussianBlur(void* threadData)
{
	ThreadData* data = reinterpret_cast<ThreadData*>(threadData);
	std::vector<uint8_t>& imageData = data->imageData;
	int startColumn = data->startColumn;
	int endColumn = data->endColumn;
	int imageHeight = data->imageHeight;
	int imageWidth = data->imageWidth;

	const int kernelSize = 5;
	double sigma = 1.0;

	double kernel[kernelSize][kernelSize];
	double kernelSum = 0.0;
	int radius = kernelSize / 2;

	for (int i = -radius; i <= radius; ++i)
	{
		for (int j = -radius; j <= radius; ++j)
		{
			double x = i * 1.0;
			double y = j * 1.0;
			kernel[i + radius][j + radius] = exp(-(x * x + y * y) / (2.0 * sigma * sigma)) / (2.0 * 3.14 * sigma * sigma);
			kernelSum += kernel[i + radius][j + radius];
		}
	}

	for (int i = 0; i < kernelSize; ++i)
	{
		for (int j = 0; j < kernelSize; ++j)
		{
			kernel[i][j] /= kernelSum;
		}
	}

	for (int y = 0; y < imageHeight; ++y)
	{
		for (int x = startColumn; x < endColumn; ++x)
		{
			double blurredPixelR = 0.0;
			double blurredPixelG = 0.0;
			double blurredPixelB = 0.0;

			for (int i = -radius; i <= radius; ++i)
			{
				for (int j = -radius; j <= radius; ++j)
				{
					int xOffset = x + i;
					int yOffset = y + j;

					if (xOffset >= 0 && xOffset < imageWidth && yOffset >= 0 && yOffset < imageHeight)
					{
						int pixelIndex = (yOffset * imageWidth + xOffset) * 3;
						blurredPixelR += imageData[pixelIndex] * kernel[i + radius][j + radius];
						blurredPixelG += imageData[pixelIndex + 1] * kernel[i + radius][j + radius];
						blurredPixelB += imageData[pixelIndex + 2] * kernel[i + radius][j + radius];
					}
				}
			}

			int pixelIndex = (y * imageWidth + x) * 3;
			imageData[pixelIndex] = static_cast<uint8_t>(blurredPixelR);
			imageData[pixelIndex + 1] = static_cast<uint8_t>(blurredPixelG);
			imageData[pixelIndex + 2] = static_cast<uint8_t>(blurredPixelB);
		}
	}
}

void SaveBmp(const char* fileName, const BMPHeader& header, const std::vector<uint8_t>& imageData)
{
	std::ofstream outFile(fileName, std::ios::binary);
	outFile.write(reinterpret_cast<const char*>(&header), sizeof(BMPHeader));
	outFile.write(reinterpret_cast<const char*>(imageData.data()), imageData.size());
	outFile.close();
}

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "Russian");
	if (argc != 5)
	{
		std::cerr << "Использование: Lab2.exe <input.bmp> <output.bmp> <numCores> <numThreads>" << std::endl;
		return 1;
	}

	const char* inputBmpFileName = argv[1];
	const char* outputBmpFileName = argv[2];
	int numCores = std::stoi(argv[3]);
	int numThreads = std::stoi(argv[4]);

	auto startTime = std::chrono::high_resolution_clock::now();

	BMPHeader header;
	std::vector<uint8_t> imageData = ReadBmpImage(inputBmpFileName, header);

	HANDLE* handles = new HANDLE[numThreads];
	int columnsPerThread = header.width / numThreads;
	int extraColumns = header.width % numThreads;
	int startColumn = 0;

	for (int i = 0; i < numThreads; ++i)
	{
		int start = i * columnsPerThread;
		int end = (i == numThreads - 1) ? (start + columnsPerThread + extraColumns) : (start + columnsPerThread);
		handles[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ApplyGaussianBlur,
			new ThreadData{ std::ref(imageData), start, end, header.width, header.height }, 0, NULL);

		SetThreadAffinityMask(handles[i], (1ULL << numCores) - 1);
	}

	for (int i = 0; i < numThreads; i++)
	{
		ResumeThread(handles[i]);
	}
	WaitForMultipleObjects(numThreads, handles, true, INFINITE);

	SaveBmp(outputBmpFileName, header, imageData);

	auto endTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = endTime - startTime;
	std::cout << "Время выполнения: " << elapsed.count() << " секунд." << std::endl;
}