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
 * @file KVTableFactoryPrecompiled.cpp
 * @author: kyonRay
 * @date 2021-05-27
 */

#include "KVTableFactoryPrecompiled.h"
#include "Common.h"
#include "KVTablePrecompiled.h"
#include "PrecompiledResult.h"
#include "Utilities.h"
#include <bcos-framework/interfaces/protocol/Exceptions.h>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/throw_exception.hpp>

using namespace bcos;
using namespace bcos::executor;
using namespace bcos::storage;
using namespace bcos::precompiled;
using namespace bcos::protocol;

const char* const KV_TABLE_FACTORY_METHOD_OPEN_TABLE = "openTable(string)";
const char* const KV_TABLE_FACTORY_METHOD_CREATE_TABLE = "createTable(string,string,string)";

KVTableFactoryPrecompiled::KVTableFactoryPrecompiled(crypto::Hash::Ptr _hashImpl)
  : Precompiled(_hashImpl)
{
    name2Selector[KV_TABLE_FACTORY_METHOD_OPEN_TABLE] =
        getFuncSelector(KV_TABLE_FACTORY_METHOD_OPEN_TABLE, _hashImpl);
    name2Selector[KV_TABLE_FACTORY_METHOD_CREATE_TABLE] =
        getFuncSelector(KV_TABLE_FACTORY_METHOD_CREATE_TABLE, _hashImpl);
}

std::string KVTableFactoryPrecompiled::toString()
{
    return "KVTableFactory";
}

PrecompiledExecResult::Ptr KVTableFactoryPrecompiled::call(
    std::shared_ptr<executor::BlockContext> _context, bytesConstRef _param,
    const std::string& _origin, const std::string& _sender, u256& _remainGas)
{
    uint32_t func = getParamFunc(_param);
    bytesConstRef data = getParamData(_param);
    PRECOMPILED_LOG(DEBUG) << LOG_BADGE("KVTableFactory") << LOG_DESC("call")
                           << LOG_KV("func", func);

    auto callResult = std::make_shared<PrecompiledExecResult>();
    auto gasPricer = m_precompiledGasFactory->createPrecompiledGas();
    gasPricer->setMemUsed(_param.size());

    if (func == name2Selector[KV_TABLE_FACTORY_METHOD_OPEN_TABLE])
    {
        // openTable(string)
        openTable(_context, data, callResult, gasPricer);
    }
    else if (func == name2Selector[KV_TABLE_FACTORY_METHOD_CREATE_TABLE])
    {
        // createTable(string,string,string)
        createTable(_context, data, callResult, _origin, _sender, gasPricer);
    }
    else
    {
        PRECOMPILED_LOG(ERROR) << LOG_BADGE("KVTableFactoryPrecompiled")
                               << LOG_DESC("call undefined function!");
    }
    gasPricer->updateMemUsed(callResult->m_execResult.size());
    _remainGas -= gasPricer->calTotalGas();
    return callResult;
}

crypto::HashType KVTableFactoryPrecompiled::hash()
{
    return m_memoryTableFactory->hash();
}

void KVTableFactoryPrecompiled::checkCreateTableParam(
    const std::string& _tableName, std::string& _keyField, std::string& _valueField)
{
    std::vector<std::string> fieldNameList;
    boost::split(fieldNameList, _valueField, boost::is_any_of(","));
    boost::trim(_keyField);
    if (_keyField.size() > (size_t)SYS_TABLE_KEY_FIELD_NAME_MAX_LENGTH)
    {  // mysql TableName and fieldName length limit is 64
        BOOST_THROW_EXCEPTION(PrecompiledError() << errinfo_comment(
                                  std::string("table field name length overflow ") +
                                  std::to_string(SYS_TABLE_KEY_FIELD_NAME_MAX_LENGTH)));
    }
    for (auto& str : fieldNameList)
    {
        boost::trim(str);
        if (str.size() > (size_t)SYS_TABLE_KEY_FIELD_NAME_MAX_LENGTH)
        {  // mysql TableName and fieldName length limit is 64
            BOOST_THROW_EXCEPTION(PrecompiledError() << errinfo_comment(
                                      std::string("table field name length overflow ") +
                                      std::to_string(SYS_TABLE_KEY_FIELD_NAME_MAX_LENGTH)));
        }
    }

    std::vector<std::string> keyFieldList{_keyField};
    checkNameValidate(_tableName, keyFieldList, fieldNameList);

    _valueField = boost::join(fieldNameList, ",");
    if (_valueField.size() > (size_t)SYS_TABLE_VALUE_FIELD_MAX_LENGTH)
    {
        BOOST_THROW_EXCEPTION(PrecompiledError() << errinfo_comment(
                                  std::string("total table field name length overflow ") +
                                  std::to_string(SYS_TABLE_VALUE_FIELD_MAX_LENGTH)));
    }

    auto tableName = precompiled::getTableName(_tableName, true);
    if (tableName.size() > (size_t)USER_TABLE_NAME_MAX_LENGTH_S)
    {  // mysql TableName and fieldName length limit is 64
        BOOST_THROW_EXCEPTION(
            PrecompiledError() << errinfo_comment(std::string("tableName length overflow ") +
                                                  std::to_string(USER_TABLE_NAME_MAX_LENGTH)));
    }
    PRECOMPILED_LOG(INFO) << LOG_BADGE("KVTableFactory") << LOG_KV("createTable", _tableName)
                          << LOG_KV("keyField", _keyField) << LOG_KV("valueFiled", _valueField);
}

