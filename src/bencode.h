#pragma once
#include <string>
#include <vector>
#include <map>

enum class BType 
{
    STRING, INTEGER, LIST, DICT, NONE
};




struct BValue
{
    typedef std::string BString;
    typedef int BInteger;
    typedef std::vector<BValue> BList;
    typedef std::map<std::string,BValue> BDict;

    BType type;
    union
    {
        BString vString;
        BInteger vInteger;
        BList * vList;
        BDict * vDict;
    };

    std::string encode() const;
    bool decode(const std::string & body);

    void release();
    void resetType(BType type)
    {
        if (type != this->type) {
            release();
            this->type = type;
        }
    }

    BValue() {}
    BValue(const std::string & value);
    BValue(const char * value);
    BValue(int value);
    BValue(const BList & value);
    BValue(const BDict & value);
    BValue(const BValue & value);

    BValue & operator = (const std::string & value);
    BValue & operator = (const char * value);
    BValue & operator = (int value);
    BValue & operator = (const BList & value);
    BValue & operator = (const BDict & value);
    BValue & operator = (const BValue & value);

    BValue & operator[](const std::string & key) 
    { 
        resetType(BType::DICT);
        return (*vDict)[key]; 
    }
    void push (const BValue & value) 
    { 
        resetType(BType::LIST);
        return vList->push_back(value);
    }

    ~BValue();
};

using BString = BValue::BString;
using BInteger = BValue::BInteger;
using BList= BValue::BList;
using BDict = BValue::BDict;
