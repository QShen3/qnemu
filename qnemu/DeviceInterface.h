/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

namespace qnemu
{

template<typename ValueType, typename AddressType>
class DeviceInterface
{

public:
    virtual ~DeviceInterface() = default;

    virtual ValueType read(AddressType address) const = 0;
    virtual void write(AddressType address, const ValueType& value) = 0;
    virtual void reset() = 0;
    virtual void step() = 0;
    virtual bool accepts(AddressType address) const = 0;
};

}  // namespace qnemu
