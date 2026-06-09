#include "BigEndianDataStream.h"

#ifndef __wii__
#define IS_LITTLE_ENDIAN
#endif

#ifdef IS_LITTLE_ENDIAN

static uint16_t dobyteswap16(uint16_t val) {
	return (val >> 8) | (val << 8);
}

static uint32_t dobyteswap32(uint32_t val) {
	return ((val >> 24) & 0x000000FF) |
		((val >> 8) & 0x0000FF00) |
		((val << 8) & 0x00FF0000) |
		((val << 24) & 0xFF000000);
}

static uint64_t dobyteswap64(uint64_t val) {
	return ((val >> 56) & 0x00000000000000FFULL) |
		((val >> 40) & 0x000000000000FF00ULL) |
		((val >> 24) & 0x0000000000FF0000ULL) |
		((val >> 8) & 0x00000000FF000000ULL) |
		((val << 8) & 0x000000FF00000000ULL) |
		((val << 24) & 0x0000FF0000000000ULL) |
		((val << 40) & 0x00FF000000000000ULL) |
		((val << 56) & 0xFF00000000000000ULL);
}

#else

static uint16_t dobyteswap16(uint16_t val) { return val; }
static uint32_t dobyteswap32(uint32_t val) { return val; }
static uint64_t dobyteswap64(uint64_t val) { return val; }

#endif

BigEndianDataOutputStream::BigEndianDataOutputStream(std::ostream& out)
{
	m_OutputStream = &out;
}

void BigEndianDataOutputStream::WriteDouble(double d)
{
	WriteU64(*reinterpret_cast<uint64_t*>(&d));
}

void BigEndianDataOutputStream::WriteSingle(float f)
{
	WriteU32(*reinterpret_cast<uint64_t*>(&f));
}

void BigEndianDataOutputStream::WriteI64(int64_t i)
{
	WriteU64(*reinterpret_cast<uint64_t*>(&i));
}

void BigEndianDataOutputStream::WriteI32(int32_t i)
{
	WriteU32(*reinterpret_cast<uint32_t*>(&i));
}

void BigEndianDataOutputStream::WriteI16(int16_t i)
{
	WriteU16(*reinterpret_cast<uint16_t*>(&i));
}

void BigEndianDataOutputStream::WriteI8(int8_t i)
{
	WriteU8(*reinterpret_cast<uint8_t*>(&i));
}

void BigEndianDataOutputStream::WriteArray(int8_t* arr, size_t size)
{
	m_OutputStream->write((const char*)arr, size);
}

void BigEndianDataOutputStream::WriteU64(uint64_t i)
{
	i = dobyteswap64(i);
	m_OutputStream->write(reinterpret_cast<char*>(&i), sizeof(uint64_t));
}

void BigEndianDataOutputStream::WriteU32(uint32_t i)
{
	i = dobyteswap32(i);
	m_OutputStream->write(reinterpret_cast<char*>(&i), sizeof(uint32_t));
}

void BigEndianDataOutputStream::WriteU16(uint16_t i)
{
	i = dobyteswap16(i);
	m_OutputStream->write(reinterpret_cast<char*>(&i), sizeof(uint16_t));
}

void BigEndianDataOutputStream::WriteU8(uint8_t i)
{
	m_OutputStream->write(reinterpret_cast<char*>(&i), sizeof(uint8_t));
}

void BigEndianDataOutputStream::WriteString(std::string_view string)
{
	WriteU16(static_cast<uint16_t>(string.size()));
	for (size_t i = 0; i < string.size(); i++)
	{
		m_OutputStream->write(string.data() + i, sizeof(char));
	}
}

BigEndianDataInputStream::BigEndianDataInputStream(std::istream& in)
{
	m_InputStream = &in;
}

double BigEndianDataInputStream::ReadDouble()
{
	uint64_t raw = ReadU64();
	return *reinterpret_cast<double*>(&raw);
}

float BigEndianDataInputStream::ReadSingle()
{
	uint32_t raw = ReadU32();
	return *reinterpret_cast<float*>(&raw);
}

int64_t BigEndianDataInputStream::ReadI64()
{
	return static_cast<int64_t>(ReadU64());
}

int32_t BigEndianDataInputStream::ReadI32()
{
	return static_cast<int32_t>(ReadU32());
}

int16_t BigEndianDataInputStream::ReadI16()
{
	return static_cast<int16_t>(ReadU16());
}

int8_t BigEndianDataInputStream::ReadI8()
{
	return static_cast<int8_t>(ReadU8());
}

void BigEndianDataInputStream::ReadArray(int8_t* array, size_t size)
{
	m_InputStream->read((char*)array, size);
}

uint64_t BigEndianDataInputStream::ReadU64()
{
	uint64_t val;
	m_InputStream->read(reinterpret_cast<char*>(&val), sizeof(uint64_t));
	return dobyteswap64(val);
}

uint32_t BigEndianDataInputStream::ReadU32()
{
	uint32_t val;
	m_InputStream->read(reinterpret_cast<char*>(&val), sizeof(uint32_t));
	return dobyteswap32(val);
}

uint16_t BigEndianDataInputStream::ReadU16()
{
	uint16_t val;
	m_InputStream->read(reinterpret_cast<char*>(&val), sizeof(uint16_t));
	return dobyteswap16(val);
}

uint8_t BigEndianDataInputStream::ReadU8()
{
	uint8_t val;
	m_InputStream->read(reinterpret_cast<char*>(&val), sizeof(uint8_t));
	return val;
}

std::string BigEndianDataInputStream::ReadString()
{
	uint16_t length = ReadU16();
	std::string result(length, '\0');
	for (int i = 0; i < length; i++)
	{
		m_InputStream->read(result.data() + i, 1);
	}
	return result;
}