/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <vector>

#include "qnemu/gb/cartridge/GbCartridgeInterface.h"
#include "qnemu/gb/cartridge/mbc/GbMbcFactoryInterface.h"
#include "qnemu/save/SaveTarget.h"

namespace qnemu
{

class GbCartridge : public GbCartridgeInterface
{

public:
    GbCartridge() = delete;
    explicit GbCartridge(const GbMbcFactoryInterface& gbMbcFactory);
    ~GbCartridge() override;

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;

    void load(const char* filePath) override;
    bool isGbcCartridge() const override;

    bool hasBattery() const override;
    std::size_t ramSize() const override;
    std::vector<uint8_t> dumpRam() const override;
    void loadRam(const std::vector<uint8_t>& data) override;

    bool hasRtc() const override;
    std::vector<uint8_t> dumpRtc() override;
    void loadRtc(const std::vector<uint8_t>& data) override;
    void advanceRtc(uint64_t seconds) override;

private:
    static std::string getPublisherFromCartridge(const std::vector<uint8_t>& buffer);
    static size_t getRamSizeFromCartridge(uint8_t flag);
    static size_t getRomSizeFromCartridge(uint8_t flag);
    bool validate(const std::vector<uint8_t>& buffer) const;

    std::ifstream file;
    const GbMbcFactoryInterface& gbMbcFactory;
    bool gbcCartridge;
    bool japanCartridge;
    bool loaded;
    std::unique_ptr<GbMbcInterface> mbc;
    std::string publisher;
    size_t ramSizeBytes;
    size_t romSize;
    std::string title;
    uint8_t type;
};

}  // namespace qnemu