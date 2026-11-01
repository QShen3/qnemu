/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <memory>
#include <utility>

namespace qnemu
{

/**
 * One end of a virtual Game Boy Color infrared link.
 *
 * The GBC IR port is a raw LED/phototransistor pair, not a protocol engine.
 * The transport only exposes the local LED state and the peer's LED state;
 * each game's software implements its own pulse encoding.
 */
class GbInfraredLink
{
public:
    virtual ~GbInfraredLink() = default;

    virtual void setLed(bool on) = 0;
    virtual bool peerLed() const = 0;
    virtual void reset() = 0;
};

using GbInfraredLinkPair =
    std::pair<std::shared_ptr<GbInfraredLink>, std::shared_ptr<GbInfraredLink>>;

}  // namespace qnemu
