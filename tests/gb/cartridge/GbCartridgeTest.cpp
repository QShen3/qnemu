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

#include <QtCore/QFile>
#include <QtCore/QTemporaryDir>

#include "mock/gb/cartridge/mbc/MockGbMbc.h"
#include "mock/gb/cartridge/mbc/MockGbMbcFactory.h"
#include "qnemu/gb/cartridge/GbCartridge.h"

namespace qnemuTest
{

TEST(GbCartridgeTest, TEST1)
{
    const testing::StrictMock<qnemuMock::MockGbMbc> mockGbMbc;
    const testing::StrictMock<qnemuMock::MockGbMbcFactory> mockGbMbcFactory;

    const qnemu::GbCartridge gbCartridge(mockGbMbcFactory);
}

TEST(GbCartridgeTest, ReadHeaderMetadata)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("header.gb"));
    QFile file(path);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
    QByteArray data(0x150, 0);
    const QByteArray title("TETRIS");
    data.replace(0x134, title.size(), title);
    data[0x143] = static_cast<char>(0x80);
    data[0x147] = static_cast<char>(0x00);
    data[0x14A] = static_cast<char>(0x01);
    data[0x14B] = static_cast<char>(0x31);
    ASSERT_EQ(file.write(data), data.size());
    file.close();

    const qnemu::GbCartridgeHeader header =
        qnemu::GbCartridge::readHeader(path.toStdString());
    EXPECT_EQ(header.title, "TETRIS");
    EXPECT_EQ(header.publisher, "Nintendo");
    EXPECT_EQ(header.region, "World");
    EXPECT_TRUE(header.cgb);
    EXPECT_EQ(header.titleBytes.at(0), static_cast<uint8_t>('T'));
    EXPECT_EQ(header.oldLicensee, 0x31);
    EXPECT_EQ(header.newLicensee.at(0), 0);
}

}  // namespace qnemuTest