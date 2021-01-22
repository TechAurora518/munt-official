// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#include "NJSIWalletListener.hpp"
using namespace std;

void NJSIWalletListener::notifyBalanceChange_aimpl__(const BalanceRecord & new_balance)
{
    const auto& env = Env();
    Napi::HandleScope scope(env);
    //Wrap parameters
    std::vector<napi_value> args;
    auto arg_0 = Napi::Object::New(env);
    auto arg_0_1 = Napi::Value::From(env, new_balance.availableIncludingLocked);
    arg_0.Set("availableIncludingLocked", arg_0_1);
    auto arg_0_2 = Napi::Value::From(env, new_balance.availableExcludingLocked);
    arg_0.Set("availableExcludingLocked", arg_0_2);
    auto arg_0_3 = Napi::Value::From(env, new_balance.availableLocked);
    arg_0.Set("availableLocked", arg_0_3);
    auto arg_0_4 = Napi::Value::From(env, new_balance.unconfirmedIncludingLocked);
    arg_0.Set("unconfirmedIncludingLocked", arg_0_4);
    auto arg_0_5 = Napi::Value::From(env, new_balance.unconfirmedExcludingLocked);
    arg_0.Set("unconfirmedExcludingLocked", arg_0_5);
    auto arg_0_6 = Napi::Value::From(env, new_balance.unconfirmedLocked);
    arg_0.Set("unconfirmedLocked", arg_0_6);
    auto arg_0_7 = Napi::Value::From(env, new_balance.immatureIncludingLocked);
    arg_0.Set("immatureIncludingLocked", arg_0_7);
    auto arg_0_8 = Napi::Value::From(env, new_balance.immatureExcludingLocked);
    arg_0.Set("immatureExcludingLocked", arg_0_8);
    auto arg_0_9 = Napi::Value::From(env, new_balance.immatureLocked);
    arg_0.Set("immatureLocked", arg_0_9);
    auto arg_0_10 = Napi::Value::From(env, new_balance.totalLocked);
    arg_0.Set("totalLocked", arg_0_10);

    args.push_back(arg_0);
    Napi::Value calling_function_as_value = Value().Get("notifyBalanceChange");
    if(!calling_function_as_value.IsUndefined() && !calling_function_as_value.IsNull())
    {
        Napi::Function calling_function = calling_function_as_value.As<Napi::Function>();
        auto result_notifyBalanceChange = calling_function.Call(args);
        if(result_notifyBalanceChange.IsEmpty())
        {
            Napi::Error::New(env, "NJSIWalletListener::notifyBalanceChange call failed").ThrowAsJavaScriptException();
            return;
        }
    }
}

void NJSIWalletListener::notifyBalanceChange(const BalanceRecord & new_balance)
{
    uv_work_t* request = new uv_work_t;
    request->data = new std::tuple<NJSIWalletListener*, BalanceRecord>(this, new_balance);

    uv_queue_work(uv_default_loop(), request, [](uv_work_t*) -> void{}, [](uv_work_t* req, int status) -> void
    {
        NJSIWalletListener* pthis = std::get<0>(*((std::tuple<NJSIWalletListener*, BalanceRecord>*)req->data));
        pthis->notifyBalanceChange_aimpl__(std::get<1>(*((std::tuple<NJSIWalletListener*, BalanceRecord>*)req->data)));
        delete (std::tuple<NJSIWalletListener*, BalanceRecord>*)req->data;
        req->data = nullptr;
    }
    );
}

