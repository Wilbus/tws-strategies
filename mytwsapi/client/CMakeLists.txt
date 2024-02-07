cmake_minimum_required(VERSION 3.10)
project(mytwsapi)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

qt_add_library(${PROJECT_NAME} SHARED
    ${CMAKE_CURRENT_LIST_DIR}/src/ContractCondition.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/Decimal.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/DefaultEWrapper.cpp

    ${CMAKE_CURRENT_LIST_DIR}/src/EClient.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/EClientSocket.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/EDecoder.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/EMessage.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/EMutex.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/EOrderDecoder.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/EReader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/EReaderOSSignal.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/ESocket.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/executioncondition.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/MarginCondition.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/OperatorCondition.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/OrderCondition.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/PercentChangeCondition.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/PriceCondition.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/SoftDollarTier.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/StdAfx.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/TimeCondition.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/Utils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/VolumeCondition.cpp

#includes for qtcreator to find in ide
    include/bar.h
    include/CommissionReport.h
    include/CommonDefs.h
    include/Contract.h
    include/ContractCondition.h
    include/Decimal.h
    include/DefaultEWrapper.h
    include/DepthMktDataDescription.h
    include/EClient.h
    include/EClientException.h
    include/EClientMsgSink.h
    include/EClientSocket.h
    include/EDecoder.h
    include/EMessage.h
    include/EMutex.h
    include/EOrderDecoder.h
    include/EPosixClientSocketPlatform.h
    include/EReader.h
    include/EReaderOSSignal.h
    include/EReaderSignal.h
    include/ESocket.h
    include/ETransport.h
    include/EWrapper_prototypes.h
    include/EWrapper.h
    include/Execution.h
    include/executioncondition.h
    include/FamilyCode.h
    include/HistogramEntry.h
    include/HistoricalSession.h
    include/HistoricalTick.h
    include/HistoricalTickBidAsk.h
    include/HistoricalTickLast.h
    include/IExternalizable.h
    include/MarginCondition.h
    include/NewsProvider.h
    include/OperatorCondition.h
    include/Order.h
    include/OrderCondition.h
    include/OrderState.h
    include/PercentChangeCondition.h
    include/platformspecific.h
    include/PriceCondition.h
    include/PriceIncrement.h
    include/resource.h
    include/ScannerSubscription.h
    include/SoftDollarTier.h
    include/StdAfx.h
    include/TagValue.h
    include/TickAttrib.h
    include/TickAttribBidAsk.h
    include/TickAttribLast.h
    include/TimeCondition.h
    include/TwsSocketClientErrors.h
    include/Utils.h
    include/VolumeCondition.h
    include/WshEventData.h
)

target_include_directories(mytwsapi PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)



target_compile_options(mytwsapi PRIVATE -Wall -Wextra -Wfatal-errors -Wunused-parameter -Wunused-variable -DLOG_FORMAT_VERIFICATION)

#get the libintelrdfpmath-dev_2.0u2-6_amd64.deb package and install first
#or bulild it locally and paste it into the lib folder for linking
target_link_libraries(mytwsapi /usr/lib/x86_64-linux-gnu/libbidgcc000.a)
