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

#include "mock/gb/cpu/MockGbCpu.h"
#include "qnemu/gb/interrupt/GbInterruptHandler.h"

namespace qnemuTest
{

TEST(GbInterruptTest, readAndWriteRegister)
{
    auto mockGbCpu = std::make_shared<testing::StrictMock<qnemuMock::MockGbCpu>>();
    qnemu::GbInterruptHandler gbInterruptHandler(mockGbCpu);
}


}  // namespace qnemuTest