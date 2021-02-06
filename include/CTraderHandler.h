/* 
 * ReqOrderAction,ReqOrderInsert,ReqQryExchange,ReqQryExchangeMarginRate
 * OnRtnOrder,OnRtnTrade,OnRspQryExchange,OnRspQryExchangeMarginRate
 * ReqQryExchangeRate,ReqQryInstrumentCommissionRate,ReqQryInstrumentMarginRate
 * OnRspQryExchangeRate,OnRspQryInstrumentCommissionRate,OnRspQryInstrumentMarginRate
 * ReqQryOrder,ReqQryTrade,ReqQryTradingAccount
 * OnRspQryOrder,OnRspQryTrade,OnRspQryTradingAccount
 */

#include <iostream>
#include <atomic>
#include <vector>
#include <cstring>
#include <unistd.h>
#include "ThostFtdcTraderApi.h"

class CTraderHandler : public CThostFtdcTraderSpi{
    public:
        CTraderHandler():request_id(1){}

        void connect(){
            // 初始化 m_mdApi
            m_ptraderapi = CThostFtdcTraderApi::CreateFtdcTraderApi("./log/");
            m_ptraderapi->RegisterSpi(this);
            m_ptraderapi->RegisterFront("tcp://180.168.146.187:10130");
            m_ptraderapi->SubscribePublicTopic(THOST_TERT_QUICK);
            m_ptraderapi->SubscribePrivateTopic(THOST_TERT_RESTART); //设置私有流订阅模式
            m_ptraderapi->Init();
        }

        void OnFrontConnected() override {
            std::cout << "trade server connect success" << std::endl; 
        }

        void getVersion(){
            std::cout << "The api version is " << m_ptraderapi->GetApiVersion() << std::endl;
        }

        void login(std::string user_id, std::string pwd, std::string bro_id){
            CThostFtdcReqUserLoginField login_field = {0};
            strcpy(login_field.UserID, user_id.c_str());
            strcpy(login_field.Password, pwd.c_str());
            strcpy(login_field.BrokerID, bro_id.c_str());
            while (m_ptraderapi->ReqUserLogin(&login_field, request_id++)!=0) sleep(1);
        }

