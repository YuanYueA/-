#ifndef CMDUSERHANDLER_H_
#define CMDUSERHANDLER_H_

#include <iostream>
#include <cstring>
#include <unistd.h>
#include "ThostFtdcMdApi.h"

class CMduserHandler: public CThostFtdcMdSpi{
    public:
        CMduserHandler(std::string user_id, std::string pwd, std::string bro_id):
            user_id_(user_id), pwd_(pwd), bro_id_(bro_id) {}

        void connect(){
            // 初始化 m_mdApi
            m_mdApi = CThostFtdcMdApi::CreateFtdcMdApi("./log/");
            m_mdApi->RegisterSpi(this);
            m_mdApi->RegisterFront("tcp://180.168.146.187:10131");
            m_mdApi->Init();
        }

        void OnFrontConnected() override {
            std::cout << "market server connect success" << std::endl; 
        }

        void login(){
            CThostFtdcReqUserLoginField login_field = {0};
            strcpy(login_field.UserID, user_id_.c_str());
            strcpy(login_field.Password, pwd_.c_str());
            strcpy(login_field.BrokerID, bro_id_.c_str());
            while(m_mdApi->ReqUserLogin(&login_field, request_id++) != 0) sleep(1);
        }

        void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
            std::cout << "user login market server success" << std::endl;
        }

        void logout(){
            CThostFtdcUserLogoutField logout_field = {0};
            strcpy(logout_field.UserID, user_id_.c_str());
            strcpy(logout_field.BrokerID, bro_id_.c_str());
            while(m_mdApi->ReqUserLogout(&logout_field, request_id++) != 0) sleep(1);
        }

        void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
            std::cout << "user logout market server success" << std::endl;
        };

        void subscribe(){
            char **ppInstrument = new char * [50]; 
            ppInstrument[0] = "al2103";
            while(m_mdApi->SubscribeMarketData(ppInstrument, 1) != 0) sleep(1);
        }
        void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData){
            std::cout << "OnPtnDepthMarketData\n";
            std::cout << pDepthMarketData->LastPrice << std::endl;
        }

        void unsubscribe(){
            char **ppInstrument = new char * [50]; 
            ppInstrument[0] = "al2103";
            while(m_mdApi->UnSubscribeMarketData(ppInstrument, 1) != 0) sleep(1);
        }

        void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
            std::cout << "unsubscribe" << std::endl;
        }
    private:
        CThostFtdcMdApi* m_mdApi;
        std::string user_id_;
        std::string pwd_;
        std::string bro_id_;
        int request_id = 1;
};

#endif
