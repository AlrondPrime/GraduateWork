#include "pch.h"

class Byte
{
private:
    std::bitset<8> bitset{0b0000'0001};

public:
    using value_type = uint8_t;

    value_type getAmount()
    {
        return (bitset & std::bitset<8>(0b0111'1111)).to_ulong();
    }

    void setAmount(value_type amount)
    {
        assert(amount <= 127 && "Amount above 127");
        bool temp = getValue();
        bitset = amount;
        setValue(temp);
    }

    bool getValue()
    {
        return bitset.test(7);
    }

    void setValue(bool val)
    {
        bitset.set(7, val);
    }

    void incrementAmount()
    {
        assert(getAmount() < 127 && "Increment above 127");
        setAmount(getAmount() + 1);
    }

    void resetAmount()
    {
        bool temp = getValue();
        bitset = 0b0000'0001;
        setValue(temp);
    }

    Byte &operator=(value_type &val)
    {
        bitset = val;
        return *this;
    }

    Byte &operator=(value_type &&val)
    {
        bitset = val;
        return *this;
    }

    value_type to_uint8_t()
    {
        return bitset.to_ulong();
    }

    std::string to_string()
    {
        return bitset.to_string();
    }

    bool equal(value_type val)
    {
        return val == this->to_uint8_t();
    }

    explicit operator int() const
    {
        return bitset.to_ulong();
    }
};


std::ostream &operator<<(std::ostream &out, Byte byte)
{
    out << byte.to_uint8_t();
    return out;
}

std::ofstream &operator<<(std::ofstream &out, Byte byte)
{
    out << byte.to_uint8_t();
    return out;
}