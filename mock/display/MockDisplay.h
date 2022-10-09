/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <functional>

#include <QtGui/QImage>
#include <gmock/gmock.h>

#include "qnemu/display/DisplayInterface.h"

namespace qnemuMock
{

class MockDisplay : public qnemu::DisplayInterface
{
public:
    MOCK_METHOD(void, enable, (), (override));
    MOCK_METHOD(void, disable, (), (override));
    MOCK_METHOD(void, requestRefresh, (), (override));
    MOCK_METHOD(void, waitFroRefresh, (), (override));
    MOCK_METHOD(QImage&, getBuffer, (), (override));
    MOCK_METHOD(void, setKeyPressCallback, (std::function<void(int)>), (override));
    MOCK_METHOD(void, setKeyReleaseCallback, (std::function<void(int)>), (override));
};

}  // namespace qnemuMock