void NJSIWalletListener::notifyNewMutation_aimpl__(const MutationRecord & mutation, bool self_committed)
{
    const auto& env = Env();
    Napi::HandleScope scope(env);
    //Wrap parameters
    std::vector<napi_value> args;
    auto arg_0 = Napi::Object::New(env);
    auto arg_0_1 = Napi::Value::From(env, mutation.change);
    arg_0.Set("change", arg_0_1);
    auto arg_0_2 = Napi::Value::From(env, mutation.timestamp);
    arg_0.Set("timestamp", arg_0_2);
    auto arg_0_3 = Napi::String::New(env, mutation.txHash);
    arg_0.Set("txHash", arg_0_3);
    auto arg_0_4 = Napi::String::New(env, mutation.recipient_addresses);
    arg_0.Set("recipient_addresses", arg_0_4);
    auto arg_0_5 = Napi::Value::From(env, (int)mutation.status);
    arg_0.Set("status", arg_0_5);
    auto arg_0_6 = Napi::Value::From(env, mutation.depth);
    arg_0.Set("depth", arg_0_6);

    args.push_back(arg_0);
    auto arg_1 = Napi::Value::From(env, self_committed);
    args.push_back(arg_1);
    Napi::Value calling_function_as_value = Value().Get("notifyNewMutation");
    if(!calling_function_as_value.IsUndefined() && !calling_function_as_value.IsNull())
    {
        Napi::Function calling_function = calling_function_as_value.As<Napi::Function>();
        auto result_notifyNewMutation = calling_function.Call(args);
        if(result_notifyNewMutation.IsEmpty())
        {
            Napi::Error::New(env, "NJSIWalletListener::notifyNewMutation call failed").ThrowAsJavaScriptException();
            return;
        }
    }
}

void NJSIWalletListener::notifyNewMutation(const MutationRecord & mutation, bool self_committed)
{
    uv_work_t* request = new uv_work_t;
    request->data = new std::tuple<NJSIWalletListener*, MutationRecord, bool>(this, mutation, self_committed);

    uv_queue_work(uv_default_loop(), request, [](uv_work_t*) -> void{}, [](uv_work_t* req, int status) -> void
    {
        NJSIWalletListener* pthis = std::get<0>(*((std::tuple<NJSIWalletListener*, MutationRecord, bool>*)req->data));
        pthis->notifyNewMutation_aimpl__(std::get<1>(*((std::tuple<NJSIWalletListener*, MutationRecord, bool>*)req->data)), std::get<2>(*((std::tuple<NJSIWalletListener*, MutationRecord, bool>*)req->data)));
        delete (std::tuple<NJSIWalletListener*, MutationRecord, bool>*)req->data;
        req->data = nullptr;
    }
    );
}

