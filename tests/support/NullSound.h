/*
 *  Copyright [2024] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstddef>
#include <cstdint>

#include "qnemu/sound/SoundInterface.h"

namespace qnemuTest
{

/// Discards APU output; used by the headless ROM runner.
class NullSound : public qnemu::SoundInterface
{
public:
    void enable() override {}
    void disable() override {}
    void addSamples(const int16_t*, std::size_t) override {}
};

}  // namespace qnemuTest
