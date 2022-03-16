/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable:4275 )
#endif
#include <gtest/gtest.h>
#ifdef _MSC_VER
#pragma warning( pop )
#endif

#include "mock/gb/cartridge/mbc/MockGbMbc.h"
#include "mock/gb/cartridge/mbc/MockGbMbcFactory.h"
#include "qnemu/gb/cartridge/GbCartridge.h"

namespace qnemuTest
{

TEST(GbCartridgeTest, TEST1)
{
    testing::StrictMock<qnemuMock::MockGbMbc> mockGbMbc;
    testing::StrictMock<qnemuMock::MockGbMbcFactory> mockGbMbcFactory;

    qnemu::GbCartridge gbCartridge(mockGbMbcFactory);
}

}  // namespace qnemuTest