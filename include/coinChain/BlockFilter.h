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

#ifndef BLOCKFILTER_H
#define BLOCKFILTER_H

#include <coinChain/Export.h>
#include <coinChain/Filter.h>

#include <coin/uint256.h>
#include <coinChain/PeerManager.h>

#include <string>
#include <map>

class Block;
class BlockChain;
class Inventory;

class COINCHAIN_EXPORT BlockFilter : public Filter {
public:
    BlockFilter(Notifier& notifier, BlockChain& bc, bool support_normalized = false) : Filter(notifier), _blockChain(bc), _support_normalized(support_normalized) {}
    
    class Listener : private boost::noncopyable {
    public:
        virtual void operator()(const Block&) = 0;
        virtual ~Listener() {}
    };
    typedef boost::shared_ptr<Listener> listener_ptr;
    typedef std::set<listener_ptr> Listeners;
    
    void subscribe(listener_ptr listener) { _listeners.insert(listener); }
    
    virtual bool operator()(Peer* origin, Message& msg);
    
    virtual std::set<std::string> commands() {
        std::set<std::string> c;
        c.insert("block");
        c.insert("getblocks");
        c.insert("getheaders");
        c.insert("inv");
        c.insert("getdata");
        c.insert("version");
        return c;
    }
    
    /// Call process to get a hook into the block processing, e.g. if for injecting mining generated blocks
    void process(const Block& block, Peers peers);
    
private:
    uint256 getOrphanRoot(const Block& pblock);
    
    bool alreadyHave(const Inventory& inv);
    
private:
    BlockChain& _blockChain;
    Listeners _listeners;
    
    typedef std::map<uint256, Block> Orphans;
    Orphans _orphans;
    typedef std::multimap<uint256, Orphans::iterator> OrphansByPrev;
    OrphansByPrev _orphansByPrev;
    
    bool _support_normalized;
};

class COINCHAIN_EXPORT ShareFilter : public Filter {
public:
    ShareFilter(Notifier& notifier, BlockChain& bc) : Filter(notifier), _blockChain(bc) {}
    
    class Listener : private boost::noncopyable {
    public:
        virtual void operator()(const Block&) = 0;
        virtual ~Listener() {}
    };
    typedef boost::shared_ptr<Listener> listener_ptr;
    typedef std::set<listener_ptr> Listeners;
    
    void subscribe(listener_ptr listener) { _listeners.insert(listener); }
    
    virtual bool operator()(Peer* origin, Message& msg) { return false; }
    
    virtual std::set<std::string> commands() {
        std::set<std::string> c;
        c.insert("share");
        c.insert("version");
        return c;
    }
    
    /// Call process to get a hook into the share processing, e.g. if for injecting mining generated blocks
    bool process(const Block& block, Peers peers);
    
private:
    uint256 getOrphanRoot(const Block* block);
    
    bool alreadyHave(const Inventory& inv);
    
private:
    BlockChain& _blockChain;
    Listeners _listeners;
    
    std::map<uint256, Block*> _orphanShares;
    std::multimap<uint256, Block*> _orphanSharesByPrev;
};

#endif // BLOCKFILTER_H
