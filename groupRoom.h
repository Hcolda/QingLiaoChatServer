#ifndef GROUP_ROOM_H
#define GROUP_ROOM_H

#include "room.h"

namespace qls
{
    /*
    * @brief 群聊房间
    */
    class GroupRoom : public qls::BaseRoom
    {
    public:
        struct User : public qls::BaseRoom::BaseUserSetting {};

        enum class ReturnState
        {
            UNKNOWN_STATE = 0,
            OK_STATE,
            NO_MEMBER_STATE,
            MEMBER_MUTED_STATE
        };

        class Permission
        {
        public:
            enum class PermissionType
            {
                Default = 0,
                Operator,
                Administrator
            };

            Permission() = default;
            ~Permission() = default;

            /*
            * @brief 修改权限
            * @param permissionName 权限名
            * @param type 权限类型
            */
            void modifyPermission(const std::string& permissionName, PermissionType type = PermissionType::Default);

            /*
            * @brief 删除权限
            * @param permissionName 权限名
            */
            void removePermission(const std::string& permissionName);

            /*
            * @brief 获取权限的权限类型
            * @param permissionName 权限名
            */
            PermissionType getPermissionType(const std::string& permissionName) const;

            /*
            * @brief 修改用户
            * @param user_id 用户的id
            * @param type 权限类型
            */
            void modifyUserPermission(long long user_id, PermissionType type = PermissionType::Default);

            /*
            * @brief 删除用户
            * @param user_id 用户的id
            */
            void removeUser(long long user_id);

            /*
            * @brief 删除用户
            * @param user_id 用户的id
            * @param permissionName 权限名
            */
            bool userHasPermission(long long user_id, const std::string& permissionName) const;

            /*
            * @brief 获取用户的权限类型
            * @param user_id 用户的id
            */
            PermissionType getUserPermissionType(long long user_id) const;

        private:
            std::unordered_map<std::string, PermissionType> m_permission_map;
            mutable std::shared_mutex                       m_permission_map_mutex;

            std::unordered_map<long long, PermissionType>   m_user_permission_map;
            mutable std::shared_mutex                       m_user_permission_map_mutex;
        };

        GroupRoom(long long group_id);
        ~GroupRoom() = default;

        /*
        * 初始化
        */
        void init();

        /*
        * @brief 添加用户进入群聊
        * @param user_id 用户id
        * @return true 添加成功 | false 添加失败
        */
        bool addMember(long long user_id);

        /*
        * @brief 从群聊中移除用户
        * @param user_id 用户id
        * @return true 移除成功 | false 移除失败
        */
        bool removeMember(long long user_id);

        /*
        * @brief 将用户连接加入群聊房间
        * @param socket_ptr socket指针
        * @param user 用户数据
        * @return true 加入成功 | false 加入失败
        */
        bool joinRoom(const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr, const User& user);

        /*
        * @brief 从群聊房间删除用户连接
        * @param socket_ptr socket指针
        * @return true 离开成功 | false 离开失败
        */
        bool leaveRoom(const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr);

        /*
        * @brief 广播消息
        * @param sender_user_id 发送者user_id
        * @param message 发送的消息（非二进制数据）
        * @param sender_user_id 发送者user_id
        * @return true 发送成功 | false 发送失败
        */
        asio::awaitable<bool> sendMessage(long long sender_user_id, const std::string& message);

        /*
        * @brief 发送提示消息
        * @param message 发送的消息（非二进制数据）
        * @return true 发送成功 | false 发送失败
        */
        asio::awaitable<bool> sendTipMessage(long long sender_user_id, const std::string& message);

        /*
        * @brief 发送提示消息给单独某个用户
        * @param message 发送的消息（非二进制数据）
        * @param receiver_user_id 接收者user_id
        * @return true 发送成功 | false 发送失败
        */
        asio::awaitable<bool> sendUserTipMessage(long long sender_user_id, const std::string& message, long long receiver_user_id);

        /*
        * @brief 是否有此用户
        * @param user_id 用户的id
        * @return true 有 | false 无
        */
        bool hasUser(long long user_id) const;

        /*
        * @brief 获取群聊拥有者
        * @return user_id
        */
        long long getAdministrator() const;

        /*
        * @brief 设置群聊拥有者
        * @param user_id 用户id
        */
        void setAdministrator(long long user_id);

        /*
        * @brief 获取群聊id
        * @return 群聊id
        */
        long long getGroupID() const;

    private:
        const long long                 m_group_id;
        std::atomic<long long>          m_administrator_user_id;
        Permission                      m_permission;

        std::unordered_set<long long>   m_user_id_map;
        mutable std::shared_mutex       m_user_id_map_mutex;

        std::unordered_set<long long>   m_muted_user_set;
        mutable std::shared_mutex       m_muted_user_set_mutex;

        std::priority_queue<std::pair<long long, std::string>,
            std::vector<std::pair<long long, std::string>>,
            std::greater<std::pair<long long, std::string>>>        m_message_queue;
        std::shared_mutex                                           m_message_queue_mutex;
    };
}

#endif // !GROUP_ROOM_H
