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
#include "gabriel/login/server.hpp"
#include "gabriel/protocol/server/center/msg_type.pb.h"
#include "gabriel/protocol/server/center/default.pb.h"

using namespace std;

namespace gabriel {
namespace login {

Server::Server()
{
    type(gabriel::base::LOGIN_SERVER);
}

Server::~Server()
{
}

void Server::on_connection_shutdown(gabriel::base::Client_Connection *client_connection)
{
    //客户端连接掉线
    gabriel::base::Server::on_connection_shutdown(client_connection);
}

void Server::on_connection_shutdown_ordinary(gabriel::base::Server_Connection *server_connection)
{
    if(server_connection == &m_record_connection)
    {
        cout << "error: 与record服务器失去连接" << endl;
    }
}

void Server::do_main_server_connection_ordinary()
{
    m_record_connection.do_main();
}

bool Server::verify_connection(gabriel::base::Client_Connection *client_connection)
{
    return true;
}

int32 Server::init_hook_ordinary()
{
    zone_id(1);
    m_supercenter_addr.set(20000);

    return 0;
}

void Server::update_hook()
{
    //游戏循环
}

void Server::reconnect_ordinary()
{
    if(m_record_connection.lost_connection())
    {
        gabriel::base::Server_Connection *tmp = &m_record_connection;
            
        if(m_connector.connect(tmp, m_record_connection.inet_addr()) < 0)
        {
            cout << "error: 尝试重新连接到record服务器失败" << endl;
        }
        else
        {
            cout << "尝试重新连接到record服务器成功" << endl;
        }
    }
}
    
void Server::register_msg_handler_ordinary()
{
    using namespace gabriel::protocol::server::center;
    m_center_msg_handler.register_handler(DEFAULT_MSG_TYPE, REGISTER_SERVER, this, &Server::register_rsp);
}
    
void Server::init_reactor()
{
    delete ACE_Reactor::instance(new ACE_Reactor(new ACE_Dev_Poll_Reactor(5000, true), true), true);
    m_center_connection.reactor(ACE_Reactor::instance());
    m_record_connection.reactor(ACE_Reactor::instance());
}

void Server::handle_connection_msg(gabriel::base::Client_Connection *client_connection, uint32 msg_type, uint32 msg_id, void *data, uint32 size)
{
    m_client_msg_handler.handle_message(msg_type, msg_id, client_connection, data, size);
}

void Server::register_rsp(gabriel::base::Server_Connection *server_connection, void *data, uint32 size)
{
    using namespace gabriel::protocol::server::center;
    PARSE_MSG(Register_Rsp, msg);

    if(id() > 0)
    {
        return;
    }

    if(msg.info_size() != 2)
    {
        state(gabriel::base::SERVER_STATE::SHUTDOWN);
        cout << "error: 从center服务器接收到的本服务器信息有误" << endl;
        
        return;
    }

    for(uint32 i = 0; i != msg.info_size(); ++i)
    {
        const auto &info = msg.info(i);

        if(info.server_type() == gabriel::base::LOGIN_SERVER)
        {
            id(info.server_id());
            
            if(m_acceptor.open(ACE_INET_Addr(info.port(), info.outer_addr().c_str()), ACE_Reactor::instance()) < 0)
            {
                state(gabriel::base::SERVER_STATE::SHUTDOWN);
                cout << "error: 启动login服务器失败" << endl;
            
                return;
            }

            cout << "启动login服务器成功" << endl;    
        }
        else
        {
            gabriel::base::Server_Connection *tmp = &m_record_connection;

            if(m_connector.connect(tmp, ACE_INET_Addr(info.port(), info.inner_addr().c_str())) < 0)
            {
                cout << "error: 连接到record服务器失败" << endl;
                state(gabriel::base::SERVER_STATE::SHUTDOWN);
            }
            else
            {
                cout << "连接到record服务器成功" << endl;
            }
        }
    }
}

void Server::handle_connection_msg_ordinary(gabriel::base::Server_Connection *server_connection, uint32 msg_type, uint32 msg_id, void *data, uint32 size)
{
    if(server_connection == &m_center_connection)
    {
        m_center_msg_handler.handle_message(msg_type, msg_id, server_connection, data, size);
    }
    else if(server_connection == &m_record_connection)
    {
        m_record_msg_handler.handle_message(msg_type, msg_id, server_connection, data, size);
    }
}

void Server::fini_hook()
{
    //停服操作 比如释放资源
}

}
}

#include "gabriel/main.cpp"
