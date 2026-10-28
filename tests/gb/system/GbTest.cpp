/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include <functional>
#include <memory>
#include <utility>

#include <gtest/gtest.h>

#include "qnemu/gb/Gb.h"
#include "qnemu/input/InputInterface.h"
#include "tests/support/MemoryDisplay.h"
#include "tests/support/NullSound.h"

namespace qnemuTest
{

namespace
{

class RecordingInput : public qnemu::InputInterface
{
public:
    void setButtonPressCallback(std::function<void(qnemu::GbButton)> callback) override
    {
        press = std::move(callback);
    }

    void setButtonReleaseCallback(std::function<void(qnemu::GbButton)> callback) override
    {
        release = std::move(callback);
    }

    std::function<void(qnemu::GbButton)> press;
    std::function<void(qnemu::GbButton)> release;
};

}  // namespace

TEST(GbTest, ConstructsWithInjectedHostInterfaces)
{
    auto display = std::make_shared<MemoryDisplay>();
    qnemu::Gb system(qnemu::GbModel::dmg, display,
        std::make_unique<NullSound>(), nullptr);

    system.stop();
    EXPECT_EQ(display->frameCount, 0U);
}

TEST(GbTest, WiresAndClearsInputCallbacks)
{
    auto display = std::make_shared<MemoryDisplay>();
    auto input = std::make_shared<RecordingInput>();

    {
        qnemu::Gb system(qnemu::GbModel::dmg, display,
            std::make_unique<NullSound>(), input);
        EXPECT_TRUE(input->press);
        EXPECT_TRUE(input->release);
    }

    EXPECT_FALSE(input->press);
    EXPECT_FALSE(input->release);
}

}  // namespace qnemuTest