        void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
            std::cout << "user login trade server success" << std::endl;
        }

        void logout(std::string user_id, std::string bro_id){
            CThostFtdcUserLogoutField logout_field = { 0 };
            strcpy(logout_field.BrokerID, bro_id.c_str());
            strcpy(logout_field.UserID, user_id.c_str());
            while (m_ptraderapi->ReqUserLogout(&logout_field, request_id++)!=0) sleep(1);
        }

        void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
            std::cout << "user logout trade server success" << std::endl;
        };

        //获取交易日信息
        void getTday(){
            std::cout << "Trading day is " << m_ptraderapi->GetTradingDay() << std::endl;
        };

        //请求查询合约
        void qryInstrument(){
            CThostFtdcQryInstrumentField inst_field = {0};
            m_ptraderapi->ReqQryInstrument(&inst_field, request_id++);
        }
        void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override {
            //std::cout << pInstrument->InstrumentID << std::endl; 
            insts_vec.push_back(std::move(*pInstrument));
        }

        //询价录入请求
        void quoteInsert(std::string bro_id, std::string iner, std::string inst, std::string user_id, std::string ex_id){
            CThostFtdcInputForQuoteField quote_field = {0};
            strcpy(quote_field.UserID, user_id.c_str());
            strcpy(quote_field.ExchangeID, ex_id.c_str());
            strcpy(quote_field.InstrumentID, inst.c_str());
            strcpy(quote_field.BrokerID, bro_id.c_str());
            strcpy(quote_field.InvestorID, iner.c_str());
            m_ptraderapi->ReqForQuoteInsert(&quote_field, request_id++);
        }

        void OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp) override{
            std::cout << "quote rsp:" << std::endl;
            std::cout << "pForQuoteRsp->ForQuoteSysID" << std::endl;
        }

        //期货发起银行资金转期货请求
        void bankTofut(double output_num){
            CThostFtdcReqTransferField tran_field = {0};
            strcpy(tran_field.TradeCode, "202001"); //业务功能码
            strcpy(tran_field.BankID, "1"); 
            strcpy(tran_field.BankBranchID, "0000");
            strcpy(tran_field.BrokerID, "9999");
            tran_field.LastFragment = THOST_FTDC_LF_Yes;///最后分片标志 '0'=是最后分片
            tran_field.IdCardType = THOST_FTDC_ICT_IDCard;///证件类型
            tran_field.CustType = THOST_FTDC_CUSTT_Person;///客户类型
            strcpy(tran_field.BankAccount, "621485212110187");
            strcpy(tran_field.AccountID, "1000001");///投资者帐号
            strcpy(tran_field.Password, "123456");///期货密码--资金密码
            tran_field.InstallID = 1;///安装编号
            tran_field.FutureSerial = 0;///期货公司流水号
            tran_field.VerifyCertNoFlag = THOST_FTDC_YNI_No;///验证客户证件号码标志
            strcpy(tran_field.CurrencyID, "CNY");///币种代码
            tran_field.TradeAmount = output_num;///转帐金额
            tran_field.FutureFetchAmount = 0;///期货可取金额
            tran_field.CustFee = 0;///应收客户费用
            tran_field.BrokerFee = 0;///应收期货公司费用
            tran_field.SecuPwdFlag = THOST_FTDC_BPWDF_BlankCheck;///期货资金密码核对标志
            tran_field.RequestID = 0;///请求编号
            tran_field.TID = 0;///交易ID
            m_ptraderapi->ReqFromBankToFutureByFuture(&tran_field, request_id++);
        }
        
        void OnRtnFromBankToFutureByFuture(CThostFtdcRspTransferField *pRspTransfer) override{
            std::cout << "tansfer from bank to future success" << std::endl;
        }

        void futTobank(double output_num, int SessionID){
            CThostFtdcReqTransferField tran_field = {0};
            strcpy(tran_field.TradeCode, "202002"); //业务功能码
            strcpy(tran_field.BankID, "1"); 
            strcpy(tran_field.BankBranchID, "0000");
            strcpy(tran_field.BrokerID, "9999");
            tran_field.LastFragment = THOST_FTDC_LF_Yes;///最后分片标志 '0'=是最后分片
            tran_field.SessionID = SessionID;
            tran_field.IdCardType = THOST_FTDC_ICT_IDCard;///证件类型
            strcpy(tran_field.IdentifiedCardNo, "310110198701011914");///证件号码
            strcpy(tran_field.BankAccount, "621485212110187");
            strcpy(tran_field.AccountID, "1000001");///投资者帐号
            strcpy(tran_field.Password, "123456");///期货密码--资金密码
            tran_field.InstallID = 1;///安装编号
            tran_field.CustType = THOST_FTDC_CUSTT_Person;
            tran_field.VerifyCertNoFlag = THOST_FTDC_YNI_No;///验证客户证件号码标志
            strcpy(tran_field.CurrencyID, "CNY");///币种代码
            tran_field.TradeAmount = output_num;///转帐金额
            tran_field.FutureFetchAmount = 0;///期货可取金额
            tran_field.CustFee = 0;///应收客户费用
            tran_field.BrokerFee = 0;///应收期货公司费用
            tran_field.RequestID = 0;///请求编号
            tran_field.TID = 0;///交易ID
            m_ptraderapi->ReqFromFutureToBankByFuture(&tran_field, request_id++);
        }
        void OnRtnFromFutureToBankByFuture(CThostFtdcRspTransferField *pRspTransfer) override{
            std::cout << "tansfer from future to bank success" << std::endl;
        }

        //期权自对冲操作请求
        void optSelfaction(){
            CThostFtdcInputOptionSelfCloseActionField optse_field = { 0 };
            strcpy(optse_field.BrokerID, "9999");
            strcpy(optse_field.InvestorID, "1000001"); 
            strcpy(optse_field.OptionSelfCloseRef, "000000258");//期权自对冲引用
            optse_field.FrontID = 1;
            optse_field.SessionID = 6442531;
            strcpy(optse_field.ExchangeID, "SHFE");
            optse_field.ActionFlag = THOST_FTDC_AF_Delete;
            strcpy(optse_field.UserID, "1000001");
            strcpy(optse_field.InstrumentID, "rb1809");
            m_ptraderapi->ReqOptionSelfCloseAction(&optse_field, request_id++);
        }

        void OnRtnOptionSelfClose(CThostFtdcOptionSelfCloseField *pOptionSelfClose) override{

        }

        //期权自对冲录入请求
        void optSelfinsert(){
            CThostFtdcInputOptionSelfCloseField optse_field = { 0 };
            strcpy(optse_field.BrokerID, "9999");
            strcpy(optse_field.InvestorID, "1000001"); 
            strcpy(optse_field.InstrumentID, "rb1809");
            strcpy(optse_field.UserID, "1000001");
            optse_field.Volume = 1;
            optse_field.HedgeFlag = THOST_FTDC_HF_Speculation;
            optse_field.OptSelfCloseFlag = THOST_FTDC_OSCF_CloseSelfOptionPosition;
            strcpy(optse_field.ExchangeID, "SHFE");
            strcpy(optse_field.AccountID, "1000001");
            strcpy(optse_field.CurrencyID, "CNY");
            m_ptraderapi->ReqOptionSelfCloseInsert(&optse_field, request_id++);
        }

        //报单操作请求
        void reqOrderaction(std::string order_id){
            CThostFtdcInputOrderActionField inorder_field = { 0 };
            strcpy(inorder_field.BrokerID, "9999");
            strcpy(inorder_field.InvestorID, "1000001");
            strcpy(inorder_field.UserID, "1000001");
            strcpy(inorder_field.OrderSysID, order_id.c_str());  //对应要撤报单的OrderSysID
            strcpy(inorder_field.ExchangeID, "SHFE");
            strcpy(inorder_field.InstrumentID, "rb1809");
            inorder_field.ActionFlag = THOST_FTDC_AF_Delete;
            m_ptraderapi->ReqOrderAction(&inorder_field, request_id++);
        }
        void reqOrderaction(int front_id, int sess_id, std::string order_ref){
            CThostFtdcInputOrderActionField inorder_field = { 0 };
            strcpy(inorder_field.BrokerID, "9999");
            strcpy(inorder_field.InvestorID, "1000001");
            strcpy(inorder_field.UserID, "1000001");
            inorder_field.FrontID = 1;   //对应要撤报单的FrontID
            inorder_field.SessionID = -788541;   //对应要撤报单的sessionid
            strcpy(inorder_field.OrderRef, order_ref.c_str());  //对应要撤报单的OrderSysID
            strcpy(inorder_field.ExchangeID, "SHFE");
            strcpy(inorder_field.InstrumentID, "rb1809");
            inorder_field.ActionFlag = THOST_FTDC_AF_Delete;
            m_ptraderapi->ReqOrderAction(&inorder_field, request_id++);
        }

        void OnRtnOrder(CThostFtdcOrderField *pOrder) override {

        };

        //报单录入请求
        void reqOrderinsert(){
            CThostFtdcInputOrderField ord_field = { 0 };
            strcpy(ord_field.BrokerID, "0000");
            strcpy(ord_field.InvestorID, "00001"); 
            strcpy(ord_field.ExchangeID, "SHFE");
            strcpy(ord_field.InstrumentID, "ag1801");
            strcpy(ord_field.UserID, "00001");
            ord_field.OrderPriceType = THOST_FTDC_OPT_LimitPrice;//限价
            ord_field.Direction = THOST_FTDC_D_Buy;//买
            ord_field.CombOffsetFlag[0] = THOST_FTDC_OF_Open;//开
            ord_field.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;//投机
            ord_field.LimitPrice = 100;
            ord_field.VolumeTotalOriginal = 1;
            ord_field.TimeCondition = THOST_FTDC_TC_GFD;///当日有效
            ord_field.VolumeCondition = THOST_FTDC_VC_AV;///任意数量
            ord_field.MinVolume = 1;
            ord_field.ContingentCondition = THOST_FTDC_CC_Immediately;
            ord_field.StopPrice = 0;
            ord_field.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
            ord_field.IsAutoSuspend = 0;
            m_ptraderapi->ReqOrderInsert(&ord_field, request_id++);
        }
        void OnRtnTrade(CThostFtdcTradeField *pTrade) override {

        };

        //预埋撤单请求
        void reqParkorderAction(){
            CThostFtdcParkedOrderActionField parord_field = { 0 };
            strcpy(parord_field.BrokerID, "9999");
            strcpy(parord_field.InvestorID, "1000001");
            strcpy(parord_field.ExchangeID, "SHFE");
            strcpy(parord_field.OrderSysID, "    10061782"); 
            strcpy(parord_field.UserID, "1000001");
            strcpy(parord_field.InstrumentID, "rb1809");
            parord_field.ActionFlag = THOST_FTDC_AF_Delete;
            m_ptraderapi->ReqParkedOrderAction(&parord_field, request_id++);
        }

        void traversalInsts(){
            for(auto it = insts_vec.begin(); it != insts_vec.end(); ++it){
                std::cout << it->InstrumentID << std::endl;
            }
            std::cout << insts_vec.size() << std::endl;
        }


    private:
        CThostFtdcTraderApi *m_ptraderapi;
        std::atomic<int> request_id;
        std::vector<CThostFtdcInstrumentField> insts_vec;
};