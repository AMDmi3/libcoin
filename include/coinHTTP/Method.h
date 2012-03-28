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

#ifndef METHOD_H
#define METHOD_H

#include <coinHTTP/Export.h>

#include <string>
#include <map>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>

#include "json/json_spirit.h"

/// The base class for Method functors - The definitions follow the JSON_RPC 2.0 definition and strives at introspection support.
class COINHTTP_EXPORT Method
{
public:    
    /// The actual function call - you need to overload this to implement a method
    virtual json_spirit::Value operator()(const json_spirit::Array& params, bool fHelp) = 0;
    
    typedef boost::function<void (json_spirit::Value)> MethodDone;

    /// Non blocking version - pr. default it will be blocking, though
    virtual void operator()(const json_spirit::Array& params, bool fHelp, MethodDone onDone) {
        onDone(operator()(params, fHelp));
    }
    
    /// Get the name of the method. Default implemented by lowercase typeid name. - REQUIRED
    virtual const std::string name() const { 
        if (_name.empty())
            return extractName();
        else
            return _name;
    }
    
    virtual const std::string summary() const { return ""; } // OPTIONAL
    virtual const std::string help() const { return "https://en.bitcoin.it/wiki/Original_Bitcoin_client/API_Calls_list"; } // OPTIONAL
    virtual const std::string params() const  { return ""; } // OPTIONAL
    virtual const std::string ret() const { return ""; } // OPTIONAL
    
    /// setName is to be able easily to overwrite the name of a Method.
    /// Nice for registering several of the same RPC calls in the same Server.
    virtual void setName(std::string name) { _name = name; }
    
    virtual const std::string extractName() const;

private:
    std::string _name;
};

typedef boost::shared_ptr<Method> method_ptr;
typedef std::map<std::string, method_ptr> Methods;

class Server;
class COINHTTP_EXPORT Stop : public Method {
public:
    Stop(Server& server) : _server(server) {}
    json_spirit::Value operator()(const json_spirit::Array& params, bool fHelp);
protected:
    Server& _server;
};

#endif // METHOD_H
