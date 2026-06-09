#pragma once

#include <cstdint>
#include <ostream>
#include <istream>
#include <string_view>

class BigEndianDataOutputStream
{
public:
	BigEndianDataOutputStream(std::ostream& out);

	void WriteDouble(double d);
	void WriteSingle(float f);

	void WriteI64(int64_t i);
	void WriteI32(int32_t i);
	void WriteI16(int16_t i);
	void WriteI8(int8_t i);
	void WriteArray(int8_t* arr, size_t size);

	void WriteU64(uint64_t i);
	void WriteU32(uint32_t i);
	void WriteU16(uint16_t i);
	void WriteU8(uint8_t i);

	void WriteString(std::string_view string);

private:
	std::ostream* m_OutputStream;
};

class BigEndianDataInputStream
{
public:
	BigEndianDataInputStream(std::istream& in);

	double ReadDouble();
	float ReadSingle();

	int64_t ReadI64();
	int32_t ReadI32();
	int16_t ReadI16();
	int8_t  ReadI8();
	void ReadArray(int8_t* array, size_t size);

	uint64_t ReadU64();
	uint32_t ReadU32();
	uint16_t ReadU16();
	uint8_t  ReadU8();

	std::string ReadString();

private:
	std::istream* m_InputStream;
};