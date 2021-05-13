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
    typedef std::map<BString,BValue> BDict;
	struct IterValue
	{
		const std::string & key;
		const BValue & value;
	};

    struct Iterator
    {
        BType type;
        bool operator != (const Iterator & other)
        {
            return type != other.type || listIter != other.listIter || dictIter != other.dictIter;
        }
        
        void operator ++ ()
        {
            if (type == BType::DICT) {
                ++ dictIter;
            }else if (type == BType::LIST) {
                ++ listIter;
            }
        }

		const IterValue operator * () 
		{
			if (type == BType::LIST) {
				return {"", *listIter};
            } else {
				return {dictIter->first, dictIter->second};
            }
		}

        BList::iterator listIter;
        BDict::iterator dictIter;
    };

    BType type = BType::NONE;
    union
    {
        BInteger vInteger;
        BString * vString;
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
            if (type == BType::LIST) {
                vList = new BList;
            }else if (type == BType::DICT) {
                vDict = new BDict;
            }else if (type == BType::STRING) {
				vString = new BString;
			}
        }
    }

    BValue():vDict(nullptr){}
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
        return vDict->operator[](key); 
    }
    void push (const BValue & value) 
    { 
        resetType(BType::LIST);
        return vList->push_back(value);
    }

    Iterator begin() { return {type, vList->begin(), vDict->begin()}; }
    Iterator end() { return {type, vList->end(), vDict->end()}; }

    ~BValue();
};

using BString = BValue::BString;
using BInteger = BValue::BInteger;
using BList= BValue::BList;
using BDict = BValue::BDict;
