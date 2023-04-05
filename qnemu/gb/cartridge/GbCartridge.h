/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>
#include <fstream>

#include "qnemu/gb/cartridge/GbCartridgeInterface.h"
#include "qnemu/gb/cartridge/mbc/GbMbcFactoryInterface.h"

namespace qnemu
{

class GbCartridge : public GbCartridgeInterface
{

public:
    GbCartridge() = delete;
    explicit GbCartridge(const GbMbcFactoryInterface& gbMbcFactory);
    ~GbCartridge();

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;

    void load(const char* filePath) override;
    bool isGbcCartridge() const override;

private:
    std::string getPublisherFromCartridge(const std::vector<uint8_t>& buffer) const;
    size_t getRamSizeFromCartridge(uint8_t flag) const;
    size_t getRomSizeFromCartridge(uint8_t flag) const;
    bool validate(const std::vector<uint8_t>& buffer) const;

    std::ifstream file;
    const GbMbcFactoryInterface& gbMbcFactory;
    bool gbcCartridge;
    bool japanCartridge;
    bool loaded;
    std::unique_ptr<GbMbcInterface> mbc;
    std::string publisher;
    size_t ramSize;
    size_t romSize;
    std::string title;
    uint8_t type;
};

}  // namespace qnemu