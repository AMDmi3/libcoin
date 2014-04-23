/* -*-c++-*- libcoin - Copyright (C) 2012 Michael Gronager
 *
 * libcoin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * libcoin is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libcoin.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef COINPOOL_POOL_H
#define COINPOOL_POOL_H

#include <coinPool/Export.h>

#include <coin/Block.h>

#include <coinChain/Node.h>

#include <coinHTTP/Method.h>

#include <map>

/// Payee is an interface to a generalized payee, the receiver of money. The payee can choose the generate a new script for each payment, to reuse a single script over and over again, or even to e.g. to use a deterministic wallet to generate a group of public keys.
class Payee {
public:
    virtual Script current_script() = 0;
    virtual void mark_used(const Script&) {}
};

class StaticPayee : public Payee {
public:
    StaticPayee(const PubKeyHash& pkh) : _pkh(pkh) { }
    virtual Script current_script();
private:
    PubKeyHash _pkh;
};

/// Pool is an interface for running a pool. It is a basis for the pool rpc commands: 'getwork', 'getblocktemplate' and 'submitblock'
/// The Pool requests a template from the blockchain - i.e. composes a block candidate from the unconfirmed transactions. It stores the
/// candidate mapped by its merkletree root.
/// A solved block submitted either using submitblock or getwork looks up the block and replaces the nonce/time/coinbase
class COINPOOL_EXPORT Pool {
public:
    /// Initialize Pool with a node and a payee, i.e. an abstraction of a payee address generator
    Pool(Node& node, Payee& payee) : _node(node), _blockChain(node.blockChain()), _payee(payee) {
    }
    
    // map of merkletree hash to block templates
    typedef std::map<uint256, Block> BlockTemplates;
    
    /// subclass this to implement other schemes for coinbase payout
    virtual Block getBlockTemplate(bool invalid = false);
    
    virtual std::string submitBlock(const Block& stub, std::string workid = "");
    
    typedef std::pair<Block, uint256> Work;
    
    virtual Work getWork(bool invalid = false);
    
    uint256 target() const;
    
    boost::asio::io_service& get_io_service() { return _node.get_io_service(); }
    
    const BlockChain& blockChain() { return _blockChain; }
    
    Node& node() { return _node; }
protected:
    Node& _node;
    const BlockChain& _blockChain;
    Payee& _payee;
    BlockTemplates _templates;
    BlockTemplates::iterator _latest_work;
};

class COINPOOL_EXPORT SharePool : public Pool {
public:
    /// Initialize Pool with a node and a payee, i.e. an abstraction of a payee address generator
    SharePool(Node& node, Payee& payee) : Pool(node, payee) {
    }
    
    /// generate a block template that adheres to the current share chain, i.e. pays out dividend to the other share holders
    virtual Block getBlockTemplate(bool invalid = false) {
        // iterate over all dividend and add
        BlockChain::Payees payees;
        payees.push_back(_payee.current_script()); // insert our script as the first
        BlockChain::Fractions fractions;
        fractions.push_back(1);
        ShareTree::Dividend divi = _blockChain.getDividend();
        for (ShareTree::Dividend::const_iterator d = divi.begin(); d != divi.end(); ++d) {
            payees.push_back(d->first);
            fractions.push_back(d->second);
        }
        BlockIterator blk = _blockChain.getBest();
        Block block = _blockChain.getBlockTemplate(blk, payees, fractions);
        // add the block to the list of work
        return block;
    }
    
};

class COINPOOL_EXPORT PoolMethod : public Method {
public:
    PoolMethod(Pool& pool) : _pool(pool) {}
    
    virtual void dispatch(const CompletionHandler& f) {
        _pool.get_io_service().dispatch(f);
    }
    
    virtual void dispatch(const CompletionHandler& f, const Request& request) {
        dispatch(f);
    }
    
    void formatHashBuffers(Block& block, char* pmidstate, char* pdata, char* phash1);
    int formatHashBlocks(void* pbuffer, unsigned int len);
    void sha256Transform(void* pstate, void* pinput, const void* pinit);
    inline uint32_t byteReverse(uint32_t value) {
        value = ((value & 0xFF00FF00) >> 8) | ((value & 0x00FF00FF) << 8);
        return (value<<16) | (value>>16);
    }
    
protected:
    Pool& _pool;
};

#endif // COINPOOL_POOL_H
