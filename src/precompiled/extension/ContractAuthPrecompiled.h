/**
 *  Copyright (C) 2021 FISCO BCOS.
 *  SPDX-License-Identifier: Apache-2.0
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 * @file ContractAuthPrecompiled.h
 * @author: kyonRay
 * @date 2021-10-09
 */

#pragma once
#include "../../vm/Precompiled.h"
#include "../Common.h"

#if 0
function agent(string path) public returns(string);
function setAgent(string path,string agent) public returns(int256);
function setAuth(string path,bytes interface,string user,bool) public constant returns(int256);
function checkAuth(string path,bytes interface,string user) public constant returns(bool);
#endif

namespace bcos::precompiled
{
const char AGENT_FILED[] = "agent";
const char INTERFACE_AUTH[] = "interface_auth";
class ContractAuthPrecompiled : public bcos::precompiled::Precompiled
{
public:
    using Ptr = std::shared_ptr<ContractAuthPrecompiled>;
    ContractAuthPrecompiled(crypto::Hash::Ptr _hashImpl);
    virtual ~ContractAuthPrecompiled(){};

    std::shared_ptr<PrecompiledExecResult> call(
        std::shared_ptr<executor::TransactionExecutive> _executive, bytesConstRef _param,
        const std::string& _origin, const std::string& _sender) override;

private:
    void agent(const std::shared_ptr<executor::TransactionExecutive>& _executive,
        bytesConstRef& data, const std::shared_ptr<PrecompiledExecResult>& callResult,
        const PrecompiledGas::Ptr& gasPricer);

    void setAgent(const std::shared_ptr<executor::TransactionExecutive>& _executive,
        bytesConstRef& data, const std::shared_ptr<PrecompiledExecResult>& callResult,
        const std::string& _origin, const std::string& _sender,
        const PrecompiledGas::Ptr& gasPricer);

    void setAuth(const std::shared_ptr<executor::TransactionExecutive>& _executive,
        bytesConstRef& data, const std::shared_ptr<PrecompiledExecResult>& callResult,
        const std::string& _origin, const std::string& _sender,
        const PrecompiledGas::Ptr& gasPricer);

    void checkAuth(const std::shared_ptr<executor::TransactionExecutive>& _executive,
        bytesConstRef& data, const std::shared_ptr<PrecompiledExecResult>& callResult,
        const PrecompiledGas::Ptr& gasPricer);

    std::string authMap2Json(
        const std::map<bytes, std::vector<std::pair<std::string, bool>>>& _authMap);
    void json2AuthMap(const std::string& _json,
        std::map<bytes, std::vector<std::pair<std::string, bool>>>& _authMap);

    inline bool checkSender(std::string_view _sender) { return (_sender.substr(0, 4) != "/sys/"); }
};
}  // namespace bcos::precompiled