void NJSIWalletListener::notifyUpdatedTransaction_aimpl__(const TransactionRecord & transaction)
{
    const auto& env = Env();
    Napi::HandleScope scope(env);
    //Wrap parameters
    std::vector<napi_value> args;
    auto arg_0 = Napi::Object::New(env);
    auto arg_0_1 = Napi::String::New(env, transaction.txHash);
    arg_0.Set("txHash", arg_0_1);
    auto arg_0_2 = Napi::Value::From(env, transaction.timeStamp);
    arg_0.Set("timeStamp", arg_0_2);
    auto arg_0_3 = Napi::Value::From(env, transaction.amount);
    arg_0.Set("amount", arg_0_3);
    auto arg_0_4 = Napi::Value::From(env, transaction.fee);
    arg_0.Set("fee", arg_0_4);
    auto arg_0_5 = Napi::Value::From(env, (int)transaction.status);
    arg_0.Set("status", arg_0_5);
    auto arg_0_6 = Napi::Value::From(env, transaction.height);
    arg_0.Set("height", arg_0_6);
    auto arg_0_7 = Napi::Value::From(env, transaction.blockTime);
    arg_0.Set("blockTime", arg_0_7);
    auto arg_0_8 = Napi::Value::From(env, transaction.depth);
    arg_0.Set("depth", arg_0_8);
    auto arg_0_9 = Napi::Array::New(env);
    for(size_t arg_0_9_id = 0; arg_0_9_id < transaction.inputs.size(); arg_0_9_id++)
    {
        auto arg_0_9_elem = Napi::Object::New(env);
        auto arg_0_9_elem_1 = Napi::String::New(env, transaction.inputs[arg_0_9_id].address);
        arg_0_9_elem.Set("address", arg_0_9_elem_1);
        auto arg_0_9_elem_2 = Napi::String::New(env, transaction.inputs[arg_0_9_id].label);
        arg_0_9_elem.Set("label", arg_0_9_elem_2);
        auto arg_0_9_elem_3 = Napi::String::New(env, transaction.inputs[arg_0_9_id].desc);
        arg_0_9_elem.Set("desc", arg_0_9_elem_3);
        auto arg_0_9_elem_4 = Napi::Value::From(env, transaction.inputs[arg_0_9_id].isMine);
        arg_0_9_elem.Set("isMine", arg_0_9_elem_4);

        arg_0_9.Set((int)arg_0_9_id,arg_0_9_elem);
    }

    arg_0.Set("inputs", arg_0_9);
    auto arg_0_10 = Napi::Array::New(env);
    for(size_t arg_0_10_id = 0; arg_0_10_id < transaction.outputs.size(); arg_0_10_id++)
    {
        auto arg_0_10_elem = Napi::Object::New(env);
        auto arg_0_10_elem_1 = Napi::Value::From(env, transaction.outputs[arg_0_10_id].amount);
        arg_0_10_elem.Set("amount", arg_0_10_elem_1);
        auto arg_0_10_elem_2 = Napi::String::New(env, transaction.outputs[arg_0_10_id].address);
        arg_0_10_elem.Set("address", arg_0_10_elem_2);
        auto arg_0_10_elem_3 = Napi::String::New(env, transaction.outputs[arg_0_10_id].label);
        arg_0_10_elem.Set("label", arg_0_10_elem_3);
        auto arg_0_10_elem_4 = Napi::String::New(env, transaction.outputs[arg_0_10_id].desc);
        arg_0_10_elem.Set("desc", arg_0_10_elem_4);
        auto arg_0_10_elem_5 = Napi::Value::From(env, transaction.outputs[arg_0_10_id].isMine);
        arg_0_10_elem.Set("isMine", arg_0_10_elem_5);

        arg_0_10.Set((int)arg_0_10_id,arg_0_10_elem);
    }

    arg_0.Set("outputs", arg_0_10);

    args.push_back(arg_0);
    Napi::Value calling_function_as_value = Value().Get("notifyUpdatedTransaction");
    if(!calling_function_as_value.IsUndefined() && !calling_function_as_value.IsNull())
    {
        Napi::Function calling_function = calling_function_as_value.As<Napi::Function>();
        auto result_notifyUpdatedTransaction = calling_function.Call(args);
        if(result_notifyUpdatedTransaction.IsEmpty())
        {
            Napi::Error::New(env, "NJSIWalletListener::notifyUpdatedTransaction call failed").ThrowAsJavaScriptException();
            return;
        }
    }
}

void NJSIWalletListener::notifyUpdatedTransaction(const TransactionRecord & transaction)
{
    uv_work_t* request = new uv_work_t;
    request->data = new std::tuple<NJSIWalletListener*, TransactionRecord>(this, transaction);

    uv_queue_work(uv_default_loop(), request, [](uv_work_t*) -> void{}, [](uv_work_t* req, int status) -> void
    {
        NJSIWalletListener* pthis = std::get<0>(*((std::tuple<NJSIWalletListener*, TransactionRecord>*)req->data));
        pthis->notifyUpdatedTransaction_aimpl__(std::get<1>(*((std::tuple<NJSIWalletListener*, TransactionRecord>*)req->data)));
        delete (std::tuple<NJSIWalletListener*, TransactionRecord>*)req->data;
        req->data = nullptr;
    }
    );
}

Napi::FunctionReference NJSIWalletListener::constructor;

Napi::Object NJSIWalletListener::Init(Napi::Env env, Napi::Object exports) {

    Napi::Function func = DefineClass(env, "NJSIWalletListener",{});
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    exports.Set("NJSIWalletListener", func);
    return exports;
}
