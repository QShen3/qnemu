/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include "qnemu/gb/infrared/GbLocalInfraredLink.h"

#include <memory>
#include <utility>

namespace qnemu
{

GbInfraredLinkPair createLocalInfraredLinkPair()
{
    const auto state = std::make_shared<GbLocalInfraredLink::SharedState>();
    return {
        std::make_shared<GbLocalInfraredLink>(state, 0),
        std::make_shared<GbLocalInfraredLink>(state, 1),
    };
}

GbLocalInfraredLink::GbLocalInfraredLink(std::shared_ptr<SharedState> sharedState,
    int endpointIndex) :
    state(std::move(sharedState)),
    index(endpointIndex)
{
}

void GbLocalInfraredLink::setLed(bool on)
{
    state->led[index].store(on);
}

bool GbLocalInfraredLink::peerLed() const
{
    return state->led[index ^ 1].load();
}

void GbLocalInfraredLink::reset()
{
    state->led[index].store(false);
}

}  // namespace qnemu
