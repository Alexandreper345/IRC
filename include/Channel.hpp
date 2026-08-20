#include <string>
#include <set>

class Channel
{
private:
    std::string _name;
    std::string _topic;
    std::string _key;
    std::set<std::string> _members;
    std::set<std::string> _operators;
    std::set<std::string> _invited;
    bool _invitedOnly;
    bool _topicRestricted;
    size_t _userlimit;
public:
    Channel();
    Channel(const std::string& name);
    void addMember(int fd);
    void removeMember(int fd);
    void addOperator(int fd);
    bool isOperator(int fd) const;
    void invite(int fd);
    bool isInvited(int fd) const;
    bool hasMember(int fd) const;
    std::set<int> getMembers() const;
    ~Channel();
};

Channel::Channel()
{
}

Channel::~Channel()
{
}
