/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <atomic>
#include <memory>

#include "qnemu/gb/infrared/GbInfraredLink.h"

namespace qnemu
{

/// Creates two endpoints that can see each other's IR LED.
GbInfraredLinkPair createLocalInfraredLinkPair();

class GbLocalInfraredLink final : public GbInfraredLink
{
public:
    struct SharedState
    {
        std::atomic<bool> led[2] {};
    };

    GbLocalInfraredLink(std::shared_ptr<SharedState> state, int index);

    void setLed(bool on) override;
    bool peerLed() const override;
    void reset() override;

private:
    std::shared_ptr<SharedState> state;
    int index;
};

}  // namespace qnemu
