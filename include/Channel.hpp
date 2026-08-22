#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <string>
# include <set>
# include <cstddef>

class Channel
{
private:
	std::string		_name;
	std::string		_topic;
	std::string		_key;
	std::set<int>	_members;
	std::set<int>	_operators;
	std::set<int>	_invited;
	bool			_inviteOnly;
	bool			_topicRestricted;
	size_t			_userLimit;

public:
	Channel(void);
	Channel(const std::string& name);
	Channel(const Channel& other);
	Channel	&operator=(const Channel& other);
	~Channel(void);

	const std::string&		getName(void) const;
	const std::string&		getTopic(void) const;
	void					setTopic(const std::string& topic);
	const std::string&		getKey(void) const;
	void					setKey(const std::string& key);
	void					unsetKey(void);
	bool					hasKey(void) const;

	void					addMember(int fd);
	void					removeMember(int fd);
	bool					hasMember(int fd) const;
	const std::set<int>&	getMembers(void) const;
	bool					isEmpty(void) const;
	size_t					memberCount(void) const;

	void					addOperator(int fd);
	void					removeOperator(int fd);
	bool					isOperator(int fd) const;

	void					invite(int fd);
	void					removeInvite(int fd);
	bool					isInvited(int fd) const;

	bool					isInviteOnly(void) const;
	void					setInviteOnly(bool enable);
	bool					isTopicRestricted(void) const;
	void					setTopicRestricted(bool enable);
	size_t					getUserLimit(void) const;
	void					setUserLimit(size_t limit);

	std::string				getModeString(void) const;
};

#endif