void KVTableFactoryPrecompiled::openTable(const std::shared_ptr<executor::BlockContext>& _context,
    bytesConstRef& data, const std::shared_ptr<PrecompiledExecResult>& callResult,
    const PrecompiledGas::Ptr& gasPricer)
{
    // openTable(string)
    std::string tableName;
    auto codec = std::make_shared<PrecompiledCodec>(_context->hashHandler(), _context->isWasm());
    codec->decode(data, tableName);
    PRECOMPILED_LOG(DEBUG) << LOG_BADGE("KVTableFactory") << LOG_KV("openTable", tableName);
    tableName = getTableName(tableName, _context->isWasm());
    auto table = m_memoryTableFactory->openTable(tableName);
    gasPricer->appendOperation(InterfaceOpcode::OpenTable);
    if (!table)
    {
        PRECOMPILED_LOG(WARNING) << LOG_BADGE("KVTableFactoryPrecompiled")
                                 << LOG_DESC("Open new table failed")
                                 << LOG_KV("table name", tableName);
        BOOST_THROW_EXCEPTION(PrecompiledError() << errinfo_comment(tableName + " does not exist"));
    }
    auto kvTablePrecompiled = std::make_shared<KVTablePrecompiled>(m_hashImpl);
    kvTablePrecompiled->setTable(table);
    if (_context->isWasm())
    {
        auto address = _context->registerPrecompiled(kvTablePrecompiled);
        callResult->setExecResult(codec->encode(address));
    }
    else
    {
        auto address =
            Address(_context->registerPrecompiled(kvTablePrecompiled), FixedBytes<20>::FromBinary);
        callResult->setExecResult(codec->encode(address));
    }
}

void KVTableFactoryPrecompiled::createTable(const std::shared_ptr<executor::BlockContext>& _context,
    bytesConstRef& data, const std::shared_ptr<PrecompiledExecResult>& callResult,
    const std::string& _origin, const std::string& _sender, const PrecompiledGas::Ptr& gasPricer)
{
    // createTable(string,string,string)
    if (!checkAuthority(_context->getTableFactory(), _origin, _sender))
    {
        PRECOMPILED_LOG(ERROR) << LOG_BADGE("KVTableFactoryPrecompiled")
                               << LOG_DESC("permission denied") << LOG_KV("origin", _origin)
                               << LOG_KV("contract", _sender);
        BOOST_THROW_EXCEPTION(
            PrecompiledError() << errinfo_comment(
                "Permission denied. " + _origin + " can't call contract " + _sender));
    }
    std::string tableName;
    std::string keyField;
    std::string valueFiled;
    auto codec = std::make_shared<PrecompiledCodec>(_context->hashHandler(), _context->isWasm());
    codec->decode(data, tableName, keyField, valueFiled);

    if (_context->isWasm() && (tableName.empty() || tableName.at(0) != '/'))
    {
        PRECOMPILED_LOG(ERROR) << LOG_BADGE("KVTableFactoryPrecompiled")
                               << LOG_DESC("create table in wasm env: error tableName")
                               << LOG_KV("tableName", tableName);
        BOOST_THROW_EXCEPTION(PrecompiledError() << errinfo_comment("Error table name."));
    }

    checkCreateTableParam(tableName, keyField, valueFiled);

    // wasm: /data + tableName, evm: u_ + tableName
    auto newTableName = getTableName(tableName, _context->isWasm());
    int result = 0;
    auto table = m_memoryTableFactory->openTable(newTableName);
    gasPricer->appendOperation(InterfaceOpcode::OpenTable);
    if (table)
    {
        // table already exist
        result = CODE_TABLE_NAME_ALREADY_EXIST;
    }
    else
    {
        m_memoryTableFactory->createTable(newTableName, keyField, valueFiled);
        gasPricer->appendOperation(InterfaceOpcode::CreateTable);
        if (_context->isWasm())
        {
            auto inodeTableName = USER_TABLE_PREFIX_WASM + tableName;
            // create inode table
            m_memoryTableFactory->createTable(inodeTableName, SYS_KEY, SYS_VALUE);

            // set inode data of table in file system
            auto inodeTable = m_memoryTableFactory->openTable(inodeTableName);
            auto typeEntry = inodeTable->newEntry();
            typeEntry->setField(SYS_VALUE, FS_TYPE_DATA);
            inodeTable->setRow(FS_KEY_TYPE, typeEntry);
            auto addressEntry = inodeTable->newEntry();
            addressEntry->setField(SYS_VALUE, newTableName);
            inodeTable->setRow(FS_TABLE_KEY_ADDRESS, addressEntry);
            auto numEntry = inodeTable->newEntry();
            numEntry->setField(SYS_VALUE, std::to_string(_context->currentNumber()));
            inodeTable->setRow(FS_TABLE_KEY_NUM, numEntry);

            auto parentPath = inodeTableName.substr(0, inodeTableName.find_last_of('/'));
            auto tableRelativePath = tableName.substr(tableName.find_last_of('/') + 1);
            recursiveBuildDir(m_memoryTableFactory, parentPath);

            // parentPath table must exist
            // update parentPath subdirectories
            auto parentTable = m_memoryTableFactory->openTable(parentPath);
            auto entry = parentTable->getRow(FS_KEY_SUB);
            DirInfo parentDir;
            DirInfo::fromString(parentDir, entry->getField(SYS_VALUE));
            FileInfo fileInfo(tableRelativePath, FS_TYPE_DATA, _context->currentNumber());
            parentDir.getMutableSubDir().emplace_back(fileInfo);

            auto newEntry = parentTable->newEntry();
            newEntry->setField(SYS_VALUE, parentDir.toString());
            parentTable->setRow(FS_KEY_SUB, newEntry);
        }
    }
    getErrorCodeOut(callResult->mutableExecResult(), result, codec);
}