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
 * @file ParallelConfigPrecompiled.h
 * @author: kyonRay
 * @date 2021-05-28
 */

#pragma once
#include "../libvm/BlockContext.h"
#include "../libvm/Precompiled.h"
#include "Common.h"
#include <bcos-framework/interfaces/storage/TableInterface.h>
#include <bcos-framework/libcodec/abi/ContractABICodec.h>

namespace bcos
{
namespace precompiled
{
struct ParallelConfig
{
    using Ptr = std::shared_ptr<ParallelConfig>;
    std::string functionName;
    u256 criticalSize;
};
const std::string PARA_CONFIG_TABLE_PREFIX_SHORT = "cp_";

class ParallelConfigPrecompiled : public bcos::precompiled::Precompiled
{
public:
    using Ptr = std::shared_ptr<ParallelConfigPrecompiled>;
    ParallelConfigPrecompiled(crypto::Hash::Ptr _hashImpl);
    virtual ~ParallelConfigPrecompiled(){};

    std::string toString() override;

    std::shared_ptr<PrecompiledExecResult> call(std::shared_ptr<executor::BlockContext> _context,
        bytesConstRef _param, const std::string& _origin, const std::string& _sender,
        u256& _remainGas) override;

    bcos::storage::TableInterface::Ptr openTable(std::shared_ptr<executor::BlockContext> _context,
        std::string const& _contractAddress, std::string const& _origin, bool _needCreate = true);

private:
    void registerParallelFunction(std::shared_ptr<executor::BlockContext> _context,
        bytesConstRef _data, std::string const& _origin, bytes& _out);
    void unregisterParallelFunction(std::shared_ptr<executor::BlockContext> _context,
        bytesConstRef _data, std::string const& _origin, bytes& _out);
    std::string getTableName(std::string const& _contractName, bool _isWasm);

public:
    /// get parallel config, return nullptr if not found
    ParallelConfig::Ptr getParallelConfig(std::shared_ptr<executor::BlockContext> _context,
        std::string const& _contractAddress, uint32_t _selector, std::string const& _origin);
};
}  // namespace precompiled
}  // namespace bcos