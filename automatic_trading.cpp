//口座縛り
//通貨縛り
#property strict
extern string heading1 = "***パスワード***";
input int input_password;                               //パスワード
int password;                                           //パスワード

extern string heading2 = "***設定***";
extern double input_TP = 50.0;                          //テイクプロフィット
extern double input_SL = 10.0;                          //ストップロス
extern double input_TL = 2.0;                           //トレール幅
extern double input_TS = 7.0;                           //トレールスタート
extern int input_DAY = 14;                              //日数
string symbol_check = "GBPUSD";                         //通貨ペア

extern string heading3 = "***資金管理***";
extern double lots = 1;                                 //ロット
input double slippage = 0.5;                            //スリッページ(Pips)
int magic = 1;                                          //マジックナンバー
datetime flagD, flagD2;                                 //一日一回フラグ
int res;                                                //処理結果格納用
int ticket;                                             //処理結果格納用
int i;                                                  //変数使い回し用

int OnInit(){
    //口座チェック___________________________________________________________________________________________________________________________________________________________________________________
    password = PASSWORD();                       //パスワードセッティング
    if(input_password != password){
        Alert("Auth failed\n\n","パスワードが違います。");
        Print("パスワードが違います。");
        return(INIT_FAILED);
    }
    if(IsTesting() == true){
        if(symbol_check != StringSubstr(Symbol(), 0, 6)){
            Print("このEAはGBPUSDのみでバックテストが可能です");
            return(INIT_FAILED);
        }
    }
    Alert("SUCCESS\n\n","パスワードOK");
    return(INIT_SUCCEEDED);
}

void OnTick(){
    //８時以降に一日一回処理する 1440 = 60 * 24________________________________________________________________________________________________________________________________________________________
    if(Hour() >= 8 && flagD != iTime(0, 1440, 0)){
        double High1 = iHigh(0, 1440, 0); //当日高値
        double Low1 = iLow(0, 1440, 0); //当日安値
        //5日間の高値、安値を探す
        for(i = 1; i <= input_DAY; i++){
            double High2 = iHigh(0,1440,i); //i日高値（５日間のうち）
            double Low2 = iLow(0,1440,i); //i日安値（５日間のうち）

            if(High1 < High2){ //当日より過去の方が高い（高値ブレイクラインに逆指値注文を入れる）
                ticket = OrderSend(Symbol(), OP_BUYSTOP, lots, High2, int(slippage * 10), High2 - input_SL *10 * Point(), High2 + input_TP *10 * Point(), "", magic, 0, clrMagenta); //逆指値買い注文
                if(ticket < 0) Print("OrderSend failed with error #", GetLastError()); else PlaySound("ok.wav"); //エラー処理
                High1 = High2; //High1をi日高値に更新する
            }

            if(Low1 > Low2){ //当日より過去の方が低い（安値ブレイクラインに逆指値注文を入れる）
                ticket = OrderSend(Symbol(), OP_SELLSTOP, lots, Low2, int(slippage * 10), Low2 + input_SL *10 * Point(), Low2 - input_TP *10 * Point(), "", magic, 0, clrMagenta); //逆指値売り注文
                if(ticket < 0) Print("OrderSend failed with error #", GetLastError()); else PlaySound("ok.wav"); //エラー処理
                Low1 = Low2; //Low1をi日安値に更新する
            }
        }
        flagD = iTime(0,1440,0); //フラグ更新
    }

    //19時以降に一日一回する処理_______________________________________________________________________________________________________________________________________________________________________
    if(Hour() >= 19 && flagD2 != iTime(0,1440,0)){
        Delete_Symbol(); //保留中の注文を取り消す
        Close_Symbol(); //エントリー中の注文を決済する
        flagD2 = iTime(0,1440,0); //フラグ更新
    }
    if(Hour() >= 8 && Hour() < 19)Trail_Symbol();
}

//関数///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//待機中の注文全決済_________________________________________________________________________________________
void Delete_Symbol(){
    for(i = OrdersTotal() - 1; i >= 0; i--){
        res = OrderSelect(i, SELECT_BY_POS);
        if(OrderSymbol() != Symbol() || OrderMagicNumber() != magic)
        continue; //通貨、マジックナンバー違いは処理スキップ（次の i へ）
        res = OrderDelete(OrderTicket(), clrYellowGreen);
    }
}
//エントリー中の注文全決済____________________________________________________________________________________
void Close_Symbol(){
    for(i = OrdersTotal() - 1; i >= 0; i--){
        res = OrderSelect(i, SELECT_BY_POS);
        if(OrderSymbol() != Symbol() || OrderMagicNumber() != magic)
        continue; //通貨、マジックナンバー違いは処理スキップ（次の i へ）
        res = OrderClose(OrderTicket(), OrderLots(), OrderClosePrice(), int(slippage * 10), clrYellowGreen);
    }
}
//トレール__________________________________________________________________________________________________
void Trail_Symbol(){
    for(i = OrdersTotal() - 1; i >= 0; i--){
        res = OrderSelect(i, SELECT_BY_POS);
        if(OrderSymbol() != Symbol() || OrderMagicNumber() != magic)
            continue; //通貨、マジックナンバー違いは処理スキップ（次の i へ）
        if(OrderType()==OP_BUY && OrderStopLoss()+input_TL*10*Point() < Bid && OrderOpenPrice()+input_TS*10*Point() < Bid)
            res = OrderModify(OrderTicket(),OrderOpenPrice(),Bid - input_TL*10*Point(),Bid + input_TL*10*Point(),OrderExpiration(),clrYellowGreen);
        if(OrderType()==OP_SELL && OrderStopLoss()-input_TL*10*Point() > Ask && OrderOpenPrice()-input_TS*10*Point() > Ask)
            res = OrderModify(OrderTicket(),OrderOpenPrice(),Ask + input_TL*10*Point(),Ask + input_TL*10*Point(),OrderExpiration(),clrYellowGreen);
    }
}
//口座縛り__________________________________________________________________________________________________
int PASSWORD(){
    //口座の1番先頭*5+2番目*2+3番目*7
    int func_pass = StringToInteger(StringSubstr(IntegerToString(AccountNumber()),0,1))*5+
    StringToInteger(StringSubstr(IntegerToString(AccountNumber()),1,1))*2+
    StringToInteger(StringSubstr(IntegerToString(AccountNumber()),2,1))*7;
    return(func_pass);
}