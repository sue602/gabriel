/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *      _____       ___   _____   _____    _   _____   _               *
 *     /  ___|     /   | |  _  \ |  _  \  | | | ____| | |              *
 *     | |        / /| | | |_| | | |_| |  | | | |__   | |              *
 *     | |  _    / / | | |  _  { |  _  /  | | |  __|  | |              *
 *     | |_| |  / /  | | | |_| | | | \ \  | | | |___  | |___           *
 *     \_____/ /_/   |_| |_____/ |_|  \_\ |_| |_____| |_____|          *
 *                                                                     *
 *     gabriel is an angel from the Holy Bible, this engine is named   *
 *   gabriel, means bringing people good news. the goal of gabriel     *
 *   server engine is to help people to develop various online games,  *
 *   welcome you to join in.                                           *
 *                                                                     *
 *   @author: lichuan                                                  *
 *   @qq: 308831759                                                    *
 *   @email: 308831759@qq.com                                          *
 *   @site: www.lichuan.me                                             *
 *   @github: https://github.com/lichuan/gabriel                       *
 *   @date: 2014-01-09 12:48:05                                        *
 *                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <iostream>
#include "ace/Dev_Poll_Reactor.h"
#include "gabriel/supercenter/server.hpp"
#include "gabriel/protocol/server/supercenter/msg_type.pb.h"
#include "gabriel/protocol/server/supercenter/default.pb.h"

using namespace std;

namespace gabriel {
namespace supercenter {

Server::Server()
{
    type(gabriel::base::SUPERCENTER_SERVER);
}

Server::~Server()
{
}

bool Server::verify_connection(gabriel::base::Client_Connection *client_connection)
{
    return true;
}

void Server::on_connection_shutdown(gabriel::base::Client_Connection *client_connection)
{
    //客户端连接掉线
    gabriel::base::Server::on_connection_shutdown(client_connection);
    
    for(auto iter : m_zone_connections)
    {
        if(iter.second == client_connection)
        {
            m_zone_connections.erase(iter.first);
            cout << iter.first << "区center服务器与本服务器断开连接" << endl;
            
            break;
        }
    }
}

void Server::init_reactor()
{
    delete ACE_Reactor::instance(new ACE_Reactor(new ACE_Dev_Poll_Reactor(1000, true), true), true);
}

void Server::test_timer(std::string arg_str)
{
    cout << "timer arg_str: " << arg_str << endl;
}
    
int32 Server::init_hook()
{
    if(m_acceptor.open(ACE_INET_Addr(20000), ACE_Reactor::instance()) < 0)
    {
        cout << "error: 启动supercenter服务器失败" << endl;

        return -1;
    }
    
    cout << "启动supercenter服务器成功" << endl;

    //test timer
    schedule_timer(std::bind(&Server::test_timer, this, "yours timer"), 1000);    
    
    const uint32 zone_id = 1;    
    //先手写服务器的相关配置数据，以后改成配置或数据库读取方式。
    {
        gabriel::protocol::server::Server_Info *info = new gabriel::protocol::server::Server_Info;
        info->set_server_id(2);
        info->set_server_type(gabriel::base::CENTER_SERVER);
        info->set_outer_addr("127.0.0.1");
        info->set_inner_addr("127.0.0.1");        
        info->set_port(20002);
        m_server_infos[zone_id].push_back(info);
    }
    {        
        gabriel::protocol::server::Server_Info *info = new gabriel::protocol::server::Server_Info;
        info->set_server_id(3);
        info->set_server_type(gabriel::base::RECORD_SERVER);
        info->set_outer_addr("127.0.0.1");
        info->set_inner_addr("127.0.0.1");
        info->set_port(20003);
        m_server_infos[zone_id].push_back(info);
    }
    {   
        gabriel::protocol::server::Server_Info *info = new gabriel::protocol::server::Server_Info;     
        info->set_server_id(4);
        info->set_server_type(gabriel::base::LOGIN_SERVER);
        info->set_outer_addr("127.0.0.1");
        info->set_inner_addr("127.0.0.1");
        info->set_port(20004);
        m_server_infos[zone_id].push_back(info);
    }
    {        
        gabriel::protocol::server::Server_Info *info = new gabriel::protocol::server::Server_Info;
        info->set_server_id(100);
        info->set_server_type(gabriel::base::GAME_SERVER);
        info->set_outer_addr("127.0.0.1");
        info->set_inner_addr("127.0.0.1");
        info->set_port(20100);
        m_server_infos[zone_id].push_back(info);
    }
    {        
        gabriel::protocol::server::Server_Info *info = new gabriel::protocol::server::Server_Info;
        info->set_server_id(101);
        info->set_server_type(gabriel::base::GAME_SERVER);
        info->set_outer_addr("127.0.0.1");
        info->set_inner_addr("127.0.0.1");
        info->set_port(20101);
        m_server_infos[zone_id].push_back(info);
    }
    {        
        gabriel::protocol::server::Server_Info *info = new gabriel::protocol::server::Server_Info;
        info->set_server_id(200);
        info->set_server_type(gabriel::base::GATEWAY_SERVER);
        info->set_outer_addr("192.168.1.122");
        info->set_inner_addr("127.0.0.1");
        info->set_port(20200);
        m_server_infos[zone_id].push_back(info);
    }
    {        
        gabriel::protocol::server::Server_Info *info = new gabriel::protocol::server::Server_Info;
        info->set_server_id(201);
        info->set_server_type(gabriel::base::GATEWAY_SERVER);
        info->set_outer_addr("127.0.0.1");
        info->set_inner_addr("127.0.0.1");
        info->set_port(20201);
        m_server_infos[zone_id].push_back(info);
    }
    
    return 0;
}

void Server::register_msg_handler()
{
    using namespace gabriel::protocol::server::supercenter;
    m_client_msg_handler.register_handler(DEFAULT_MSG_TYPE, REGISTER_SERVER, this, &Server::register_req);
    m_client_msg_handler.register_handler(DEFAULT_MSG_TYPE, CENTER_ADDR_REQ, this, &Server::center_addr_req);
}

void Server::center_addr_req(gabriel::base::Client_Connection *client_connection, void *data, uint32 size)
{
    using namespace gabriel::protocol::server::supercenter;
    PARSE_MSG(Center_Addr_Req, msg);
    const uint32 zone_id = msg.zone_id();
    auto iter = m_server_infos.find(zone_id);

    if(iter == m_server_infos.end())
    {
        return;
    }

    auto &infos = iter->second;    
    Center_Addr msg_rsp;

    for(auto info : infos)
    {
        if(info->server_type() == gabriel::base::CENTER_SERVER)
        {
            msg_rsp.mutable_info()->CopyFrom(*info);

            break;            
        }
    }

    client_connection->send(DEFAULT_MSG_TYPE, CENTER_ADDR_REQ, msg_rsp);
}

void Server::update_hook()
{
}
    
void Server::fini_hook()
{
    for(auto iter : m_server_infos)
    {
        for(auto &info : iter.second)
        {
            delete info;
        }
    }

    m_server_infos.clear();
}

void Server::register_req(gabriel::base::Client_Connection *client_connection, void *data, uint32 size)
{
    using namespace gabriel::protocol::server::supercenter;
    PARSE_MSG(Register, msg);
    const uint32 zone_id = msg.zone_id();
    auto iter = m_server_infos.find(zone_id);

    if(iter == m_server_infos.end())
    {
        return;
    }
    
    auto &infos = iter->second;
    Register_Rsp msg_rsp;
    
    for(auto info : infos)
    {
        msg_rsp.add_info()->CopyFrom(*info);
    }

    client_connection->send(DEFAULT_MSG_TYPE, REGISTER_SERVER, msg_rsp);
    m_zone_connections[zone_id] = client_connection;
    cout << zone_id << "区center服务器注册到本服务器" << endl;
}
    
void Server::handle_connection_msg(gabriel::base::Client_Connection *client_connection, uint32 msg_type, uint32 msg_id, void *data, uint32 size)
{
    m_client_msg_handler.handle_message(msg_type, msg_id, client_connection, data, size);
}

}
}

#include "gabriel/main.cpp"
