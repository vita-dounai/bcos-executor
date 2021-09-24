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
 * @file ConsensusPrecompiled.h
 * @author: kyonRay
 * @date 2021-05-26
 */

#pragma once
#include "../executive/BlockContext.h"
#include "../vm/Precompiled.h"
#include "Common.h"
#include <bcos-framework/interfaces/storage/Common.h>
#include <bcos-framework/interfaces/storage/Table.h>
namespace bcos
{
namespace precompiled
{
class ConsensusPrecompiled : public bcos::precompiled::Precompiled
{
#if 0
contract ConsensusSystemTable
{
    function addSealer(string nodeID) public returns(int256);
    function addObserver(string nodeID) public returns(int256);
    function remove(string nodeID) public returns(int256);
}
#endif
public:
    using Ptr = std::shared_ptr<ConsensusPrecompiled>;
    ConsensusPrecompiled(crypto::Hash::Ptr _hashImpl);
    virtual ~ConsensusPrecompiled(){};

    std::shared_ptr<PrecompiledExecResult> call(std::shared_ptr<executor::BlockContext> _context,
        bytesConstRef _param, const std::string& _origin, const std::string& _sender,
        int64_t _remainGas) override;

private:
    int addSealer(const std::shared_ptr<executor::BlockContext>& _context, bytesConstRef& _data,
        const std::string& _origin);

    int addObserver(const std::shared_ptr<executor::BlockContext>& _context, bytesConstRef& _data,
        const std::string& _origin);

    int removeNode(const std::shared_ptr<executor::BlockContext>& _context, bytesConstRef& _data,
        const std::string& _origin);

    int setWeight(const std::shared_ptr<executor::BlockContext>& _context, bytesConstRef& _data,
        const std::string& _origin);


private:
    void showConsensusTable(std::shared_ptr<executor::BlockContext> context);
    bool checkIsLastSealer(std::optional<bcos::storage::Table> table, std::string const& nodeID);
    std::shared_ptr<std::map<std::string, std::optional<storage::Entry>>> getRowsByNodeType(
        std::optional<bcos::storage::Table> _table, std::string const& _nodeType);
};
}  // namespace precompiled
}  // namespace bcos
