#include "bencode.h"

BValue::BValue(const std::string & value):type(BType::STRING), vString(value) {}
BValue::BValue(const char * value):type(BType::STRING), vString(value) {}
BValue::BValue(int value):type(BType::INTEGER), vInteger(value) {}
BValue::BValue(const BList & value):type(BType::LIST), vList(new BList(value)) {}
BValue::BValue(const BDict & value):type(BType::DICT), vDict(new BDict(value)) {}

void BValue::release()
{
    if (type == BType::LIST && vList) {
        delete vList;
    } else if (type == BType::DICT && vDict) {
        delete vDict;
    }
}

BValue::BValue(const BValue & value):type(value.type) 
{
    *this = value;
}

BValue & BValue::operator = (const BValue & value)
{
    type = value.type;
    switch (type)
    {
        case BType::STRING: vString = value.vString; break;
        case BType::INTEGER: vInteger = value.vInteger; break;
        case BType::LIST:
            release();
            vList = new BList(*value.vList);
            break;
        case BType::DICT: 
            release();
            vDict= new BDict(*value.vDict);
            break;
    }
    return * this;
}

BValue & BValue::operator = (const std::string & value)
{
    *this = value;
    return *this;
}

BValue & BValue::operator = (const char * value)
{
    *this = value;
    return *this;
}

BValue & BValue::operator = (int value)
{
    *this = value;
    return *this;
}

BValue & BValue::operator = (const BList & value)
{
    *this = value;
    return *this;
}

BValue & BValue::operator = (const BDict & value)
{
    *this = value;
    return *this;
}

BValue::~BValue()
{
    release();
}

std::string encode() const;
bool decode(const std::string & body);


