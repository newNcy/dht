#include "bencode.h"
#include <sstream>
#include <string>
#include <cctype>

BValue::BValue(const std::string & value):type(BType::STRING), vString(new BString(value)) {}
BValue::BValue(const char * value):type(BType::STRING), vString(new BString(value)) {}
BValue::BValue(int value):type(BType::INTEGER), vInteger(value) {}
BValue::BValue(const BList & value):type(BType::LIST), vList(new BList(value)) {}
BValue::BValue(const BDict & value):type(BType::DICT), vDict(new BDict(value)) {}

void BValue::release()
{
    if (type == BType::LIST && vList) {
        delete vList;
    } else if (type == BType::DICT && vDict) {
        delete vDict;
    } else if (type == BType::STRING && vString) {
        delete vString;
	}
}

BValue::BValue(const BValue & value)
{
    *this = value;
}

BValue & BValue::operator = (const BValue & value)
{
	resetType(value.type);
    switch (value.type)
    {
        case BType::INTEGER: vInteger = value.vInteger; break;
        case BType::STRING: *vString = *value.vString; break;
        case BType::LIST: *vList = *value.vList; break;
        case BType::DICT:  *vDict= *value.vDict; break;
    }
    return * this;
}

BValue & BValue::operator = (const std::string & value)
{
    resetType(BType::STRING);
    *vString = value;
    return *this;
}

BValue & BValue::operator = (const char * value)
{
    resetType(BType::STRING);
    *vString = value;
    return *this;
}

BValue & BValue::operator = (int value)
{
    resetType(BType::INTEGER);
    vInteger = value;
    return *this;
}

BValue & BValue::operator = (const BList & value)
{
    resetType(BType::LIST);
    *vList = value;
    return *this;
}

BValue & BValue::operator = (const BDict & value)
{
    resetType(BType::DICT);
    *vDict = value;
    return *this;
}

BValue::~BValue()
{
    release();
}

std::string BValue::encode() const
{
    std::stringstream ss;
    if (type == BType::STRING)
    {
		ss<<vString->length()<<':'<<*vString;
    } else if (type == BType::INTEGER) {
        ss<<'i'<<vInteger<<'e';
    } else if (type == BType::LIST) {
		ss<<'l';
		for (auto & v : *vList) {
			ss<<v.encode();
		}
		ss<<'e';
	} else if (type == BType::DICT) {
		ss<<'d';
		for (auto & v : *vDict) {
			ss<<BValue(v.first).encode()<<v.second.encode();
		}
		ss<<'e';
	}

    return ss.str();
}

bool BValue::decode(const std::string & bytes, int & pos)
{
	auto readInt = [&]()
	{
		int ret = 0;
		while (pos < bytes.length() && std::isdigit(bytes[pos])) {
			ret = ret * 10 + bytes[pos] - '0';
			pos ++;
		}

		return ret;
	};
	switch(bytes[pos++])
	{
		case 'i': 
			*this = readInt();
			pos ++;
			break;
		case 'l': 
			while (pos != bytes.length() && bytes[pos] != 'e') {
				BValue value;
				value.decode(bytes, pos);
				push(value);
			}
			if (pos != bytes.length()) pos ++;
			break;
		case 'd':
			while (pos != bytes.length() && bytes[pos] != 'e') {
				BValue key, value;
				key.decode(bytes, pos);
				value.decode(bytes, pos);
				this->operator [] (*key.vString) = value;
			}
			if (pos != bytes.length()) pos ++;
			break;
		default: 
			--pos;
			int length = readInt();
			* this = BString(bytes, pos + 1, length);
			pos += length + 1;
			break;
	}
    return true;
}


