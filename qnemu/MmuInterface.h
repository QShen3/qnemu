/* 
 *  Copyright [2023] <qazxdrcssc2006@163.com>
 */

#pragma once

namespace qnemu
{

template<typename ValueType, typename AddressType>
class MmuInterface
{
public:
    virtual ~MmuInterface() = default;

    virtual ValueType read(AddressType address) const = 0;
    virtual void write(AddressType address, const ValueType& value) = 0;
    virtual void step() = 0;
    virtual void reset() = 0;
};

} // namespace qnemu
